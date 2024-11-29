// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry

// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    //return texture0.Sample(Sampler0, input.tex);
	
	// TEMP - Using texutre shader as light source debug shader - setting all colour to white
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}