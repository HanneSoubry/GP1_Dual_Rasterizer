#pragma once

namespace dae
{
	// ENUMS FOR SETTINGS
	enum class RasterizerMode
	{
		SoftWare,
		Hardware
	};
	enum class CullMode
	{
		Back,
		Front,
		None
	};
	enum class SampleState
	{
		Point,
		Linear,
		Anisotropic
	};
	enum class ShadingMode
	{
		Combined,
		ObservedArea,
		Diffuse,
		Specular
	};

	// SETTINGS
	struct DualRasterizerSettings
	{
		// shared
		RasterizerMode rasterizerMode{ RasterizerMode::Hardware };
		bool rotating{ true };
		CullMode cullMode{ CullMode::None };
		bool uniformBackGround{ false };

		// only hardware
		bool showFireMesh{ true };
		SampleState sampleState{ SampleState::Point };

		// only software
		ShadingMode shadingMode{ ShadingMode::Combined };
		bool useNormalMap{ true };
		bool showDepthBuffer{ false };
		bool showBoundingBox{ false };
	};

}