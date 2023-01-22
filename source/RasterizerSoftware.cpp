
//External includes
#include "SDL.h"
#include "SDL_surface.h"

//includes
#include "pch.h"
#include "RasterizerSoftware.h"
#include "Mesh.h"

using namespace dae;

RasterizerSoftware::RasterizerSoftware(SDL_Window* pWindow, int width, int height)
{
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[width * height];
}

RasterizerSoftware::~RasterizerSoftware()
{
	delete[] m_pDepthBufferPixels;
}

void RasterizerSoftware::RenderStart(const DualRasterizerSettings& settings, int width, int height)
{
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	float r{ 0.1f };
	float g{ 0.1f };
	float b{ 0.1f };

	if (!settings.uniformBackGround)
	{
		r = 0.39f;
		g = 0.39f;
		b = 0.39f;
	}

	// fill rect takes color values [0, 255] (rgb was [0,1])
	r *= 255;
	g *= 255;
	b *= 255;

	const int nrPixels{ width * height };
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, r, g, b));
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);
}

void RasterizerSoftware::RenderMesh(const DualRasterizerSettings& settings, Mesh* mesh) const
{
	// TODO: projection
	// TODO: rasterization
	// TODO: shading
}

void RasterizerSoftware::RenderFinish(SDL_Window* pWindow)
{
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(pWindow);
}
