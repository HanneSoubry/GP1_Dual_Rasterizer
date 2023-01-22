#include "pch.h"
#include "EffectShader.h"
#include "Texture.h"

dae::EffectShader::EffectShader(ID3D11Device* pDevice, const std::wstring& assetFile)
	:Effect(pDevice, assetFile)
{
	// matrix variables
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
		std::wcout << L"m_pMatWorldVariable not valid!\n";

	m_pMatInvViewVariable = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
	if (!m_pMatInvViewVariable->IsValid())
		std::wcout << L"m_pMatInvViewVariable not valid!\n";

	// maps
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"m_pDiffuseMapVariable not valid!\n";

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::wcout << L"m_pNormalMapVariable not valid!\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::wcout << L"m_pSpecularMapVariable not valid!\n";

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
		std::wcout << L"m_pGlossinessMapVariable not valid!\n";
}

dae::EffectShader::~EffectShader()
{
	m_pGlossinessMapVariable->Release();
	m_pSpecularMapVariable->Release();
	m_pNormalMapVariable->Release();
	m_pDiffuseMapVariable->Release();

	m_pMatInvViewVariable->Release();
	m_pMatWorldVariable->Release();
}


void dae::EffectShader::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetResourceView());
	}
}

void dae::EffectShader::SetNormalMap(Texture* pNormalTexture)
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->SetResource(pNormalTexture->GetResourceView());
	}
}

void dae::EffectShader::SetSpecularMap(Texture* pSpecularTexture)
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetResourceView());
	}
}

void dae::EffectShader::SetGlossinessMap(Texture* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetResourceView());
	}
}

void dae::EffectShader::UpdateWorldMatrix(const float* matrix)
{
	m_pMatWorldVariable->SetMatrix(matrix);
}

void dae::EffectShader::UpdateViewInverseMatrix(const float* matrix)
{
	m_pMatInvViewVariable->SetMatrix(matrix);
}