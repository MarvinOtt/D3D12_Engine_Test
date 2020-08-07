#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;
class DescriptorHeap;

class Texture2D
{
public:
    ID3D12Resource* buffer;
    D3D12_RESOURCE_DESC desc;
    UINT64 bufferSize;

private:
    HRESULT hr;
    ID3D12Resource* uploadheap;

public:
    Texture2D();
    bool Create(GraphicsDevice*, D3D12_RESOURCE_DESC);
    bool Create(GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height);
    bool Create(GraphicsDevice* device, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, int Width, int Height);

    bool SetData(GraphicsDevice* device, BYTE* texdata, int imagesize, DescriptorHeap* descHeap, int heapindex);
};

