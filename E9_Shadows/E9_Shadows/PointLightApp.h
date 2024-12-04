// PointLightApp.h
#ifndef _POINTLIGHTAPP_H
#define _POINTLIGHTAPP_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "DepthShader.h"
#include "PointShader.h"

class PointLightApp : public BaseApplication
{
public:

	PointLightApp();
	~PointLightApp();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass(int index);
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;

	DepthShader* depthShader;
	PointShader* pointShader;

	PlaneMesh* planeMesh;
	CubeMesh* cubeMesh;

	AModel* postModel;
	SphereMesh* lampMesh;
	Light* lights[NUM_LIGHTS];
	int types[NUM_LIGHTS];

	XMFLOAT4 ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
	XMFLOAT3 attenuation = { 1.0f, 0.001f, 0.001f };

	// Model Controls
	XMFLOAT3 translation = XMFLOAT3(30.0f, 7.0f, 50.0f);
	float scaling[2] = { 5.0f, 2.5f };

	// Square Controls
	XMFLOAT3 translationC = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float scalingC = 1.0f;

	// Light Controls
	XMFLOAT3 positionG = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 positionB = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMFLOAT3 lampOffset = XMFLOAT3(3.5f, 4.9f, 0.0f);

	XMMATRIX postScaleMatrix;
	XMMATRIX lampScaleMatrix;

	ShadowMap* shadowMaps[NUM_LIGHTS];
	ID3D11ShaderResourceView* depthMaps[NUM_LIGHTS];
};

#endif


