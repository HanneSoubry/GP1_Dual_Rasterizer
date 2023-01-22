#include "pch.h"
#include "Effect.h"
#include "Texture.h"

using namespace dae;

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	// effect
	m_pEffect = LoadEffect(pDevice, assetFile);

	// techniques
	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";

	// variables
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";

	m_pSamplerState = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();
	if(!m_pSamplerState->IsValid())
		std::wcout << L"m_pSamplerState not valid!\n";

	m_pRasterizerState = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
	if (!m_pRasterizerState->IsValid())
		std::wcout << L"m_pRasterizerState not valid!\n";
}

Effect::~Effect()
{
	m_pSamplerState->Release();
	m_pRasterizerState->Release();

	m_pMatWorldViewProjVariable->Release();

	m_pTechnique->Release();

	m_pEffect->Release();
}

void dae::Effect::SetSamplerState(ID3D11SamplerState* pNewSamplerState)
{
	HRESULT result = m_pSamplerState->SetSampler(0, pNewSamplerState);
	if (FAILED(result))
		std::wcout << L"failed to set new sampler state\n";
}

void dae::Effect::SetCullMode(ID3D11RasterizerState* pNewRasterizerState)
{
	HRESULT result = m_pRasterizerState->SetRasterizerState(0, pNewRasterizerState);
	if (FAILED(result))
		std::wcout << L"failed to set new rasterizer state\n";
}

void dae::Effect::UpdateWorldViewProjectionMatrix(const float* matrix)
{
	m_pMatWorldViewProjVariable->SetMatrix(matrix);
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(	assetFile.c_str(),
											nullptr,
											nullptr,
											shaderFlags,
											0,
											pDevice,
											&pEffect,
											&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
			
			std::wcout << ss.str() << std::endl;
		}	
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}