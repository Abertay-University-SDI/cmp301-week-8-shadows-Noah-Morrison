#pragma once

#include "BaseShader.h"

#define NUM_WHEAT_CLUMPS 9

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
		InstanceData instanceData[NUM_WHEAT_CLUMPS];
	};

public:
	WheatShader(ID3D11Device* device, HWND hwnd);
	~WheatShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, InstanceData* instanceData[NUM_WHEAT_CLUMPS]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
};



