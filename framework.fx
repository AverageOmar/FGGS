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
    float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 NormalL : NORMAL ) //direct correlation in order
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul( Pos, World );
    output.Pos = mul( output.Pos, View );
	//--
	EyePosW = output.Pos - World;
	float3 toEye = normalize(EyePosW - output.Pos.xyz);

	output.Pos = mul(output.Pos, Projection);


	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
	normalW = normalize(normalW);

	float3 lightVec = normalize(LightVecW);

	float diffuseAmount = max(dot(lightVec, normalW), 0.0f);
	float3 ambient = AmbientMtrl * AmbientLight;
	float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight);

	output.Color.rgb = ambient + diffuse;
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
