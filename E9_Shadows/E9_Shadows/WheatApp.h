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

	XMFLOAT3 wheatPositions[NUM_WHEAT_CLUMPS];
	XMFLOAT3 wheatScales[NUM_WHEAT_CLUMPS];
	XMFLOAT4 wheatRotations[NUM_WHEAT_CLUMPS];

	float totalTime;
	float deltaTime;

	// Barn model controls
	XMFLOAT3 translation = XMFLOAT3(30.0f, 0.0f, 60.0f);
	XMFLOAT3 scaling = XMFLOAT3(0.16f, 0.16f, 0.16f);
	float angle;
	float rotation;
};

#endif

