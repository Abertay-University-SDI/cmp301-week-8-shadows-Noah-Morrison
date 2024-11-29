#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

// Define number of lights
#define NUM_LIGHTS 3

class LightShader : public BaseShader
{
private:
	struct LightData
	{
		// Generic variables
		XMFLOAT4 diffuse;
		XMFLOAT3 position;
		float padding1;
		XMFLOAT3 direction;

		// Spotlight variables
		float range;
		float cone;
		XMFLOAT3 attenuation;

		// Specular variables
		XMFLOAT4 specularColour;
		float specularPower;

		// Type variable
		int type;

		// Padding
		XMFLOAT2 padding2;
	};

	struct LightBufferType
	{
		// Global ambient light
		XMFLOAT4 ambient;

		LightData lights[NUM_LIGHTS];
	};

	struct CameraBufferType
	{
		XMFLOAT3 position;
		float padding;
	};

public:
	LightShader(ID3D11Device* device, HWND hwnd);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, XMFLOAT4 ambient, Light* lights[NUM_LIGHTS], int types[NUM_LIGHTS], Camera* camera);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
};



