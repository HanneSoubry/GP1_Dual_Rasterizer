
#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(ID3D11Device* pDevice, SDL_Surface* pSurface) : 
		m_pSurface{pSurface},
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

		if (FAILED(hr))
			return;

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);

		if (FAILED(hr))
			return;
	}

	Texture::~Texture()
	{
		m_pSRV->Release();
		m_pResource->Release();

		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& path)
	{
		//Load SDL_Surface using IMG_LOAD
		SDL_Surface* pSurface{};
		pSurface = IMG_Load(path.c_str());

		//Create & Return a new Texture Object (using SDL_Surface)
		Texture* newTexture{ new Texture{pDevice, pSurface} };
		return newTexture;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//Sample the correct texel for the given uv
		// uv range [0, 1] to range [0, texturewidth or height]
		const int u{ static_cast<int>(uv.x * m_pSurface->w) };
		const int v{ static_cast<int>(uv.y * m_pSurface->h) };
	
		uint8_t r{};
		uint8_t g{};
		uint8_t b{};
	
		// get the color in [0, 255]
		SDL_GetRGB(m_pSurfacePixels[u + v * m_pSurface->w], m_pSurface->format, &r, &g, &b );
	
		// color range to [0 ,1]
		// optimization -> prefer multiply over devision
		return { r * m_DivideBy255, g * m_DivideBy255, b * m_DivideBy255 };
	}
}