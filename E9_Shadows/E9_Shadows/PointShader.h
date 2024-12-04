#ifndef _POINTSHADER_H_
#define _POINTSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;

#define NUM_LIGHTS 19 // One for each 'face' of the light

class PointShader : public BaseShader
{
private:
	struct LightMatrices
	{
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	struct LightData
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 position;
		float padding;
		XMFLOAT3 direction;
		int type;
	};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;

		LightMatrices lightMatrices[NUM_LIGHTS];
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT3 attenuation;
		float padding;

		LightData lights[NUM_LIGHTS];
	};

public:

	PointShader(ID3D11Device* device, HWND hwnd);
	~PointShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMaps[NUM_LIGHTS], XMFLOAT4 ambient, Light* lights[NUM_LIGHTS], XMFLOAT3 attenuation, int types[NUM_LIGHTS]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
};

#endif
