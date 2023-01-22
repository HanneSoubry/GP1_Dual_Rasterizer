#pragma once
#include "SettingsStruct.h"

struct SDL_Window;

namespace dae
{
	class Mesh;

	class RasterizerHardware final
	{
	public:
		RasterizerHardware();
		virtual ~RasterizerHardware();

		RasterizerHardware(const RasterizerHardware& other) = delete;
		RasterizerHardware& operator=(const RasterizerHardware& other) = delete;
		RasterizerHardware(RasterizerHardware&& other) = delete;
		RasterizerHardware& operator=(RasterizerHardware&& other) = delete;

		void RenderStart(const DualRasterizerSettings& settings);
		void RenderMesh(const DualRasterizerSettings& settings, Mesh* mesh) const;
		void RenderFinish();

		HRESULT InitializeDirectX(SDL_Window* pWindow, int width, int height);
		ID3D11Device* GetDevice() { return m_pDevice; }

	private:
		//DIRECTX
		// =========================
		bool m_IsInitialized{ false };
		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};

	};

}