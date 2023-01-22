#pragma once
#include "SettingsStruct.h"
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class RasterizerHardware;
	class RasterizerSoftware;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		// toggle settings
		void ToggleSoftwareOrHardware();
		void ToggleRotation();
		void CycleCullMode();
		void ToggleBackgroundColor();
		void ToggleFireMesh();
		void CycleSampleStates();
		void CycleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBox();

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		float m_AspectRatio{};
		Camera m_Camera{};

		// helper functions
		// =======================
		void PrintKeyBindings();

		// meshes
		// =======================
		Mesh* m_pVehicle;
		Mesh* m_pFire;

		// rasterizers
		// =======================
		RasterizerHardware* m_pRasterizerHardware;
		RasterizerSoftware* m_pRasterizerSoftware;

		// toggle settings
		// =======================
		DualRasterizerSettings m_Settings{};

		//DIRECTX
		// =========================
		bool m_IsInitialized{ false };
	};
}
