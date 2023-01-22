#pragma once
#include "SettingsStruct.h"

struct SDL_Window;

namespace dae
{
	class Mesh;

	class RasterizerSoftware final
	{
	public:
		RasterizerSoftware(SDL_Window* pWindow, int width, int height);
		virtual ~RasterizerSoftware();

		RasterizerSoftware(const RasterizerSoftware& other) = delete;
		RasterizerSoftware& operator=(const RasterizerSoftware& other) = delete;
		RasterizerSoftware(RasterizerSoftware&& other) = delete;
		RasterizerSoftware& operator=(RasterizerSoftware&& other) = delete;

		void RenderStart(const DualRasterizerSettings& settings, int width, int height);
		void RenderMesh(const DualRasterizerSettings& settings, Mesh* mesh) const;
		void RenderFinish(SDL_Window* pWindow);

	private:
		// buffers
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		// directional light
		const Vector3 m_LightDirection{ .577f, -.577f, .577f };
		const float m_LightIntensity{ 7.f };

		// functions
		void Projection();
		void Rasterization();
		void PixelShading();
	};

}