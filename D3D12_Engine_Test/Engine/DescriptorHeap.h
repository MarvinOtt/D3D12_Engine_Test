#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsDevice;
class Texture2D;

class DescriptorHeap
{
public:
	ID3D12DescriptorHeap* descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE descriptorType;
    D3D12_DESCRIPTOR_HEAP_FLAGS descriptorFlags;
	int descriptorSize;
	int descriptorNum;

private:
	HRESULT hr;
	GraphicsDevice* device;

public:
	DescriptorHeap();
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE, D3D12_DESCRIPTOR_HEAP_FLAGS);
	bool Create(GraphicsDevice*, int);
    bool SetTexture2D(Texture2D* tex, int index);
    bool SetSRV(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, int index);
};
