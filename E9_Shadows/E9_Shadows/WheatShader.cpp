#include "WheatShader.h"

WheatShader::WheatShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"wheat_vs.cso", L"wheat_ps.cso");
}

WheatShader::~WheatShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the instance buffer
	if (instanceBuffer)
	{
		instanceBuffer->Release();
		instanceBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void WheatShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC instanceBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);


	// Setup the description of the dynamic instance constant buffer that is in the vertex shader.
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(InstanceData) * NUM_WHEAT_CLUMPS;
	instanceBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	instanceBufferDesc.StructureByteStride = sizeof(InstanceData);

	// 'initData' stands for 'initial data' -> data can't be initialized when there isn't instance data yet
	//D3D11_SUBRESOURCE_DATA initData = {}; - TODO
	// 'pStsMem' is a const void* pointer to the initialization data
	// .data() documentation: https://cplusplus.com/reference/vector/vector/data/
	// returns direct pointer to the memory array used to store the vector
	//initData.pSysMem = instances.data(); - TODO

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	//renderer->CreateBuffer(&instanceBufferDesc, &initData, &instanceBuffer); - TODO
	//renderer->CreateBuffer(&instanceBufferDesc, nullptr , &instanceBuffer);
	renderer->CreateBuffer(&instanceBufferDesc, NULL , &instanceBuffer);
	// Is an issue being caused here because 'nullptr' is being used instead of 'NULL'
	// nullptr documentation: https://learn.microsoft.com/en-us/cpp/extensions/nullptr-cpp-component-extensions?view=msvc-170
	// keyword that represents a null pointer value
	// NULL documentation: https://en.cppreference.com/w/cpp/types/NULL
	// macro that essentially represents a 0

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//srvDesc.Buffer.ElementWidth = instances.size(); - TODO
	// ElementWidth is the 'width of each element (in bytes)' - Type: 'UNIT' - An unsigned INT. The range is 0 through 4294967295 decimal.
	// .size() documentation: https://cplusplus.com/reference/vector/vector/size/
	// returns the number of elements in the vector
	srvDesc.Buffer.ElementWidth = sizeof(InstanceData); // Based on documentation - I think this is correct

	// because 'instanceBuffer' is dynamically updating, -
	// this function needs to be seen in the render loop - 
	// so 'instanceBufferSRV' dynamically updates as well.
	//renderer->CreateShaderResourceView(instanceBuffer, &srvDesc, &instanceBufferSRV);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}


void WheatShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
									ID3D11ShaderResourceView* texture,
									XMFLOAT3 positions[NUM_WHEAT_CLUMPS], XMFLOAT3 scales[NUM_WHEAT_CLUMPS], XMFLOAT4 rotations[NUM_WHEAT_CLUMPS])
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


	// Send instance data
	for (size_t i = 0; i < NUM_WHEAT_CLUMPS; ++i) {
		InstanceData instance;
		instance.position = positions[i];
		instance.scale = scales[i];
		instance.rotation = rotations[i];

		instance.padding0 = 0.0f;
		instance.padding1 = 0.0f;
		
		instances.push_back(instance);
	}

	deviceContext->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, instances.data(), sizeof(InstanceData) * NUM_WHEAT_CLUMPS);
	deviceContext->Unmap(instanceBuffer, 0);

	// 'instanceBufferSRV' now dynamically updates
	renderer->CreateShaderResourceView(instanceBuffer, &srvDesc, &instanceBufferSRV);

	deviceContext->VSSetShaderResources(0, 1, &instanceBufferSRV);


	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

// custom render() function since the one from 'BaseShader.cpp' doesn't use the correct draw call to support instance rendering
void WheatShader::render(ID3D11DeviceContext* deviceContext, int indexCount, int instanceCount) 
{
	deviceContext->IASetInputLayout(layout);

	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	deviceContext->CSSetShader(NULL, NULL, 0);

	deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}

// render() in 'BaseShader.cpp' for referene
//// De/Activate shader stages and send shaders to GPU.
//void BaseShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
//{
//	// Set the vertex input layout.
//	deviceContext->IASetInputLayout(layout);
//
//	// Set the vertex and pixel shaders that will be used to render.
//	deviceContext->VSSetShader(vertexShader, NULL, 0);
//	deviceContext->PSSetShader(pixelShader, NULL, 0);
//	deviceContext->CSSetShader(NULL, NULL, 0);
//
//	// if Hull shader is not null then set HS and DS
//	if (hullShader)
//	{
//		deviceContext->HSSetShader(hullShader, NULL, 0);
//		deviceContext->DSSetShader(domainShader, NULL, 0);
//	}
//	else
//	{
//		deviceContext->HSSetShader(NULL, NULL, 0);
//		deviceContext->DSSetShader(NULL, NULL, 0);
//	}
//
//	// if geometry shader is not null then set GS
//	if (geometryShader)
//	{
//		deviceContext->GSSetShader(geometryShader, NULL, 0);
//	}
//	else
//	{
//		deviceContext->GSSetShader(NULL, NULL, 0);
//	}
//
//	// Render the triangle.
//	deviceContext->DrawIndexed(indexCount, 0, 0);
//}
