// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

	void setAmbientAndDiffuse(Light* light, XMFLOAT3 ambient, XMFLOAT3 diffuse);

private:
	TextureShader* textureShader;
	PlaneMesh* planeMesh;

	// Additional geometry
	CubeMesh* cubeMesh;
	TriangleMesh* triangleMesh;
	SphereMesh* sphereMesh;

	OrthoMesh* orthoMesh;
	RenderTexture* lightTexture;

	Light* light;
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap;

	float deltaTime;
	float cubeOffset = 0.0f;
	float velocity = 5.0f;

	// Add directional light controls
	XMFLOAT3 lightAngle = { 90.0f, -90.0f, 0.0f };
	XMFLOAT3 lightDirection = { 1.0f, -1.0f, 0.0f };

	float time = 0.0f;
};

#endif