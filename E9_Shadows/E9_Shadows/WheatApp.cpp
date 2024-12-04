#include "WheatApp.h"

WheatApp::WheatApp()
{

}

void WheatApp::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	planeMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wheat", L"res/wheat.png");
	textureMgr->loadTexture(L"barn", L"res/barn.jpg");
	textureMgr->loadTexture(L"post", L"res/lamp_post.png");
	textureMgr->loadTexture(L"checkerboard", L"res/checkerboard.png");

	// Initialise models
	wheatModel = new AModel(renderer->getDevice(), "res/wheat.obj");
	barnModel = new AModel(renderer->getDevice(), "res/barn.obj");

	postModel = new AModel(renderer->getDevice(), "res/lamp_post_post.obj");
	lampMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	wheatShader = new WheatShader(renderer->getDevice(), hwnd);

	for (int y = 0; y < 40; y++)
	{
		for (int x = 0; x < 27; x++)
		{
			wheatPositions[(y * 27) + x] = XMFLOAT3(2.4f + (x * 3.6f), 0.0f, 1.5f + (y * 2.4f));
			wheatScales[(y * 27) + x] = XMFLOAT3(0.3f, 0.3f, 0.3f);
			wheatRotations[(y * 27) + x] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

}

WheatApp::~WheatApp()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// TODO - Release the Direct3D object.

}


bool WheatApp::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Get delta + total time
	deltaTime = timer->getTime();
	totalTime += deltaTime;

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool WheatApp::render()
{
	finalPass();

	return true;
}

void WheatApp::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	planeMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render wheat clumps
	if (wheat) 
	{
		wheatModel->sendData(renderer->getDeviceContext());
		wheatShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
			textureMgr->getTexture(L"wheat"),
			wheatPositions, wheatScales, wheatRotations,
			totalTime);
		wheatShader->render(renderer->getDeviceContext(), wheatModel->getIndexCount(), NUM_WHEAT_CLUMPS);
	}

	// Render pond mesh
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(60.0f / 0.35f, 0.1f / 0.35f, 4.0f / 0.35f);
	worldMatrix = XMMatrixMultiply(worldMatrix, pondScaleMatrix);
	planeMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"checkerboard"));
	textureShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render barn
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(27.5f / 0.16f, 0.0f, 85.0f / 0.16f);
	worldMatrix = XMMatrixMultiply(worldMatrix, barnRotationMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, barnScaleMatrix);
	barnModel->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"barn"));
	textureShader->render(renderer->getDeviceContext(), barnModel->getIndexCount());

	// Render lampposts
	for (int i = 0; i < NUM_LAMPPOSTS; i++)
	{
		// Render lamppost
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(postTranslations[i].x, postTranslations[i].y, postTranslations[i].z);
		worldMatrix = XMMatrixMultiply(worldMatrix, postRotationMatrix[i]);
		postModel->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"post"));
		textureShader->render(renderer->getDeviceContext(), postModel->getIndexCount());

		// Render lamp
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation((postTranslations[i].x + lampOffset.x) / lampScale, (postTranslations[i].y + lampOffset.y) / lampScale, (postTranslations[i].z + lampOffset.z) / lampScale);
		worldMatrix = XMMatrixMultiply(worldMatrix, lampScaleMatrix);
		worldMatrix = XMMatrixMultiply(worldMatrix, postRotationMatrix[i]);
		lampMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"checkerboard"));
		textureShader->render(renderer->getDeviceContext(), lampMesh->getIndexCount());
	}

	gui();
	renderer->endScene();
}



void WheatApp::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	
	// Wheat Toggle
	ImGui::Checkbox("Wheat", &wheat);

	// Model Controls
	ImGui::Text("Translation:");
	ImGui::SliderFloat("X##Translation", &translation.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Y##Translation", &translation.y, -10.0f, 10.0f);
	ImGui::SliderFloat("Z##Translation", &translation.z, -10.0f, 10.0f);

	ImGui::Text("Scaling:");
	ImGui::SliderFloat("##Scaling", &scaling, 0.0f, 1.0f);

	ImGui::Text("Rotation:");
	ImGui::SliderFloat("##Rotation", &rotation, 0.0f, 360.0f);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

