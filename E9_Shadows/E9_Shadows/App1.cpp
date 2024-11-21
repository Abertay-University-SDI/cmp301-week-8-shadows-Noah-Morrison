// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	planeMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"checkerboard", L"res/checkerboard.png");

	// Create addition geometry objects
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	triangleMesh = new TriangleMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 8192;
	int shadowmapHeight = 8192;
	int sceneWidth = 200;
	int sceneHeight = 200;

	// This is your shadow map
	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Ortho mesh to view light POV
	//orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 512, 512, -screenWidth / 1.3, screenHeight / 1.8);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(),  200, 200, -screenWidth / 2.7, screenHeight / 2.7);

	// Light render texture
	//lightTexture = new RenderTexture(renderer->getDevice(), shadowmapWidth, shadowmapHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Configure directional light
	light = new Light();
	light->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	light->setPosition(-lightDirection.x * 10.f, -lightDirection.y * 10.f, -lightDirection.z * 10.f);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);


	// TODO - implement projection matrix to create frustum
	//light->generateProjectionMatrix(0.1f, 100.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	// Get delta time
	deltaTime = timer->getTime();

	// Animate sphere by offsetting it back and forth on the X-axis
	if ((cubeOffset + deltaTime) >= 20.f || (cubeOffset + deltaTime <= -20.f))
	{
		velocity = -velocity;
	};

	cubeOffset += deltaTime * velocity;

	light->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	light->setPosition(-lightDirection.x * 30.f, -lightDirection.y * 30.f, -lightDirection.z * 30.f);

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	light->generateViewMatrix();
	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();
	// TODO - implement projection matrix to create frustum
	//XMMATRIX lightProjectionMatrix = light->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	planeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Add cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(cubeOffset, 7.f, 10.f);
	// Render cube
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Add triangle
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(-20.f, 10.f, 5.f);
	// Render triangle - TODO - Fix issue where shadow displays as an outline and not filled - May be a light and not model error
	triangleMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), triangleMesh->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	planeMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(cubeOffset, 7.f, 10.f);
	cubeMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Render triangle
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(-20.f, 10.f, 5.f);
	triangleMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), triangleMesh->getIndexCount());

	// Render sphere (light position)
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(-lightDirection.x * 30.f , -lightDirection.y * 30.f, -lightDirection.z * 30.f);
	sphereMesh->sendData(renderer->getDeviceContext());
	setAmbientAndDiffuse(light, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"checkerboard"), shadowMap->getDepthMapSRV(), light);
	setAmbientAndDiffuse(light, { 0.3f, 0.3f, 0.3f }, { 1.0f, 1.0f, 1.0f });
	shadowShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());

	// Render light POV - TODO - Why isn't this working anymore?
	renderer->setZBuffer(false);

	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	// Directional Light Controls
	if (ImGui::CollapsingHeader("Light Direction")) {
		ImGui::Text("Angle Values:");
		ImGui::SliderFloat("X##Direction", &lightAngle.x, -90.0f, 90.0f);
		ImGui::SliderFloat("Y##Direction", &lightAngle.y, -90.0f, 90.0f);
		ImGui::SliderFloat("Z##Direction", &lightAngle.z, -90.0f, 90.0f);

		// Don't all to equal zero simultaneously
		if (abs(lightAngle.x) + abs(lightAngle.y) + abs(lightAngle.z) == 0.0f) {
			lightAngle.x = 1.0f;
		}

		lightDirection.x = sin(lightAngle.x * 0.0174532f);
		lightDirection.y = sin(lightAngle.y * 0.0174532f);
		lightDirection.z = sin(lightAngle.z * 0.0174532f);
	}

	// Time
	if (ImGui::CollapsingHeader("Time")) {
		ImGui::Text("Time Values:");
		ImGui::SliderFloat("Time##Direction", &time, 0.0f, 360.0f);

		lightDirection.x = sin(time * 0.0174532f);
		lightDirection.y = cos(time * 0.0174532f);
		lightDirection.z = sin(-90.0f * 0.0174532f);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::setAmbientAndDiffuse(Light* light, XMFLOAT3 ambient, XMFLOAT3 diffuse)
{
	light->setAmbientColour(ambient.x, ambient.y, ambient.z, 1.0f);
	light->setDiffuseColour(diffuse.x, diffuse.y, diffuse.z, 1.0f);
}

