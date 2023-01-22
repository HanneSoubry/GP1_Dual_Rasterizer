#include "pch.h"
#include "Effect.h"
#include "Texture.h"

using namespace dae;

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	// effect
	m_pEffect = LoadEffect(pDevice, assetFile);

	// techniques
	m_pTechniquePoint = m_pEffect->GetTechniqueByName("PointTechnique");
	if (!m_pTechniquePoint->IsValid())
		std::wcout << L"Technique Point not valid\n";

	m_pTechniqueLinear = m_pEffect->GetTechniqueByName("LinearTechnique");
	if (!m_pTechniqueLinear->IsValid())
		std::wcout << L"Technique Linear not valid\n";

	m_pTechniqueAnisotropic = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!m_pTechniqueAnisotropic->IsValid())
		std::wcout << L"Technique Anisotropic not valid\n";

	// variables
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";

}

Effect::~Effect()
{
	m_pMatWorldViewProjVariable->Release();

	m_pTechniqueAnisotropic->Release();
	m_pTechniqueLinear->Release();
	m_pTechniquePoint->Release();

	m_pEffect->Release();
}

ID3DX11EffectTechnique* dae::Effect::GetEffectTechnique(EffectTechnique technique)
{
	switch (technique)
	{
	case dae::Effect::EffectTechnique::Point:
		return m_pTechniquePoint;
	case dae::Effect::EffectTechnique::Linear:
		return m_pTechniqueLinear;
	case dae::Effect::EffectTechnique::Anisotropic:
		return m_pTechniqueAnisotropic;
	}
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