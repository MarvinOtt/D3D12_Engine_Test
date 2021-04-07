#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "Framework.h"

class CommandAllocator;
class CommandQueue;
class CommandList;
class Fence;
class SwapChain;
class DescriptorHeap;
class RenderTarget;

class GraphicsDevice
{
public:
	IDXGIFactory4* dxgiFactory;
	ID3D12Device5* device;
	HWND hwnd = NULL;

	CommandAllocator* commandAllocator;
	CommandQueue* commandQueue;
	CommandList* commandList;
	Fence* fences;
	SwapChain* swapChain;
	DescriptorHeap* rtvHeap;
	DescriptorHeap* dsHeap;
	RenderTarget* backBuffer;



	int bufferWidth, bufferHeight;
	int frameBufferCount = 3;
	int frameIndex;
private:
	HRESULT hr;

public:
	GraphicsDevice(HWND, int, int);
	bool WaitForEventCompletion(int index);
	bool Create();
};



