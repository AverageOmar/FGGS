//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 DiffuseMtrl;
	float4 DiffuseLight;
	float4 AmbientMtrl;
	float4 AmbientLight;
	float4 SpecularMtrl;
	float4 SpecularLight;
	float SpecularPower;
	float3 EyePosW;
	float3 LightVecW;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0; // Replace with Normal in World space
	// Add position in world space
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 NormalL : NORMAL ) //direct correlation in order
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul( Pos, World );
	//--
	float3 toEye = normalize(EyePosW - output.Pos.xyz); // Move to PS

	output.Pos = mul( output.Pos, View );
	output.Pos = mul(output.Pos, Projection);

	float3 lightVec = normalize(LightVecW);

	// leave in VS
	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
	normalW = normalize(normalW);
	// end

	float3 r = reflect(-lightVec, normalW);
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);

	float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

	float diffuseAmount = max(dot(lightVec, normalW), 0.0f);
	float3 ambient = (AmbientMtrl * AmbientLight).rgb;
	float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;

	output.Color.rgb = ambient + diffuse + specular;
	output.Color.a = DiffuseMtrl.a;

	//--
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
    return input.Color;
}
