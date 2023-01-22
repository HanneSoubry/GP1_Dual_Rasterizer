#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Dual Rasterizer - 2DAE15 Soubry Hanne",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool printFPS = true;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				//if (e.key.keysym.scancode == SDL_SCANCODE_X)
				break;
			case SDL_KEYDOWN:
				// shared
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
					pRenderer->ToggleSoftwareOrHardware();
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleRotation();
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
					pRenderer->CycleCullMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
					pRenderer->ToggleBackgroundColor();
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					std::cout << COUT_COLOR_YELLOW;
					std::cout << "**(SHARED) Print FPS = ";

					printFPS = !printFPS;

					if (printFPS)
						std::cout << "ON\n";
					else
						std::cout << "OFF\n";
					std::cout << COUT_COLOR_RESET;
				}
				// only hardware
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
					pRenderer->ToggleFireMesh();
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
					pRenderer->CycleSampleStates();
				// only software
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
					pRenderer->CycleShadingMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
					pRenderer->ToggleNormalMap();
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
					pRenderer->ToggleDepthBuffer();
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
					pRenderer->ToggleBoundingBox();
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		if (printFPS)
		{
			printTimer += pTimer->GetElapsed();
			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}