
#include "pch.h"
#include "RasterizerHardware.h"
#include "Mesh.h"
#include "Effect.h"

using namespace dae;

RasterizerHardware::RasterizerHardware()
{
	// nothing to do 
	// initialize needs to be called dirctly and return result
}

RasterizerHardware::~RasterizerHardware()
{
	m_pRenderTargetView->Release();
	m_pRenderTargetBuffer->Release();

	m_pDepthStencilView->Release();
	m_pDepthStencilBuffer->Release();

	m_pSwapChain->Release();

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	m_pDevice->Release();
}

void dae::RasterizerHardware::RenderStart(const DualRasterizerSettings& settings)
{
	// before rendering meshes
	ColorRGB clearColor = ColorRGB{ 0.1f, 0.1f, 0.1f };

	if (!settings.uniformBackGround)
	{
		clearColor = { .39f, .59f, .93f };
	}

	// 1. Clear RTV & DSV
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

void RasterizerHardware::RenderMesh(const DualRasterizerSettings& settings, Mesh* mesh) const
{
	//0. GetMeshRenderInfo
	Effect* pEffect;
	ID3D11InputLayout* pInputLayout;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	uint32_t numIndices;

	mesh->GetHardwareInfo(&pEffect, &pInputLayout, &pVertexBuffer, &pIndexBuffer, &numIndices);

	//1. Set Primitive Topology
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Inputt Layout
	m_pDeviceContext->IASetInputLayout(pInputLayout);

	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	m_pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	pEffect->GetEffectTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		pEffect->GetEffectTechnique()->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(numIndices, 0, 0);
	}
}

void dae::RasterizerHardware::RenderFinish()
{
	// when done rendering
	
	// 3. Present backbuffer (SWAP)
	m_pSwapChain->Present(0, 0);
}

HRESULT RasterizerHardware::InitializeDirectX(SDL_Window* pWindow, int width, int height)
{
	// already initialized
	if (m_IsInitialized)
		return S_OK;

	// 1. Create Device and DeviceContext
	// ========================================
	D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
	uint32_t createDeviceFlags{ 0 };
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
		1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

	if (FAILED(result))
		return result;

	// Create DXGI Factory
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
	if (FAILED(result))
		return result;

	// 2. Create SwapChain
	// ====================
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	// Get the handle (HWND) from the SDL Backbuffer
	SDL_SysWMinfo sysWMinfo{};
	SDL_VERSION(&sysWMinfo.version);
	SDL_GetWindowWMInfo(pWindow, &sysWMinfo);
	swapChainDesc.OutputWindow = sysWMinfo.info.win.window;

	// Create SwapChain
	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
	{
		if (pDxgiFactory)
			pDxgiFactory->Release();

		return result;
	}

	pDxgiFactory->Release();

	// 3. Create DepthStencil (DS) and DepthStencilView (DSV)
	// ========================================================

	// Resource
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	// 4. Create RenderTarget (RT) and RenderTargetView (RTV)
	// ========================================================

	// Resource 
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	// View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	// 5. Bind RTV & DSV to Output Merger Stage
	// ==========================================
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 6. Set Viewport
	// ==================
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	m_IsInitialized = true;

	return S_OK;
}
