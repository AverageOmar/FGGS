//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register ( t0 );
SamplerState samLinear : register ( s0 );

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
	float2 Tex : TEXCOORD0;

	float3 NormalW : NORMAL;
	float4 PosW : POSITION;

};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 NormalL : NORMAL, float2 Tex : TEXCOORD0) //direct correlation in order
{
	float4 textureColour = txDiffuse.Sample(samLinear, Tex);

    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul( Pos, World );
	
	output.PosW = output.Pos;

	output.Pos = mul( output.Pos, View );
	output.Pos = mul(output.Pos, Projection);

	// leave in VS
	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
	output.NormalW = normalize(normalW);
	// end

	output.Tex = Tex;
	//--
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float3 lightVec = normalize(LightVecW);
	float3 normalW = normalize(input.NormalW);

	float3 toEye = normalize(EyePosW - input.PosW.xyz); // Move to PS

	float3 r = reflect(-lightVec, normalW);
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);
	float diffuseAmount = max(dot(lightVec, normalW), 0.0f);

	if (diffuseAmount <= 0.0f)
	{
		specularAmount = 0.0f;
	}

	float3 ambient = (AmbientMtrl * AmbientLight).rgb;
	float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;
	float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

	float4 color;

	color.rgb = ambient + diffuse + specular;
	color.a = DiffuseMtrl.a;

    return color;
}
