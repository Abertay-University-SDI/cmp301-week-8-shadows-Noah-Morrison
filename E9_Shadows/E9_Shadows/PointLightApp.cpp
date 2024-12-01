#include "PointLightApp.h"

PointLightApp::PointLightApp()
{

}

void PointLightApp::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	planeMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"checkerboard", L"res/checkerboard.png");

	// Initialise mesh objects for lamppost
	postModel = new AModel(renderer->getDevice(), "res/lamp_post_post.obj");
	lampMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	pointShader = new PointShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 8192;
	int shadowmapHeight = 8192;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		shadowMaps[i] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	}

	// Initialise lights
	lights[0] = new Light();
	lights[0]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	lights[0]->setDirection(-1.0f, 0.0f, 0.0f);
	lights[0]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp

	lights[1] = new Light();
	lights[1]->setDiffuseColour(0.0f, 1.0f, 0.0f, 1.0f);
	lights[1]->setDirection(1.0f, 0.0f, 0.0f);
	lights[1]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp

	lights[2] = new Light();
	lights[2]->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f);
	lights[2]->setDirection(0.0f, -1.0f, 0.0f);
	lights[2]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp

	lights[3] = new Light();
	lights[3]->setDiffuseColour(1.0f, 1.0f, 0.0f, 1.0f);
	lights[3]->setDirection(0.0f, 1.0f, 0.0f);
	lights[3]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp

	lights[4] = new Light();
	lights[4]->setDiffuseColour(1.0f, 0.0f, 1.0f, 1.0f);
	lights[4]->setDirection(0.0f, 0.0f, -1.0f);
	lights[4]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp

	lights[5] = new Light();
	lights[5]->setDiffuseColour(0.0f, 1.0f, 1.0f, 1.0f);
	lights[5]->setDirection(0.0f, 0.0f, 1.0f);
	lights[5]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp
}

PointLightApp::~PointLightApp()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
}


bool PointLightApp::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// White point light is at the same position as the centre of the lamp sphere
	//lights[0]->setPosition((translation.x + lampOffset.x * scaling[0]), (translation.y + lampOffset.y * scaling[0]), (translation.z + lampOffset.z * scaling[0]));

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		lights[i]->setPosition((translation.x + lampOffset.x * scaling[0]), (translation.y + lampOffset.y * scaling[0]), (translation.z + lampOffset.z * scaling[0]));
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool PointLightApp::render()
{
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		depthPass(i);
	}

	finalPass();

	return true;
}

void PointLightApp::depthPass(int index)
{
	// Set the render target to be the render to texture.
	shadowMaps[index]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	lights[index]->generateViewMatrix();
	lights[index]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

	XMMATRIX worldMatrix;
	XMMATRIX lightViewMatrix = lights[index]->getViewMatrix();
	XMMATRIX lightProjectionMatrix = lights[index]->getProjectionMatrix();

	// Render Plane
	worldMatrix = renderer->getWorldMatrix();
	planeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render Post
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(translation.x / scaling[0], translation.y / scaling[0], translation.z / scaling[0]);
	postScaleMatrix = XMMatrixScaling(scaling[0], scaling[0], scaling[0]);
	worldMatrix = XMMatrixMultiply(worldMatrix, postScaleMatrix);
	postModel->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), postModel->getIndexCount());

	// Render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(translationC.x / scalingC, translationC.y / scalingC, translationC.z / scalingC));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(scalingC, scalingC, scalingC));
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void PointLightApp::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		depthMaps[i] = shadowMaps[i]->getDepthMapSRV();
	}

	worldMatrix = renderer->getWorldMatrix();
	// Render Plane
	planeMesh->sendData(renderer->getDeviceContext());
	pointShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), depthMaps, ambient, lights);
	pointShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render Post
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(translation.x / scaling[0], translation.y / scaling[0], translation.z / scaling[0]);
	postScaleMatrix = XMMatrixScaling(scaling[0], scaling[0], scaling[0]);
	worldMatrix = XMMatrixMultiply(worldMatrix, postScaleMatrix);
	postModel->sendData(renderer->getDeviceContext());
	pointShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), depthMaps, ambient, lights);
	pointShader->render(renderer->getDeviceContext(), postModel->getIndexCount());

	// Render lamp
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation((translation.x + lampOffset.x * scaling[0]) / scaling[1], (translation.y + lampOffset.y * scaling[0]) / scaling[1], (translation.z + lampOffset.z * scaling[0]) / scaling[1]);
	lampScaleMatrix = XMMatrixScaling(scaling[1], scaling[1], scaling[1]);
	worldMatrix = XMMatrixMultiply(worldMatrix, lampScaleMatrix);
	lampMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), lampMesh->getIndexCount());

	// Render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(translationC.x / scalingC, translationC.y / scalingC, translationC.z / scalingC));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(scalingC, scalingC, scalingC));
	cubeMesh->sendData(renderer->getDeviceContext());
	pointShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"checkerboard"), depthMaps, ambient, lights);
	pointShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	gui();
	renderer->endScene();
}

void PointLightApp::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	// Model Controls
	if (ImGui::CollapsingHeader("Lamppost Controls")) {
		ImGui::Text("Translation:");
		ImGui::SliderFloat("X##TranslationL", &translation.x, -100.0f, 100.0f);
		ImGui::SliderFloat("Y##TranslationL", &translation.y, -100.0f, 100.0f);
		ImGui::SliderFloat("Z##TranslationL", &translation.z, -100.0f, 100.0f);

		ImGui::Text("Scaling:");
		ImGui::SliderFloat("Post##ScalingL", &scaling[0], 0.0f, 5.0f);
		ImGui::SliderFloat("Lamp##ScalingL", &scaling[1], 0.0f, 5.0f);
	}

	if (ImGui::CollapsingHeader("Cube Controls")) {
		ImGui::Text("Translation:");
		ImGui::SliderFloat("X##TranslationS", &translationC.x, -100.0f, 100.0f);
		ImGui::SliderFloat("Y##TranslationS", &translationC.y, -100.0f, 100.0f);
		ImGui::SliderFloat("Z##TranslationS", &translationC.z, -100.0f, 100.0f);

		ImGui::Text("Scaling:");
		ImGui::SliderFloat("##ScalingS", &scalingC, 1.0f, 10.0f);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}