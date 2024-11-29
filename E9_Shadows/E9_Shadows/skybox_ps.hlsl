//TextureCube EnviornmentMap : register(t0); // TODO - Could just make a skybox without using a texture?
//SamplerState LinearSampler : register(s0); // TODO - Is this sampler linear?

// Max value that we can store in an fp16 buffer (actually a little less so that we have room for error, real max is 65504)
static const float FP16Max = 65000.0f;

cbuffer PSConstants : register(b0)
{
    float3 sunDirection;
    bool enableSun;
    float4 skyColour;
    float4 sunColour;
    float cosSunAngularRadius;
    float intensity;
}

struct InputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD; // TODO - Sampler texture should be based of a vector?
};


float4 main(InputType input) : SV_TARGET
{
    //float4 colour = EnviornmentMap.Sample(LinearSampler, input.tex);
    float4 colour = skyColour;
    
    float3 dir = normalize(input.tex);
    if(enableSun)
    {
        float cosSunAngle = dot(dir, sunDirection);
        if (cosSunAngle >= cosSunAngularRadius)
        {
            colour = sunColour;
        }
    }
    
    colour *= intensity;
    colour = saturate(colour);
    //colour = clamp(colour, 0.0f, FP16Max);
    
    return colour;
}