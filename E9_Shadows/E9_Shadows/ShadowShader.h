// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;

// Define number of lights
#define NUM_LIGHTS 2

class ShadowShader : public BaseShader
{
private:
	struct LightMatrices
	{
		// Matrices
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	struct LightData
	{
		// Generic variables
		XMFLOAT4 diffuse;
		XMFLOAT3 position;
		float padding1;
		XMFLOAT3 direction;
		float padding2;
	};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		
		LightMatrices lightMatrices[NUM_LIGHTS];
	};

	// Old
	//struct LightBufferType
	//{
	//	XMFLOAT4 ambient;
	//	XMFLOAT4 diffuse;
	//	XMFLOAT3 direction;
	//	float padding1;
	//	XMFLOAT3 position;
	//	float padding2;
	//};

	// New
	struct LightBufferType
	{
		XMFLOAT4 ambient;

		LightData lights[NUM_LIGHTS];
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMaps[NUM_LIGHTS], XMFLOAT4 ambient, Light* lights[NUM_LIGHTS]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
};

#endif