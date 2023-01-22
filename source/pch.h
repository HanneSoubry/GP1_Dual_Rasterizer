#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#define NOMINMAX  //for directx

// SDL Headers
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"
#include "SDL_image.h"

// DirectX Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

// Framework Headers
#include "Timer.h"
#include "Math.h"

// std::cout colors
#define COUT_COLOR_RESET   "\033[0m"
#define COUT_COLOR_YELLOW  "\033[33m"
#define COUT_COLOR_GREEN   "\033[32m"
#define COUT_COLOR_MAGENTA "\033[35m"