#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
class GraphicsDevice;

class RootSignature
{
public:
	ID3D12RootSignature* rootSignature;

private:
	HRESULT hr;

public:
	RootSignature();
	bool Create(GraphicsDevice*, D3D12_STATIC_SAMPLER_DESC sampler, D3D12_ROOT_PARAMETER* rootParameters, int rootParametersCount);
	bool Create(GraphicsDevice* device, D3D12_ROOT_SIGNATURE_DESC desc);
};