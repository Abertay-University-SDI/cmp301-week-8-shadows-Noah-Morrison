// Light pixel shader

// Define light types
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

// Define number of lights
#define NUM_LIGHTS 3

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

//cbuffer LightBuffer : register(b0)
//{
//	// Generic variables
//	float4 ambient;
//	float4 diffuse;
//	float3 position;
	
//	// Specular variables
//    float specularPower;
//    float4 specularColour;
	
//	// Spotlight variables
//    float range;
//    float3 direction;
//    float cone;
//    float3 attenuation;
	
//	// Type variable
//    int type;
//};

struct Light
{
    // Generic variables
    float4 diffuse;
    float3 position;
    float padding1;
    float3 direction;
	
	// Spotlight variables
    float range;
    float cone;
    float3 attenuation;

    // Specular variables - TEMP - To make general as to allow per material specular highlights
    float4 specularColour;
    float specularPower;
	
	// Type variable
    int type;

    // Padding
    float2 padding2;
};

// TODO - Update light buffer
cbuffer LightBuffer : register(b0)
{
    // Global ambient light
    float4 ambient;

    
    Light lights[NUM_LIGHTS];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	
    float3 worldPosition : TEXCOORD1;
};

// Redundant Lighting Function
//// Calculate lighting intensity based on direction and normal. Combine with light colour.
//float4 calculateLighting(float3 lightDirection, float3 normal, float4 lightDiffuse)
//{
//	float intensity = saturate(dot(normal, lightDirection));
//	float4 colour = saturate(lightDiffuse * intensity);
//	return colour;
//}

// Calclate specular colour
float4 calcSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4
specularColour, float specularPower)
{
// blinn-phong specular calculation
    float3 halfway = normalize(lightDirection + viewVector);
    float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
    return saturate(specularColour * specularIntensity);
}

float4 calcDirectional(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = diffuse * intensity;
    return colour;
}

float4 calcPoint(float3 lightPosition, float3 normal, float4 diffuse, float3 worldPosition)
{
    float3 direction = normalize(lightPosition - worldPosition);
    float intensity = saturate(dot(normal, direction));
    float4 colour = diffuse * intensity;
    return colour;
}

float4 calcSpot(float3 position, float3 normal, float4 diffuse, float3 direction, float coneAngle, float3 attenuationVector, float3 worldPosition)
{
    float3 lightVector = normalize(position - worldPosition);
    float distance = length(lightVector);
    float spotEffect = saturate(dot(lightVector, direction));
    float coneEffect = cos(coneAngle);
    
    // Initially set intensity to null and keep that way if following check is false
    float intensity = 0.0f;
    // Check if pixel is within the spotlight cone
    if (spotEffect >= coneEffect)
    {
        // Calculate diffuse intensity
        float intensity = saturate(dot(normal, lightVector));
        
        // Calculate attenuation based on distance
        float attenuation = 1.0 / (attenuationVector.x + attenuationVector.y * distance + attenuationVector.z * distance * distance);
        
        intensity = intensity * attenuation * spotEffect;
    }
    
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calcLighting(Light light, float3 normal, float3 worldPosition)
{
    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    
    switch (light.type)
    {
        case LIGHT_TYPE_DIRECTIONAL:
            result = calcDirectional(light.direction, normal, light.diffuse);
            break;
        case LIGHT_TYPE_POINT:
            result = calcPoint(light.position, normal, light.diffuse, worldPosition);
            break;
        case LIGHT_TYPE_SPOT:
            result = calcSpot(light.position, normal, light.diffuse, light.direction, light.cone, light.attenuation, worldPosition);
            break;
    }
    return result;
}


float4 main(InputType input) : SV_TARGET
{
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    float4 textureColour = texture0.Sample(sampler0, input.tex);
    
    // TODO - Imbed this into individual light functions to remove lightVector parameter
    //float3 lightVector = normalize(position - input.worldPosition);
	//float4 lightColour = ambient + calculateLighting(lightVector, input.normal, diffuse);
    
    // TODO - Implement Specular Highlight Per Material
    //float4 specularLight = calcSpecular(lightVector, input.normal, input.worldPosition, specularColour, specularPower);
	
    //float4 lightColour = ambient + calcLighting(type, lightVector, position, diffuse, input.normal, input.worldPosition, direction, cone, attenuation);
    
    float4 totalLightContribution = float4(0.0, 0.0, 0.0, 0.0);

    
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        totalLightContribution += calcLighting(lights[i], input.normal, input.worldPosition);
    }
    
    totalLightContribution += ambient;

    return saturate(totalLightContribution * textureColour);
    
    //return lightColour * textureColour;
    
    // TODO - Implement Specular Highlight Per Material
    //return (lightColour * textureColour) + specularLight;
    
    //return specularColour;
}


