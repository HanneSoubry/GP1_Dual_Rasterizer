#include "pch.h"
#include "Mesh.h"
#include "Texture.h"
#include <cassert>

using namespace dae;

dae::Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Effect* effect)
	: m_pEffect{ effect }
	, m_Vertices{ vertices }
	, m_Indices{ indices }
{
	// Create Vertex Layout
	static constexpr uint32_t numElements{ 6 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
	
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[5].SemanticName = "VIEWDIRECTION";
	vertexDesc[5].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[5].AlignedByteOffset = 56;
	vertexDesc[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	
	// Create Input Layout for Point Technique
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetEffectTechnique(Effect::EffectTechnique::Point)->GetPassByIndex(0)->GetDesc(&passDesc);
	
	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);
	
	if (FAILED(result))
		assert(false);
	
	// Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_Vertices.data();
	
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;
	
	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(m_Indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	
	initData.pSysMem = m_Indices.data();
	
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}

dae::Mesh::~Mesh()
{
	if (m_pDiffuseMap)
		delete m_pDiffuseMap;
	if (m_pNormalMap)
		delete m_pNormalMap;
	if (m_pSpecularMap)
		delete m_pSpecularMap;
	if (m_pGlossinessMap)
		delete m_pGlossinessMap;

	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();
	
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	
	if (m_pInputLayout)
		m_pInputLayout->Release();
	
	delete m_pEffect;
}

void dae::Mesh::Update(const Timer* pTimer, const Matrix& viewProjectionMatrix, const Matrix& viewInverseMatrix)
{
	m_RotationMatrix *= Matrix::CreateRotationY(m_RotationSpeed * pTimer->GetElapsed());
	m_WorldMatrix = m_RotationMatrix * m_TranslationMatrix;
	UpdateWorldViewProjectionMatrix(viewProjectionMatrix, viewInverseMatrix);
}

void dae::Mesh::UpdateWorldViewProjectionMatrix(const Matrix& viewProjectionMatrix, const Matrix& viewInverseMatrix) const
{
	Matrix worldViewProjectionMatrix = m_WorldMatrix * viewProjectionMatrix;
	m_pEffect->UpdateWorldViewProjectionMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
	m_pEffect->UpdateWorldMatrix(reinterpret_cast<const float*>(&m_WorldMatrix));
	m_pEffect->UpdateViewInverseMatrix(reinterpret_cast<const float*>(&viewInverseMatrix));
}

void dae::Mesh::SetTranslationMatrix(const Matrix& translationMatrix, const Matrix& viewProjectionMatrix, const Matrix& viewInverseMatrix)
{
	m_TranslationMatrix = translationMatrix;
	m_WorldMatrix = m_RotationMatrix * m_TranslationMatrix;
	UpdateWorldViewProjectionMatrix(viewProjectionMatrix, viewInverseMatrix);
}

void dae::Mesh::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMap)
		delete m_pDiffuseMap;

	m_pDiffuseMap = pDiffuseTexture;
}

void dae::Mesh::SetNormalMap(Texture* pNormalTexture)
{
	if (m_pNormalMap)
		delete m_pNormalMap;

	m_pNormalMap = pNormalTexture;
}

void dae::Mesh::SetSpecularMap(Texture* pSpecularTexture)
{
	if (m_pSpecularMap)
		delete m_pSpecularMap;

	m_pSpecularMap = pSpecularTexture;
}

void dae::Mesh::SetGlossinessMap(Texture* pGlossinessTexture)
{
	if (m_pGlossinessMap)
		delete m_pGlossinessMap;

	m_pGlossinessMap = pGlossinessTexture;
}

void dae::Mesh::GetHardwareInfo(Effect** pEffect, ID3D11InputLayout** pInputLayout, ID3D11Buffer** pVertexBuffer, ID3D11Buffer** pIndexBuffer, uint32_t* numIndices)
{
	*pEffect = m_pEffect;
	*pInputLayout = m_pInputLayout;
	*pVertexBuffer = m_pVertexBuffer;
	*pIndexBuffer = m_pIndexBuffer;
	*numIndices = m_NumIndices;
}
