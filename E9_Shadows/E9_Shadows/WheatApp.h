// Application.h
#ifndef _WHEATAPP_H
#define _WHEATAPP_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "WheatShader.h"

class WheatApp : public BaseApplication
{
public:

	WheatApp();
	~WheatApp();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* planeMesh;

	WheatShader* wheatShader;
	AModel* wheatModel;
	AModel* barnModel;
	AModel* postModel;

	bool wheat;

	XMFLOAT3 wheatPositions[NUM_WHEAT_CLUMPS];
	XMFLOAT3 wheatScales[NUM_WHEAT_CLUMPS];
	XMFLOAT4 wheatRotations[NUM_WHEAT_CLUMPS];

	float totalTime;
	float deltaTime;

	XMMATRIX barnScaleMatrix = XMMatrixScaling(0.16f, 0.16f, 0.16f);
	XMMATRIX barnRotationMatrix = XMMatrixRotationY(27.0f * 0.0174532f);

	XMMATRIX postScaleMatrix;
	XMMATRIX postRotationMatrix;

	// Barn model controls
	XMFLOAT3 translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float scaling = 1.0f;
	float rotation = 0.0f;
};

#endif

