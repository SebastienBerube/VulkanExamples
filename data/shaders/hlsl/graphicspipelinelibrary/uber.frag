struct VSOutput
{
	float4 Pos : SV_POSITION;
	[[vk::location(0)]] float3 Normal : NORMAL0;
	[[vk::location(1)]] float3 Color : COLOR0;
	[[vk::location(2)]] float3 ViewVec : TEXCOORD0;
	[[vk::location(3)]] float3 LightVec : TEXCOORD1;
};

[[vk::constant_id(0)]] const int LIGHTING_MODEL = 0;

float4 main(VSOutput In) : SV_TARGET
{
	float4 outColor = float4(0,0,0,1);
	switch (LIGHTING_MODEL) {
		case 0: // Phong
		{
			float3 ambient = In.Color * float3(0.25,0.25,0.25);
			float3 N = normalize(In.Normal);
			float3 L = normalize(In.LightVec);
			float3 V = normalize(In.ViewVec);
			float3 R = reflect(-L, N);
			float3 diffuse = max(dot(N, L), 0.0) * In.Color;
			float3 specular = pow(max(dot(R, V), 0.0), 32.0) * float3(1,1,1)*0.75;
			outColor = float4(ambient + diffuse * 1.75 + specular, 1.0);		
			break;
		}
		case 1: // Toon
		{

			float3 N = normalize(In.Normal);
			float3 L = normalize(In.LightVec);
			float intensity = dot(N,L);
			float3 color;
			if (intensity > 0.98)
				color = In.Color * 1.5;
			else if  (intensity > 0.9)
				color = In.Color * 1.0;
			else if (intensity > 0.5)
				color = In.Color * 0.6;
			else if (intensity > 0.25)
				color = In.Color * 0.4;
			else
				color = In.Color * 0.2;
			outColor = float4(color,1.0);
			break;
		}
		case 2: // No shading
		{
			outColor = float4(In.Color,1.0);
			break;
		}
		case 3: // Greyscale
		{
			outColor = float4(float3(1,1,1)*dot(In.Color.rgb, float3(0.299, 0.587, 0.114)),1.0);
			break;
		}
	}

	// Scene is dark, brigthen up a bit
	outColor.rgb *= 1.25;
	return outColor;
}