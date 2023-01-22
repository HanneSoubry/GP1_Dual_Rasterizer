
//External includes
#include "SDL.h"
#include "SDL_surface.h"

//includes
#include "pch.h"
#include "RasterizerSoftware.h"
#include "Texture.h"

using namespace dae;

RasterizerSoftware::RasterizerSoftware(SDL_Window* pWindow, int width, int height)
{
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[width * height];

	m_ScreenWidth = width;
	m_ScreenHeight = height;
}

RasterizerSoftware::~RasterizerSoftware()
{
	delete[] m_pDepthBufferPixels;
}

void RasterizerSoftware::RenderStart(const DualRasterizerSettings& settings) const
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

	const int nrPixels{ m_ScreenWidth * m_ScreenHeight };
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, r, g, b));
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);
}

void RasterizerSoftware::RenderMesh(const DualRasterizerSettings& settings, const Camera& camera, Mesh* mesh) const
{
	if (mesh->IsOnlyForHardware())
		return;
	// get info for software rasterizing
	Matrix* pWorldMatrix{};
	std::vector<Vertex>* pVertices{};
	std::vector<uint32_t>* pIndices{};
	PrimitiveTopology primitiveTopology{};
	std::vector<Vertex_Out>* pVerticesOut{};
	Texture* pDiffuseMap{};
	Texture* pNormalMap{};
	Texture* pSpecularMap{};
	Texture* pGlossinessMap{};
	mesh->GetSoftwareInfo(&pWorldMatrix, &pVertices, &pIndices, primitiveTopology, &pVerticesOut, &pDiffuseMap, &pNormalMap, &pSpecularMap, &pGlossinessMap);

	ProjectionStage(camera, pWorldMatrix, pVertices, pVerticesOut);
	RasterizationStage(settings, pVerticesOut, pIndices, primitiveTopology, pDiffuseMap, pNormalMap, pSpecularMap, pGlossinessMap);
	// rasterization will call pixelShading per pixel
}

void RasterizerSoftware::RenderFinish(SDL_Window* pWindow) const
{
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(pWindow);
}

void RasterizerSoftware::ProjectionStage(const Camera& camera, Matrix* pWorldMatrix, std::vector<Vertex>* pVertices, std::vector<Vertex_Out>* pVerticesOut) const
{
	const Matrix worldViewProjectionMatrix{ (*pWorldMatrix) * camera.viewMatrix * camera.projectionMatrix };
	pVerticesOut->clear();
	pVerticesOut->reserve(pVertices->size());

	for (const Vertex& currentVertex : *pVertices)
	{
		Vertex_Out transformedVertex{	{currentVertex.position, 1.f}, 
										currentVertex.color, 
										currentVertex.uv, 
										currentVertex.normal, 
										currentVertex.tangent, 
										currentVertex.viewDirection };

		// transform from mesh position to projection
		transformedVertex.position = worldViewProjectionMatrix.TransformPoint(transformedVertex.position);

		// perspective divide
		const float invW{ 1.f / transformedVertex.position.w };
		transformedVertex.position.x *= invW;
		transformedVertex.position.y *= invW;
		transformedVertex.position.z *= invW;

		// transform normals to world space
		transformedVertex.normal = pWorldMatrix->TransformVector(transformedVertex.normal);
		transformedVertex.tangent = pWorldMatrix->TransformVector(transformedVertex.tangent);

		// calculate view direction
		transformedVertex.viewDirection = pWorldMatrix->TransformPoint(currentVertex.position) - camera.origin;

		// add to vertices_out
		pVerticesOut->emplace_back(transformedVertex);
	}
}

