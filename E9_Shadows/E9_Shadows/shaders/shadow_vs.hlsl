// Define number of lights
#define NUM_LIGHTS 2

struct LightMatrices
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};


cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	
    LightMatrices lightMatrices[NUM_LIGHTS];
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPos[NUM_LIGHTS] : TEXCOORD1;
};


OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
	// Calculate the position of the vertice as viewed by the light source.
        output.lightViewPos[i] = mul(input.position, worldMatrix);
        output.lightViewPos[i] = mul(output.lightViewPos[i], lightMatrices[i].lightViewMatrix);
        output.lightViewPos[i] = mul(output.lightViewPos[i], lightMatrices[i].lightProjectionMatrix);
    }

    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

	return output;
}