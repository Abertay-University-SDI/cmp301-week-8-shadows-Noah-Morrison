#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initialise key colours
	keyColours = {
	{0.0f, {0.0f, 0.0f, 0.5f, 1.0f}},   // Midnight (Navy Blue)
	{6.0f, {0.0f, 0.0f, 0.5f, 1.0f}},   // Early morning (Navy Blue)
	{6.5f, {1.0f, 0.447f, 0.435f, 1.0f}}, // Sunrise (Sunset Pink)
	{7.0f, {0.165f, 0.322f, 0.745f, 1.0f}}, // Daytime (Cerulean)
	{17.0f, {0.165f, 0.322f, 0.745f, 1.0f}}, // Daytime (Cerulean)
	{17.5f, {1.0f, 0.388f, 0.278f, 1.0f}}, // Sunset (Sunset Orange)
	{18.0f, {0.0f, 0.0f, 0.5f, 1.0f}}    // Midnight again
	};

	// Create Mesh object and shader object
	planeMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/wheat.obj");
	textureMgr->loadTexture(L"wheat", L"res/wheat.png");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"checkerboard", L"res/checkerboard.png");

	// Create addition geometry objects
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	triangleMesh = new TriangleMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Instance (Wheat) Shader
	wheatShader = new WheatShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 8192;
	int shadowmapHeight = 8192;
	int sceneWidth = 200;
	int sceneHeight = 200;

	shadowMaps[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMaps[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMaps[2] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Ortho mesh to view light POV
	//orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 512, 512, -screenWidth / 1.3, screenHeight / 1.8);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(),  200, 200, -screenWidth / 2.7, screenHeight / 2.7);

	// Light render texture
	lightTexture = new RenderTexture(renderer->getDevice(), shadowmapWidth, shadowmapHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Configure directional light

	lights[0] = new Light();
	lights[0]->setDiffuseColour(0.3f, 0.0f, 0.0f, 1.0f);
	lights[0]->setDirection(1.0f, -1.0f, 0.0f);
	lights[0]->setPosition(position.x, position.y, position.z);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[0]->generateProjectionMatrix(0.1f, 100.f);

	lights[1] = new Light();
	lights[1]->setDiffuseColour(0.0f, 0.3f, 0.0f, 1.0f);
	lights[1]->setDirection(-1.0f, -1.0f, 0.0f);
	lights[1]->setPosition(30.0f, 30.0f, 0.0f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[1]->generateProjectionMatrix(0.1f, 100.f);

	lights[2] = new Light();
	lights[2]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	lights[2]->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	lights[2]->setPosition(-lightDirection.x * 10.f, -lightDirection.y * 10.f, -lightDirection.z * 10.f);
	lights[2]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[2]->generateProjectionMatrix(0.1f, 100.f);
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

	// Get delta time
	deltaTime = timer->getTime();
	totalTime += deltaTime;

	// Animate sphere by offsetting it back and forth on the X-axis
	if ((cubeOffset + deltaTime) >= 20.f || (cubeOffset + deltaTime <= -20.f))
	{
		velocity = -velocity;
	};

	cubeOffset += deltaTime * velocity;

	lights[0]->setPosition(position.x, position.y, position.z);
	
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
	lights[2]->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	lights[2]->setPosition(-lightDirection.x * 30.f, -lightDirection.y * 30.f, -lightDirection.z * 30.f);

	// Perform depth passes
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		depthPass(i);
	}
	//depthPass2();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass(int index)
{
	// Set the render target to be the render to texture.
	shadowMaps[index]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	lights[index]->generateViewMatrix();
	XMMATRIX lightViewMatrix = lights[index]->getViewMatrix();
	lightProjectionMatrix = lights[index]->getOrthoMatrix();
	if (matrixToggle) {
		lightProjectionMatrix = lights[index]->getProjectionMatrix();
	}
	//XMMATRIX lightProjectionMatrix = lights[index]->getOrthoMatrix();
	// TODO - implement projection matrix to create frustum
	//XMMATRIX lightProjectionMatrix = light->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	planeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
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

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::depthPass2()
{
	// Set the render target to be the render to texture.
	shadowMaps[2]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	lights[2]->generateViewMatrix();
	XMMATRIX lightViewMatrix = lights[2]->getViewMatrix();
	lightProjectionMatrix = lights[2]->getOrthoMatrix();
	if (matrixToggle) {
		lights[2]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
		lightProjectionMatrix = lights[2]->getProjectionMatrix();
	}
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	planeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
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

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	XMFLOAT4 currentColor = calculateBackground(fractionalHours, keyColours);
	renderer->beginScene(currentColor.x, currentColor.y, currentColor.z, currentColor.w);
	ambient = currentColor;

	//renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// Set ambient colour (light grey)
	//ambient = { 0.3f, 0.3f, 0.3f, 1.0f };

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		depthMaps[i] = shadowMaps[i]->getDepthMapSRV();
	}

	//depthMaps[0] = shadowMap->getDepthMapSRV();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	planeMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), depthMaps, ambient, lights, matrixToggle);
	shadowShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), depthMaps, ambient, lights, matrixToggle);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(cubeOffset, 7.f, 10.f);
	cubeMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), depthMaps, ambient, lights, matrixToggle);
	shadowShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Render sphere (light position) // TODO - Add back after multiple lights and shadow maps implemented
	//worldMatrix = renderer->getWorldMatrix();
	//worldMatrix = XMMatrixTranslation(-lightDirection.x * 30.f , -lightDirection.y * 30.f, -lightDirection.z * 30.f);
	//sphereMesh->sendData(renderer->getDeviceContext());
	//shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
	//	textureMgr->getTexture(L"checkerboard"), depthMaps, ambient, lights, matrixToggle);
	//shadowShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());

	// Render spheres at light positions
	XMFLOAT3 lightPosition;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		worldMatrix = renderer->getWorldMatrix();
		lightPosition = lights[i]->getPosition();
		worldMatrix = XMMatrixTranslation(lightPosition.x, lightPosition.y, lightPosition.z);
		sphereMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"checkerboard"));
		textureShader->render(renderer->getDeviceContext(), sphereMesh->getIndexCount());
	}


	XMFLOAT3 wheatPositions[NUM_WHEAT_CLUMPS] = {
		XMFLOAT3(10.0f, 0.0f, 5.0f),
		XMFLOAT3(15.0f, 0.0f, 10.0f),
		XMFLOAT3(20.0f, 0.0f, 15.0f)
	};

	XMFLOAT3 wheatScales[NUM_WHEAT_CLUMPS] = {
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, 1.0f)
	};

	XMFLOAT4 wheatRotations[NUM_WHEAT_CLUMPS] = {
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		XMFLOAT4(0.707f, 0.0f, 0.707f, 0.0f),
		XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f)
	};

	// Render light POV
	renderer->setZBuffer(false);

	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, depthMaps[0]);
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

	// Matrix toggle for light depth maps
	ImGui::Checkbox("Projection Matrix", &matrixToggle);

	// Debug Light Controls
	ImGui::Text("Red Light Position:");
	ImGui::SliderFloat("X##Position", &position.x, -100.0f, 100.0f);
	ImGui::SliderFloat("Y##Position", &position.y, -100.0f, 100.0f);
	ImGui::SliderFloat("Z##Position", &position.z, -100.0f, 100.0f);

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
		ImGui::SliderInt("Hours##Direction", &hours, 0, 24);
		ImGui::SliderInt("Minutes##Direction", &minutes, 0, 59);

		fractionalHours = static_cast<float>(hours) + static_cast<float>(minutes) / 60.0f;

		if (minutes == 59) {
			minutes = 0;
			delay += 1;
		}

		// Since target FPS is 60 frames delay should last for 500 milliseconds
		// TODO - NOT RELIABLE - Should be using dt
		if (delay == 30) {
			delay = 0;
			hours += 1;
		}

		if (hours == 24) {
			hours = 0;
		}
		
		lightDirection.x = sin(((static_cast<float>(hours) / (24.f / 360.f)) + (static_cast<float>(minutes) / ((24.f * 60.f) / 360.f))) * 0.0174532f);
		lightDirection.y = cos(((static_cast<float>(hours) / (24.f / 360.f)) + (static_cast<float>(minutes) / ((24.f * 60.f) / 360.f))) * 0.0174532f);
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

XMFLOAT4 App1::lerpColour(const XMFLOAT4& colourA, const XMFLOAT4& colourB, float t)
{
	return {
		colourA.x + t * (colourB.x - colourA.x),
		colourA.y + t * (colourB.y - colourA.y),
		colourA.z + t * (colourB.z - colourA.z),
		colourA.w + t * (colourB.w - colourA.w)
	};
}

XMFLOAT4 App1::calculateBackground(float hours, std::vector<App1::TimeColour> keyColours)
{
	for (size_t i = 0; i < keyColours.size() - 1; ++i) {
		const App1::TimeColour& start = keyColours[i];
		const App1::TimeColour& end = keyColours[i + 1];

		if (start.time <= hours && hours <= end.time) {
			float t = (hours - start.time) / (end.time - start.time); // Normalize time
			return lerpColour(start.colour, end.colour, t);
		}
	}
	return keyColours.back().colour; // Default to the last color if out of range
}