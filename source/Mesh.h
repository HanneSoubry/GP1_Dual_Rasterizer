#pragma once
#include "Effect.h"

namespace dae
{
	class Effect;
	class Texture;

	struct Vertex final
	{
		Vector3 position{};
		ColorRGB color{ colors::White };
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Effect* effect);
		~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		void Update(const Timer* pTimer, const Matrix& viewProjectionMatrix, const Matrix& viewInverseMatrix);
		void UpdateWorldViewProjectionMatrix(const Matrix& viewProjectionMatrix, const Matrix& viewInverseMatrix) const;

		void SetOnlyHardWare(bool onlyHardware) { m_OnlyHardware = onlyHardware; }
		void SetSamplerState(ID3D11SamplerState* pNewSamplerState);
		void SetCullMode(ID3D11RasterizerState* pNewRasterizerState);

		void SetRotationSpeed(float speed) { m_RotationSpeed = speed; }
		void SetTranslationMatrix(const Matrix& translationMatrix, const Matrix& viewProjectionMatrix, const Matrix& viewInverseMatrix);

		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);

		// access for hardware
		void GetHardwareInfo(	Effect** pEffect,
								ID3D11InputLayout** pInputLayout,
								ID3D11Buffer** pVertexBuffer,
								ID3D11Buffer** pIndexBuffer,
								uint32_t& numIndices);

		// access for software
		void GetSoftwareInfo(	Matrix** pWorldMatrix, 
								std::vector<Vertex>** pVertices,
								std::vector<uint32_t>** pIndices,
								PrimitiveTopology& primitiveTopology,
								std::vector<Vertex_Out>** pVertices_out, 
								Texture** pDiffuseMap, 
								Texture** pNormalMap, 
								Texture** pSpecularMap, 
								Texture** pGlossinessMap);

		bool IsOnlyForHardware() { return m_OnlyHardware; }

	private:
		// shared
		float m_RotationSpeed{ 45.0f * TO_RADIANS };
		Matrix m_WorldMatrix{};
		Matrix m_TranslationMatrix{};
		Matrix m_RotationMatrix{};

		// SAFETY CHECK: no transparency in software & no normal, specular and glossiness map for transparent objects
		bool m_OnlyHardware{ true }; 

		// hardware
		Effect* m_pEffect;

		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		uint32_t m_NumIndices{};

		// software
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };
		std::vector<Vertex_Out> m_Vertices_out{};

		Texture* m_pDiffuseMap{ nullptr };
		Texture* m_pNormalMap{ nullptr };
		Texture* m_pSpecularMap{ nullptr };
		Texture* m_pGlossinessMap{ nullptr };
	};
}