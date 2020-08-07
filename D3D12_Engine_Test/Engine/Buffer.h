#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "GraphicsDevice.h"
#include "CommandList.h"


class Buffer
{
public:
    ID3D12Resource* buffer;
    UINT64 bufferSize;

private:
    HRESULT hr;

public:
    Buffer(GraphicsDevice* device, UINT64 buffersize, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heap_type);

};


