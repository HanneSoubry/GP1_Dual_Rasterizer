#include "pch.h"
#include "Renderer.h"

#include "Mesh.h"
#include "Utils.h"

#include "EffectShader.h"
#include "EffectTransparency.h"
#include "Texture.h"

#include "RasterizerHardware.h"
#include "RasterizerSoftware.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		// Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		m_AspectRatio = static_cast<float>(m_Width) / m_Height;

		// Create Rasterizers
		m_pRasterizerHardware = new RasterizerHardware();
		m_pRasterizerSoftware = new RasterizerSoftware(m_pWindow, m_Width, m_Height);

		// Initialize DirectX pipeline
		HRESULT result = m_pRasterizerHardware->InitializeDirectX(m_pWindow, m_Width, m_Height);
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		PrintKeyBindings();

		m_Camera.Initialize(45.f, { 0.0f, 0.0f, 0.0f }, m_AspectRatio);

		// borrow device, DO NOT DESTROY 
		ID3D11Device* pDevice{ m_pRasterizerHardware->GetDevice() };

		// create rasterizer state
		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;

		ID3D11RasterizerState* newRasterizerState{};

		result = pDevice->CreateRasterizerState(&rasterizerDesc, &newRasterizerState);
		if (FAILED(result))
			std::wcout << L"new rasterizerState failed\n";

		// create sample state
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		ID3D11SamplerState* newSamplerState{};

		result = pDevice->CreateSamplerState(&samplerDesc, &newSamplerState);
		if (FAILED(result))
			std::wcout << L"new samplerState failed\n";

		// initialize Vehicle
		EffectShader* shaderEffect = new EffectShader{ pDevice, L"Resources/PosCol3D.fx" };
		
		Texture* pTexVehDiffuse = Texture::LoadFromFile(pDevice, "Resources/vehicle_diffuse.png");
		Texture* pTexVehNormal = Texture::LoadFromFile(pDevice, "Resources/vehicle_normal.png");
		Texture* pTexVehSpecular = Texture::LoadFromFile(pDevice, "Resources/vehicle_specular.png");
		Texture* pTexVehGlossiness = Texture::LoadFromFile(pDevice, "Resources/vehicle_gloss.png");
		
		shaderEffect->SetDiffuseMap(pTexVehDiffuse);
		shaderEffect->SetNormalMap(pTexVehNormal);
		shaderEffect->SetSpecularMap(pTexVehSpecular);
		shaderEffect->SetGlossinessMap(pTexVehGlossiness);

		shaderEffect->SetSamplerState(newSamplerState);
		shaderEffect->SetCullMode(newRasterizerState);
			
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);
		m_pVehicle = new Mesh{ pDevice, vertices, indices, shaderEffect };
		
		m_pVehicle->SetTranslationMatrix(Matrix::CreateTranslation(0, 0, 50), m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
		m_pVehicle->SetOnlyHardWare(false);
		
		// mesh takes ownership of textures and will delete them
		m_pVehicle->SetDiffuseMap(pTexVehDiffuse);
		m_pVehicle->SetNormalMap(pTexVehNormal);
		m_pVehicle->SetSpecularMap(pTexVehSpecular);
		m_pVehicle->SetGlossinessMap(pTexVehGlossiness);
		
		// initialize Fire
		EffectTransparency* transparencyEffect = new EffectTransparency{ pDevice, L"Resources/Transparency.fx" };
		
		Texture* pFireTexture = Texture::LoadFromFile(pDevice, "Resources/fireFX_diffuse.png");
		transparencyEffect->SetDiffuseMap(pFireTexture);

		transparencyEffect->SetSamplerState(newSamplerState);
		transparencyEffect->SetCullMode(newRasterizerState);
		
		vertices.clear();
		indices.clear();
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);
		m_pFire = new Mesh{ pDevice, vertices, indices, transparencyEffect };
		
		m_pFire->SetDiffuseMap(pFireTexture);

		m_pFire->SetTranslationMatrix(Matrix::CreateTranslation(0, 0, 50), m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
		m_pFire->SetOnlyHardWare(true);

		newSamplerState->Release();
		newRasterizerState->Release();
	}

	Renderer::~Renderer()
	{
		delete m_pVehicle;
		delete m_pFire;

		delete m_pRasterizerHardware;
		delete m_pRasterizerSoftware;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer, m_Settings);

		if (m_Settings.rotating)
		{
			m_pVehicle->Update(pTimer, m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
			m_pFire->Update(pTimer, m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
		}
		else
		{
			// update view and projection matrix (from camera)
			m_pVehicle->UpdateWorldViewProjectionMatrix(m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
			m_pFire->UpdateWorldViewProjectionMatrix(m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
		}
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		switch (m_Settings.rasterizerMode)
		{
		case dae::RasterizerMode::SoftWare:
		{
			// 1. Buffer setup
			m_pRasterizerSoftware->RenderStart(m_Settings);

			// 2. Draw meshes
			m_pRasterizerSoftware->RenderMesh(m_Settings, m_Camera, m_pVehicle);
			
			// 3. Swap buffers
			m_pRasterizerSoftware->RenderFinish(m_pWindow);
			break;
		}
		case dae::RasterizerMode::Hardware:
		{
			if (!m_IsInitialized)
				return;

			// 1. Clear RTV & DSV
			m_pRasterizerHardware->RenderStart(m_Settings);

			// 2. Set pipeline + invoke drawcalls (RENDER)
			m_pRasterizerHardware->RenderMesh(m_Settings, m_pVehicle);

			if (m_Settings.showFireMesh)
			{
				m_pRasterizerHardware->RenderMesh(m_Settings, m_pFire);
			}

			// 3. Present backbuffer (SWAP)
			m_pRasterizerHardware->RenderFinish();

			break;
		}
		}

	}

	void Renderer::ToggleSoftwareOrHardware()
	{
		std::cout << COUT_COLOR_YELLOW;
		std::cout << "**(SHARED) Rasterizer Mode = ";

		switch (m_Settings.rasterizerMode)
		{
		case dae::RasterizerMode::SoftWare:
			m_Settings.rasterizerMode = RasterizerMode::Hardware;
			std::cout << "HARDWARE\n";
			break;

		case dae::RasterizerMode::Hardware:
			m_Settings.rasterizerMode = RasterizerMode::SoftWare;
			std::cout << "SOFTWARE\n";
			break;
		}

		std::cout << COUT_COLOR_RESET;
	}

	void Renderer::ToggleRotation()
	{
		std::cout << COUT_COLOR_YELLOW;
		std::cout << "**(SHARED) Rotation = ";

		m_Settings.rotating = !m_Settings.rotating;

		if (m_Settings.rotating)
			std::cout << "ON\n";
		else
			std::cout << "OFF\n";
		std::cout << COUT_COLOR_RESET;
	}

	void Renderer::CycleCullMode()
	{
		std::cout << COUT_COLOR_YELLOW;
		std::cout << "**(SHARED) CullMode = ";

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;

		switch (m_Settings.cullMode)
		{
		case dae::CullMode::Back:
			m_Settings.cullMode = CullMode::Front;
			rasterizerDesc.CullMode = D3D11_CULL_FRONT;
			std::cout << "FRONT\n";
			break;
		case dae::CullMode::Front:
			m_Settings.cullMode = CullMode::None;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			std::cout << "NONE\n";
			break;
		case dae::CullMode::None:
			m_Settings.cullMode = CullMode::Back;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			std::cout << "BACK\n";
			break;
		}

		ID3D11Device* pDevice = m_pRasterizerHardware->GetDevice();
		ID3D11RasterizerState* newRasterizerState{};

		HRESULT result = pDevice->CreateRasterizerState(&rasterizerDesc, &newRasterizerState);
		if (FAILED(result))
			std::wcout << L"new rasterizerState failed\n";

		m_pVehicle->SetCullMode(newRasterizerState);
		m_pFire->SetCullMode(newRasterizerState);

		newRasterizerState->Release();

		std::cout << COUT_COLOR_RESET;
	}

	void Renderer::ToggleBackgroundColor()
	{
		std::cout << COUT_COLOR_YELLOW;
		std::cout << "**(SHARED) Uniform ClearColor = ";

		m_Settings.uniformBackGround = !m_Settings.uniformBackGround;

		if (m_Settings.uniformBackGround)
			std::cout << "ON\n";
		else
			std::cout << "OFF\n";
		std::cout << COUT_COLOR_RESET;
	}

	void Renderer::ToggleFireMesh()
	{
		// only hardware
		if (m_Settings.rasterizerMode == RasterizerMode::Hardware)
		{
			std::cout << COUT_COLOR_GREEN;
			std::cout << "**(HARDWARE) Show FireFX = ";

			m_Settings.showFireMesh = !m_Settings.showFireMesh;

			if (m_Settings.showFireMesh)
				std::cout << "ON\n";
			else
				std::cout << "OFF\n";
			std::cout << COUT_COLOR_RESET;
		}
	}

	void Renderer::CycleSampleStates()
	{
		// only hardware
		if (m_Settings.rasterizerMode == RasterizerMode::Hardware)
		{
			std::cout << COUT_COLOR_GREEN;
			std::cout << "**(HARDWARE) Show FireFX = ";

			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MinLOD = -FLT_MAX;
			samplerDesc.MaxLOD = FLT_MAX;
			samplerDesc.MipLODBias = 0.f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

			switch (m_Settings.sampleState)
			{
			case dae::SampleState::Point:
				m_Settings.sampleState = SampleState::Linear;
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				std::cout << "LINEAR\n";
				break;
			case dae::SampleState::Linear:
				m_Settings.sampleState = SampleState::Anisotropic;
				samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
				std::cout << "ANISOTROPIC\n";
				break;
			case dae::SampleState::Anisotropic:
				m_Settings.sampleState = SampleState::Point;
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				std::cout << "POINT\n";
				break;
			}

			ID3D11Device* pDevice = m_pRasterizerHardware->GetDevice();
			ID3D11SamplerState* newSamplerState{};

			HRESULT result = pDevice->CreateSamplerState(&samplerDesc, &newSamplerState);
			if (FAILED(result))
				std::wcout << L"new samplerState failed\n";

			m_pVehicle->SetSamplerState(newSamplerState);
			m_pFire->SetSamplerState(newSamplerState);

			newSamplerState->Release();

			std::cout << COUT_COLOR_RESET;
		}
	}

	void Renderer::CycleShadingMode()
	{
		// only software
		if (m_Settings.rasterizerMode == RasterizerMode::SoftWare)
		{
			std::cout << COUT_COLOR_MAGENTA;
			std::cout << "**(SOFTWARE) Shading Mode = ";

			switch (m_Settings.shadingMode)
			{
			case dae::ShadingMode::Combined:
				m_Settings.shadingMode = ShadingMode::ObservedArea;
				std::cout << "OBSERVED_AREA\n";
				break;
			case dae::ShadingMode::ObservedArea:
				m_Settings.shadingMode = ShadingMode::Diffuse;
				std::cout << "DIFFUSE\n";
				break;
			case dae::ShadingMode::Diffuse:
				m_Settings.shadingMode = ShadingMode::Specular;
				std::cout << "SPECULAR\n";
				break;
			case dae::ShadingMode::Specular:
				m_Settings.shadingMode = ShadingMode::Combined;
				std::cout << "COMBINED\n";
				break;
			}

			std::cout << COUT_COLOR_RESET;
		}
	}

	void Renderer::ToggleNormalMap()
	{
		// only software
		if (m_Settings.rasterizerMode == RasterizerMode::SoftWare)
		{
			std::cout << COUT_COLOR_MAGENTA;
			std::cout << "**(SOFTWARE) Use Normal Map = ";

			m_Settings.useNormalMap = !m_Settings.useNormalMap;

			if (m_Settings.useNormalMap)
				std::cout << "ON\n";
			else
				std::cout << "OFF\n";
			std::cout << COUT_COLOR_RESET;
		}
	}

	void Renderer::ToggleDepthBuffer()
	{
		// only software
		if (m_Settings.rasterizerMode == RasterizerMode::SoftWare)
		{
			std::cout << COUT_COLOR_MAGENTA;
			std::cout << "**(SOFTWARE) Show Depth Buffer = ";

			m_Settings.showDepthBuffer = !m_Settings.showDepthBuffer;

			// no depth buffer and normal map at the same time
			m_Settings.showBoundingBox = false;

			if (m_Settings.showDepthBuffer)
				std::cout << "ON\n";
			else
				std::cout << "OFF\n";
			std::cout << COUT_COLOR_RESET;
		}
	}

	void Renderer::ToggleBoundingBox()
	{
		// only software
		if (m_Settings.rasterizerMode == RasterizerMode::SoftWare)
		{
			std::cout << COUT_COLOR_MAGENTA;
			std::cout << "**(SOFTWARE) Show Bounding Box = ";

			m_Settings.showBoundingBox = !m_Settings.showBoundingBox;

			// no depth buffer and normal map at the same time
			m_Settings.showDepthBuffer = false;

			if (m_Settings.showBoundingBox)
				std::cout << "ON\n";
			else
				std::cout << "OFF\n";
			std::cout << COUT_COLOR_RESET;
		}
	}

	void Renderer::PrintKeyBindings()
	{
		std::cout << COUT_COLOR_YELLOW;
		std::cout << "[Key Bindings - SHARED]\n"
			<< "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n"
			<< "   [F2]  Toggle Vehicle Rotation (ON/OFF)\n"
			<< "   [F9]  Cycle CullMode (BACK/FRONT/NONE)\n"
			<< "   [F10] Toggle Uniform ClearColor (ON/OFF)\n"
			<< "   [F11] Toggle Print FPS (ON/OFF)\n";

		std::cout << COUT_COLOR_GREEN;
		std::cout << "[Key Bindings - HARDWARE]\n"
			<< "   [F3]  Toggle FireFX (ON/OFF)\n"
			<< "   [F4]  Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n";

		std::cout << COUT_COLOR_MAGENTA;
		std::cout << "[Key Bindings - SOFTWARE]\n"
			<< "   [F5]  Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n"
			<< "   [F6]  Toggle NormalMap (ON/OFF)\n"
			<< "   [F7]  Toggle DepthBuffer Visualization (ON/OFF)\n"
			<< "   [F8]  Toggle BoundingBox Visualization (ON/OFF)\n";

		std::cout << COUT_COLOR_RESET;
	}
}
