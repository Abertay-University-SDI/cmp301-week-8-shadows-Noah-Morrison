// Application.h
#ifndef _POINTLIGHTAPP_H
#define _POINTLIGHTAPP_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"

class PointLightApp : public BaseApplication
{
public:

	PointLightApp();
	~PointLightApp();
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


