#pragma once

namespace dae
{
	class Texture;

	class Effect
	{
	public:

		enum class EffectTechnique
		{
			Point,
			Linear,
			Anisotropic
		};

		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect();

		Effect(const Effect& other) = delete;
		Effect& operator=(const Effect& other) = delete;
		Effect(Effect&& other) = delete;
		Effect& operator=(Effect&& other) = delete;

		ID3DX11Effect* GetEffect() { return m_pEffect; }
		ID3DX11EffectTechnique* GetEffectTechnique(EffectTechnique technique);

		void UpdateWorldViewProjectionMatrix(const float* matrix);

		virtual void UpdateWorldMatrix(const float* matrix) {}
		virtual void UpdateViewInverseMatrix(const float* matrix) {}

	protected:
		ID3DX11Effect* m_pEffect{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};

	private:
		ID3DX11EffectTechnique* m_pTechniquePoint{};
		ID3DX11EffectTechnique* m_pTechniqueLinear{};
		ID3DX11EffectTechnique* m_pTechniqueAnisotropic{};

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	};
}