
// global variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : World;
float4x4 gViewInverseMatrix : ViewInverse;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float gLightIntensity = 7.0f;

float gPi = 3.141592653589793f;
float gShininess = 25.0f;
float4 gAmbient = float4(0.025f, 0.025f, 0.025f, 1.0f);

SamplerState gSamplerState;
RasterizerState gRasterizerState;

// other states
BlendState gBlendState
{
	BlendEnable[0] = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = 1;
	DepthFunc = less;
	StencilEnable = false;
};

// --------------------------------------------
// Input/Output Structures
// --------------------------------------------

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 ViewDirection : VIEWDIRECTION;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	//float3 Color : COLOR;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float4 WorldPosition : WORLDPOS;
};

// --------------------------------------------
// Vertex Shader
// --------------------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.UV = input.UV;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
	output.WorldPosition = mul(float4(input.Position, 1.0f), gWorldMatrix);
	return output;
};

// --------------------------------------------
// Helper Functions
// --------------------------------------------
float4 Lambert(float diffuseReflection, float4 diffuseColor)
{
	return ((diffuseColor * diffuseReflection) / gPi);
}

float4 Phong(float4 color, float specularReflection, float exponent, float3 lightDir, float3 viewDir, float3 normal)
{
	float3 reflectVector = reflect(lightDir, normal);
	float angleViewReflect = saturate(dot(reflectVector, viewDir));

	float phongValue = specularReflection * pow(angleViewReflect, exponent);

	return color * phongValue;
}

// --------------------------------------------
// Pixel Shader
// --------------------------------------------

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	// normal map
	float3 sampledNormal = input.Normal;

	float3 binormal = normalize(cross(sampledNormal, input.Tangent));
	float4x4 tangentSpace = float4x4(	float4(input.Tangent, 0.0f), 
										float4(binormal, 0.0f), 
										float4(sampledNormal, 0.0f), 
										float4(0.0f, 0.0f, 0.0f, 1.0f));
	
	sampledNormal = gNormalMap.Sample(gSamplerState, input.UV).rgb;
	sampledNormal = 2.0f * sampledNormal - float3(1.0f, 1.0f, 1.0f);
	sampledNormal = normalize(mul(float4(sampledNormal, 0.0f), tangentSpace)).rgb;

	// observedArea
	float observedArea = saturate(dot(sampledNormal, -gLightDirection));
	float3 observedAreaColor = float3(observedArea, observedArea, observedArea);

	// lambert diffuse
	float diffuseReflection = 1.f;
	float4 diffuse = Lambert(diffuseReflection, gDiffuseMap.Sample(gSamplerState, input.UV));

	// phong
	float specularReflection = 1.f;
	float exponent = gGlossinessMap.Sample(gSamplerState, input.UV).r * gShininess;
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);
	float4 specular = Phong(gSpecularMap.Sample(gSamplerState, input.UV), specularReflection, exponent, gLightDirection, viewDirection, sampledNormal);

	return (diffuse * gLightIntensity + specular + gAmbient) * observedArea;
};

// --------------------------------------------
// Technique
// --------------------------------------------

technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
};
