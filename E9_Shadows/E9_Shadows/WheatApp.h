// Application.h
#ifndef _WHEATAPP_H
#define _WHEATAPP_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
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
};

#endif