void RasterizerSoftware::RasterizationStage(const DualRasterizerSettings& settings, std::vector<Vertex_Out>* pVerticesOut, std::vector<uint32_t>* pIndices, const PrimitiveTopology& primitiveTopology, Texture* pDiffuse, Texture* pNormal, Texture* pSpecular, Texture* pGlossiness) const
{
	// get screen-space vertices (before in range of frustrum)
	std::vector<Vector2> verticesScreen{};
	verticesScreen.reserve(pVerticesOut->size());

	for (const auto& currVertex : *pVerticesOut)
	{
		verticesScreen.emplace_back(Vector2{ (currVertex.position.x + 1) * 0.5f * m_ScreenWidth, (1 - currVertex.position.y) * 0.5f * m_ScreenHeight });
	}

	int increment{ 1 };
	if (primitiveTopology == PrimitiveTopology::TriangleList)
	{
		increment = 3;
	}
	else if (primitiveTopology == PrimitiveTopology::TriangleStrip)
	{
		increment = 1;
	}

	for (int i{}; i < pIndices->size() - 2; i += increment)
	{
		// get indices
		int modulo{ 0 };	// 0 has no effect on + or -

		if (increment == 1)	// trianglestrip -> check direction of vertices
			modulo = i % 2;	// 0 or 1

		const int index0{ static_cast<int>((*pIndices)[i]) };
		const int index1{ static_cast<int>((*pIndices)[i + 1 + modulo]) };
		const int index2{ static_cast<int>((*pIndices)[i + 2 - modulo]) };

		// check if valid triangle
		if (index0 == index1 || index1 == index2 || index2 == index0)
			continue;

		//// check cullmode
		//Vector3 normal{ (*pVerticesOut)[index0].normal + (*pVerticesOut)[index1].normal + (*pVerticesOut)[index2].normal };
		//normal /= 3;
		//Vector3 viewDirection{ (*pVerticesOut)[index0].viewDirection + (*pVerticesOut)[index1].viewDirection + (*pVerticesOut)[index2].viewDirection };
		//viewDirection /= 3;
		//
		//const float dotViewAndPlane{ Vector3::Dot(normal, viewDirection) };
		//if (dotViewAndPlane == 0)
		//	continue;
		//
		//switch (settings.cullMode)
		//{
		//case CullMode::Back:
		//{
		//	if (dotViewAndPlane > 0)
		//		continue;
		//	break;
		//}
		//case CullMode::Front:
		//{
		//	if (dotViewAndPlane < 0)
		//		continue;
		//	break;
		//}
		//// case no culling: do nothing
		//}

		// get positions
		const Vector4 vertexPos0{ (*pVerticesOut)[index0].position };
		const Vector4 vertexPos1{ (*pVerticesOut)[index1].position };
		const Vector4 vertexPos2{ (*pVerticesOut)[index2].position };

		// frustrum culling => if not inside frustrum => skip this triangle
		if (!IsInsideFrustrum(vertexPos0) ||
			!IsInsideFrustrum(vertexPos1) ||
			!IsInsideFrustrum(vertexPos2))
			continue;

		// vertices
		const Vector2 vertex0{ verticesScreen[index0].x, verticesScreen[index0].y };
		const Vector2 vertex1{ verticesScreen[index1].x, verticesScreen[index1].y };
		const Vector2 vertex2{ verticesScreen[index2].x, verticesScreen[index2].y };

		// edges
		const Vector2 edge10{ vertex1 - vertex0 };
		const Vector2 edge21{ vertex2 - vertex1 };
		const Vector2 edge02{ vertex0 - vertex2 };

		const float triangleArea{ Vector2::Cross({ vertex2 - vertex0 }, edge10) };
		const float inversTriangleArea{ 1 / triangleArea };

		// bounding box																	// + 1 to correct rounding down to int
		int maxX{ std::max(static_cast<int>(vertex0.x), std::max(static_cast<int>(vertex1.x), static_cast<int>(vertex2.x))) + 1 };
		int minX{ std::min(static_cast<int>(vertex0.x), std::min(static_cast<int>(vertex1.x), static_cast<int>(vertex2.x))) };
		int maxY{ std::max(static_cast<int>(vertex0.y), std::max(static_cast<int>(vertex1.y), static_cast<int>(vertex2.y))) + 1 };
		int minY{ std::min(static_cast<int>(vertex0.y), std::min(static_cast<int>(vertex1.y), static_cast<int>(vertex2.y))) };

		if (maxX > m_ScreenWidth) maxX = m_ScreenWidth;
		if (minX < 0) minX = 0;
		if (maxY > m_ScreenHeight) maxY = m_ScreenHeight;
		if (minY < 0) minY = 0;

		// for each pixel in bounding box
		// -------------------------------
		for (int px{ minX }; px <= maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				const int pixelIndex{ px + py * m_ScreenWidth };

				if (settings.showBoundingBox)
				{
					m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255));
					continue;
				}
			
				const Vector2 currentPixel{ static_cast<float>(px), static_cast<float>(py) };

				// vectors to pixel
				const Vector2 v0toPixel{ vertex0 - currentPixel };
				const Vector2 v1toPixel{ vertex1 - currentPixel };
				const Vector2 v2toPixel{ vertex2 - currentPixel };

				// cross products
				const float crossEdge10{ Vector2::Cross(edge10, v0toPixel) };
				const float crossEdge21{ Vector2::Cross(edge21, v1toPixel) };
				const float crossEdge02{ Vector2::Cross(edge02, v2toPixel) };

				// if not in triangle
				switch (settings.cullMode)
				{
				case CullMode::Back:
				{
					// return if pixel does not hit or hits from the back side 
					if (crossEdge10 > 0 || crossEdge21 > 0 || crossEdge02 > 0)
						continue;
					break;
				}
				case CullMode::Front:
				{
					// return if pixel does not hit or hits from the front side
					if (crossEdge10 < 0 || crossEdge21 < 0 || crossEdge02 < 0)
						continue;
					break;
				}
				case CullMode::None:
				{
					// return if pixel hits on the edge
					if (!(crossEdge10 > 0 && crossEdge21 > 0 && crossEdge02 > 0) && !(crossEdge10 < 0 && crossEdge21 < 0 && crossEdge02 < 0))
						continue;
					break;
				}
				}

				// pixel is in triangle
				// -------------------------

				// weights
				const float weight0{ crossEdge21 / triangleArea };
				const float weight1{ crossEdge02 / triangleArea };
				const float weight2{ crossEdge10 / triangleArea };

				// depth (using z values for frustrum clipping)
				const float inverseZ0{ 1.f / vertexPos0.z };
				const float inverseZ1{ 1.f / vertexPos1.z };
				const float inverseZ2{ 1.f / vertexPos2.z };

				const float depth{ 1.f / (inverseZ0 * weight0 +
											inverseZ1 * weight1 +
											inverseZ2 * weight2) };

				// if further than previous rendered pixel 
				//		=> skip this pixel
				if (depth >= m_pDepthBufferPixels[pixelIndex])
					continue;

				// store new depth
				m_pDepthBufferPixels[pixelIndex] = depth;

				ColorRGB finalColor{};

				// Color
				if (settings.showDepthBuffer)
				{
					const float remappedDepth{ Remap(depth, 0.990f, 1.f) };
					finalColor = { remappedDepth, remappedDepth, remappedDepth };
				}
				else
				{
					// get uv(using w values and viewSpaceDepth for linear interpolation)
					const float inverseW0{ 1.f / vertexPos0.w };
					const float inverseW1{ 1.f / vertexPos1.w };
					const float inverseW2{ 1.f / vertexPos2.w };

					const float viewSpaceDepth{ 1.f / (inverseW0 * weight0 +
														inverseW1 * weight1 +
														inverseW2 * weight2) };

					const Vertex_Out vertexOut0{ (*pVerticesOut)[index0] };
					const Vertex_Out vertexOut1{ (*pVerticesOut)[index1] };
					const Vertex_Out vertexOut2{ (*pVerticesOut)[index2] };

					const Vector2 interpPosXY{ (vertexOut0.position.GetXY() * weight0) +
												(vertexOut1.position.GetXY() * weight1) +
												(vertexOut2.position.GetXY() * weight2) };

					const ColorRGB interpColor{ ((vertexOut0.color * inverseW0 * weight0) +
												 (vertexOut1.color * inverseW1 * weight1) +
												 (vertexOut2.color * inverseW2 * weight2)) * viewSpaceDepth };

					const Vector2 interpUV{ ((vertexOut0.uv * inverseW0 * weight0) +
											 (vertexOut1.uv * inverseW1 * weight1) +
											 (vertexOut2.uv * inverseW2 * weight2)) * viewSpaceDepth };

					const Vector3 interpNormal{ (((vertexOut0.normal * inverseW0 * weight0) +
												  (vertexOut1.normal * inverseW1 * weight1) +
												  (vertexOut2.normal * inverseW2 * weight2)) * viewSpaceDepth).Normalized() };

					const Vector3 interpTangent{ (((vertexOut0.tangent * inverseW0 * weight0) +
												   (vertexOut1.tangent * inverseW1 * weight1) +
												   (vertexOut2.tangent * inverseW2 * weight2)) * viewSpaceDepth).Normalized() };

					const Vector3 interpViewDirection{ (((vertexOut0.viewDirection * inverseW0 * weight0) +
														 (vertexOut1.viewDirection * inverseW1 * weight1) +
														 (vertexOut2.viewDirection * inverseW2 * weight2)) * viewSpaceDepth).Normalized() };

					Vertex_Out shadeInfo{ Vector4 {interpPosXY.x, interpPosXY.y, depth, viewSpaceDepth},
											interpColor,
											interpUV,
											interpNormal,
											interpTangent,
											interpViewDirection };

					// Shade
					finalColor = PixelShadingStage(settings, shadeInfo, pDiffuse, pNormal, pSpecular, pGlossiness);
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

bool RasterizerSoftware::IsInsideFrustrum(const Vector4& vertex) const
{
	// x should be between -1 and 1
	if (vertex.x < -1 || vertex.x > 1)
		return false;

	// y should be between -1 and 1
	if (vertex.y < -1 || vertex.y > 1)
		return false;

	// z should be between 0 and 1
	if (vertex.z < 0 || vertex.z > 1)
		return false;

	// inside frustrum
	return true;
}

float RasterizerSoftware::Remap(float value, float min, float max) const
{
	if (value <= min)
		return 0;
	if (value >= max)
		return 1;

	value = (value - min) / (max - min);
	return value;
}

ColorRGB RasterizerSoftware::PixelShadingStage(const DualRasterizerSettings& settings, const Vertex_Out shadeInfo, Texture* pDiffuse, Texture* pNormal, Texture* pSpecular, Texture* pGlossiness) const
{
	// normal maps
	Vector3 sampledNormal{ shadeInfo.normal };

	if (settings.cullMode == CullMode::Front)
		sampledNormal = -sampledNormal;

	if (settings.useNormalMap)
	{
		const Vector3 binormal{ Vector3::Cross(sampledNormal, shadeInfo.tangent) };
		const Matrix tangentSpace{ Matrix{shadeInfo.tangent, binormal, sampledNormal, Vector3::Zero} };

		const ColorRGB normalSampleColor{ pNormal->Sample(shadeInfo.uv) };
		sampledNormal = Vector3{ normalSampleColor.r, normalSampleColor.g, normalSampleColor.b };
		sampledNormal = 2.f * sampledNormal - Vector3{ 1, 1, 1 }; // from range [0, 1] to [-1, 1]

		sampledNormal = tangentSpace.TransformVector(sampledNormal);
	}

	// observed area
	const float dotProduct{ std::max(Vector3::Dot(sampledNormal, -m_LightDirection), 0.f) };
	const ColorRGB observedArea = { dotProduct, dotProduct, dotProduct };

	switch (settings.shadingMode)
	{
	case ShadingMode::ObservedArea:
	{
		return observedArea;
	}
	case ShadingMode::Diffuse:
	{
		// lambert diffuse
		const float reflection{ 1.f };
		const ColorRGB lambert{ pDiffuse->Sample(shadeInfo.uv) * reflection / PI };
		return lambert * m_LightIntensity * observedArea;
	}
	case ShadingMode::Specular:
	{
		// phong
		const float specularReflection{ 1.f };
		const float shininess{ 25.f };
		const ColorRGB specularColor{ pSpecular->Sample(shadeInfo.uv) };
		const float glossiness{ pGlossiness->Sample(shadeInfo.uv).r * shininess };	// grayscale map

		const Vector3 reflect{ Vector3::Reflect(m_LightDirection, sampledNormal) };
		const float cosAngle{ std::max(Vector3::Dot(reflect, -shadeInfo.viewDirection), 0.f) };
		const float phongValue{ specularReflection * powf(cosAngle, glossiness) };
		ColorRGB phong{ phongValue, phongValue, phongValue };
		phong *= specularColor;

		return phong * observedArea;
	}
	case ShadingMode::Combined:
	{
		// lambert diffuse
		const float reflection{ 1.f };
		const ColorRGB lambert{ pDiffuse->Sample(shadeInfo.uv) * reflection / PI };

		// phong
		const float specularReflection{ 1.f };
		const float shininess{ 25.f };
		const ColorRGB specularColor{ pSpecular->Sample(shadeInfo.uv) };
		const float glossiness{ pGlossiness->Sample(shadeInfo.uv).r * shininess };	// grayscale map

		const Vector3 reflect{ Vector3::Reflect(m_LightDirection, sampledNormal) };
		const float cosAngle{ std::max(Vector3::Dot(reflect, -shadeInfo.viewDirection), 0.f) };
		const float phongValue{ specularReflection * powf(cosAngle, glossiness) };
		ColorRGB phong{ phongValue, phongValue, phongValue };
		phong *= specularColor;

		const ColorRGB ambient{ 0.025f, 0.025f, 0.025f };
		return observedArea * (lambert * m_LightIntensity + phong + ambient);
	}
	}
}

