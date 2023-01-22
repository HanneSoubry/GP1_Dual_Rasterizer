#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectShader final : public Effect
	{
	public:
		EffectShader(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectShader();

		EffectShader(const EffectShader& other) = delete;
		EffectShader& operator=(const EffectShader& other) = delete;
		EffectShader(EffectShader&& other) = delete;
		EffectShader& operator=(EffectShader&& other) = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);

		void UpdateWorldMatrix(const float* matrix) override;
		void UpdateViewInverseMatrix(const float* matrix) override;

	private:
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	};
}