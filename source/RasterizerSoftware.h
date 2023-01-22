#pragma once
#include "SettingsStruct.h"
#include "Mesh.h"
#include "Camera.h"

struct SDL_Window;

namespace dae
{
	class RasterizerSoftware final
	{
	public:
		RasterizerSoftware(SDL_Window* pWindow, int width, int height);
		virtual ~RasterizerSoftware();

		RasterizerSoftware(const RasterizerSoftware& other) = delete;
		RasterizerSoftware& operator=(const RasterizerSoftware& other) = delete;
		RasterizerSoftware(RasterizerSoftware&& other) = delete;
		RasterizerSoftware& operator=(RasterizerSoftware&& other) = delete;

		void RenderStart(const DualRasterizerSettings& settings) const;
		void RenderMesh(const DualRasterizerSettings& settings, const Camera& camera, Mesh* mesh) const;
		void RenderFinish(SDL_Window* pWindow) const;

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
		void ProjectionStage(const Camera& camera, Matrix* pWorldMatrix, std::vector<Vertex>* pVertices, std::vector<Vertex_Out>* pVerticesOut) const;
		void RasterizationStage(const DualRasterizerSettings& settings, std::vector<Vertex_Out>* pVerticesOut, std::vector<uint32_t>* pIndices, const PrimitiveTopology& primitiveTopology, Texture* pDiffuse, Texture* pNormal, Texture* pSpecular, Texture* pGlossiness) const;
		ColorRGB PixelShadingStage(const DualRasterizerSettings& settings, const Vertex_Out shadeInfo, Texture* pDiffuse, Texture* pNormal, Texture* pSpecular, Texture* pGlossiness) const;

		// helper functions
		bool IsInsideFrustrum(const Vector4& vertex) const;
		float Remap(float value, float min, float max) const;

		// rendering variables
		int m_ScreenWidth{};
		int m_ScreenHeight{};
	};

}