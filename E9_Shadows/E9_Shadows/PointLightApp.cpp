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
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// Initialise mesh objects for lamppost
	postModel = new AModel(renderer->getDevice(), "res/lamp_post_post.obj");
	lampMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	lightShader = new LightShader(renderer->getDevice(), hwnd);

	// Initialise lights
	lights[0] = new Light();
	lights[0]->setDiffuseColour(1.0f, 0.0f, 0.0f, 0.0f);
	lights[0]->setPosition(0.0f, 0.0f, 0.0f);
	lights[0]->setDirection(-1.0f, 1.0f, 0.0f);
	types[0] = 0;

	lights[1] = new Light();
	lights[1]->setDiffuseColour(1.0f, 1.0f, 1.0f, 0.0f);
	lights[1]->setPosition(0.0f, 0.0f, 0.0f); // Position dynamically updates with lamp
	lights[1]->setDirection(1.0f, 0.0f, 0.0f);
	types[1] = 1;

	lights[2] = new Light();
	lights[2]->setDiffuseColour(0.0f, 0.0f, 1.0f, 0.0f);
	lights[2]->setPosition(0.0f, 0.0f, 0.0f);
	lights[2]->setDirection(1.0f, 1.0f, 0.0f);
	types[2] = 0;

}

PointLightApp::~PointLightApp()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// TODO - Release the Direct3D object.

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
	lights[1]->setPosition((translation.x + lampOffset.x * scaling[0]), (translation.y + lampOffset.y * scaling[0]), (translation.z + lampOffset.z * scaling[0]));

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
	finalPass();

	return true;
}

void PointLightApp::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render Plane
	planeMesh->sendData(renderer->getDeviceContext());
	lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), ambient, lights, types, camera);
	lightShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render Post
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(translation.x / scaling[0], translation.y / scaling[0], translation.z / scaling[0]);
	postScaleMatrix = XMMatrixScaling(scaling[0], scaling[0], scaling[0]);
	worldMatrix = XMMatrixMultiply(worldMatrix, postScaleMatrix);
	postModel->sendData(renderer->getDeviceContext());
	lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), ambient, lights, types, camera);
	lightShader->render(renderer->getDeviceContext(), postModel->getIndexCount());

	// Render lamp
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation((translation.x + lampOffset.x * scaling[0]) / scaling[1], (translation.y + lampOffset.y * scaling[0]) / scaling[1], (translation.z + lampOffset.z * scaling[0]) / scaling[1]);
	lampScaleMatrix = XMMatrixScaling(scaling[1], scaling[1], scaling[1]);
	worldMatrix = XMMatrixMultiply(worldMatrix, lampScaleMatrix);
	lampMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), lampMesh->getIndexCount());

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
		ImGui::SliderFloat("X##Translation", &translation.x, -100.0f, 100.0f);
		ImGui::SliderFloat("Y##Translation", &translation.y, -100.0f, 100.0f);
		ImGui::SliderFloat("Z##Translation", &translation.z, -100.0f, 100.0f);

		ImGui::Text("Scaling:");
		ImGui::SliderFloat("Post##Scaling", &scaling[0], 0.0f, 5.0f);
		ImGui::SliderFloat("Lamp##Scaling", &scaling[1], 0.0f, 5.0f);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
