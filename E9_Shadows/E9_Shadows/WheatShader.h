#pragma once

#include "BaseShader.h"
#include <vector>

#define NUM_WHEAT_CLUMPS 1080

using namespace std;
using namespace DirectX;

class WheatShader : public BaseShader
{
private:
	struct InstanceData {
		XMFLOAT3 position;
		float padding0;
		XMFLOAT3 scale;
		float padding1;
		XMFLOAT4 rotation;
	};

	struct InstanceBufferType {
		InstanceData instances[NUM_WHEAT_CLUMPS];
	};

	struct TimeBufferType {
		float time;
		XMFLOAT3 padding0;
	};

public:
	WheatShader(ID3D11Device* device, HWND hwnd);
	~WheatShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
							ID3D11ShaderResourceView* texture, 
							XMFLOAT3 positions[NUM_WHEAT_CLUMPS], XMFLOAT3 scales[NUM_WHEAT_CLUMPS], XMFLOAT4 rotations[NUM_WHEAT_CLUMPS],
							float time);

	void render(ID3D11DeviceContext* deviceContext, int indexCount, int instanceCount);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* instanceBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11ShaderResourceView* instanceBufferSRV;
	ID3D11SamplerState* sampleState;

	vector<InstanceData> instances;

	D3D11_SUBRESOURCE_DATA initData;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
};



