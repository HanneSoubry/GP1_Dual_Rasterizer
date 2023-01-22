#include "pch.h"
#include "EffectTransparency.h"
#include "Texture.h"

namespace dae
{

	EffectTransparency::EffectTransparency(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect(pDevice, assetFile)
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
	}

	EffectTransparency::~EffectTransparency()
	{
		m_pDiffuseMapVariable->Release();
	}

	void EffectTransparency::SetDiffuseMap(Texture* pTexture)
	{
		m_pDiffuseMapVariable->SetResource(pTexture->GetResourceView());
	}

}