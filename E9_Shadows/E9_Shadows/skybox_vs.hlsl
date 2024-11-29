cbuffer MatrixBuffer : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float4 position : POSITION;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD;
};

OutputType main(InputType input)
{  
    OutputType output;
    
    float3 position = mul(input.position.xyz, (float3x3) viewMatrix);

    output.position = mul(float4(position, 1.0f), projectionMatrix);

    output.tex = input.position;

    return output;
}