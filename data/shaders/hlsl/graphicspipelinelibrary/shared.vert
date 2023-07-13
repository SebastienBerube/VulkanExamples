struct VSInput
{
	[[vk::location(0)]] float3 Pos : POSITION0;
	[[vk::location(1)]] float3 Normal : NORMAL0;
	[[vk::location(2)]] float3 Color : COLOR0;
};

struct UBO
{
	float4x4 projection;
	float4x4 model;
	float4 lightPos;
};

cbuffer ubo : register(b0) { UBO ubo; }

struct VSOutput
{
	float4 Pos : SV_POSITION;
	[[vk::location(0)]] float3 Normal : NORMAL0;
	[[vk::location(1)]] float3 Color : COLOR0;
	[[vk::location(2)]] float3 ViewVec : TEXCOORD0;
	[[vk::location(3)]] float3 LightVec : TEXCOORD1;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	
	output.Normal = input.Normal;    //Same as glsl version
	output.Color = input.Color;      //Same as glsl version
	
	float4 pos = float4(input.Pos, 1.0); //Same as glsl version
	
	output.Pos = mul(ubo.projection, mul(ubo.model, pos)); //Same as glsl version
	
	pos = mul(ubo.model, pos);                    //Same as glsl version, but weird (why multiply again with model matrix?).
	 
	output.Normal = mul((float3x3)ubo.model, input.Normal); //Same as glsl version
	
	float3 lPos = ubo.lightPos.xyz;               //Same as glsl version
	
	output.LightVec = lPos - pos.xyz; //Same as glsl version, but weird (because pos was multiplied twice)
	output.ViewVec = - pos.xyz; //Same as glsl version, but weird (is pos in view space?)
	
	//outViewVec = -pos.xyz;
    return output;
}