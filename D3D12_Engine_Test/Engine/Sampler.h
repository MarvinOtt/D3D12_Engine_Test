#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;

class Sampler
{
public:
	D3D12_STATIC_SAMPLER_DESC sampler;
	
private:
	HRESULT hr;

public:
	Sampler(D3D12_STATIC_SAMPLER_DESC);
};
