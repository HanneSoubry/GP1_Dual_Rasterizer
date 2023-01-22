#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(ID3D11Device* pDevice, const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;

		ID3D11ShaderResourceView* GetResourceView() { return m_pSRV; }

	private:
		Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;

		float m_DivideBy255{ 1.f / 255.f };
	};
}