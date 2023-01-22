#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectTransparency final : public Effect
	{
	public:
		EffectTransparency(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectTransparency();

		void SetDiffuseMap(Texture* pTexture);

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	};

}