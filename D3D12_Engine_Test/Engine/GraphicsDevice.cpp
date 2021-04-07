
#include "GraphicsDevice.h"
#include "RenderTarget.h"
#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Fence.h"
#include "DescriptorHeap.h"
#include "SwapChain.h"
#include "d3dx12.h"

using namespace Microsoft::WRL;

GraphicsDevice::GraphicsDevice(HWND hwnd2, int Width, int Height)
{
	hwnd = hwnd2;
	bufferWidth = Width;
	bufferHeight = Height;
	commandAllocator = new CommandAllocator[frameBufferCount];
	fences = new Fence[frameBufferCount];
}

bool GraphicsDevice::Create()
{
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		return false;

	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)
	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0
	bool adapterFound = false; // set this to true when a good one was found

	// find first hardware gpu that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		//ID3D12Device5* pDevice;
		ID3D12Device5* pDevice;
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
		if (SUCCEEDED(hr))
		{
			D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
			HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
			if (FAILED(hr) || features5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
			{
				//msgBox("Raytracing is not supported on this device. Make sure your GPU supports DXR (such as Nvidia's Volta or Turing RTX) and you're on the latest drivers. The DXR fallback layer is not supported.");
				exit(1);
			}
			adapterFound = true;
			break;
		}
		adapterIndex++;
	}
	if (!adapterFound)
		return false;

	// Create the device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device)
	);
	if (FAILED(hr))
		return false;


	// Command Queue
	commandQueue = new CommandQueue();
	if (!commandQueue->Create(this, D3D12_COMMAND_LIST_TYPE_DIRECT))
		return false;

	// Swap Chain
	swapChain = new SwapChain();
	if (!swapChain->Create(this, commandQueue))
		return false;

	frameIndex = swapChain->swapChain->GetCurrentBackBufferIndex();

	rtvHeap = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	rtvHeap->Create(this, frameBufferCount);

	dsHeap = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	dsHeap->Create(this, frameBufferCount);

	// Back Buffer
	backBuffer = new RenderTarget(rtvHeap);
	backBuffer->Create(this, bufferWidth, bufferHeight, dsHeap);

	// Command Allocators
	for (int i = 0; i < frameBufferCount; ++i)
	{
		commandAllocator[i] = CommandAllocator();
		commandAllocator[i].Create(this, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	// Command List
	commandList = new CommandList(this, &commandAllocator[frameIndex], D3D12_COMMAND_LIST_TYPE_DIRECT, hr);

	// Fences
	for (int i = 0; i < frameBufferCount; ++i)
	{
		fences[i] = Fence();
		fences[i].Create(this);
	}
	
	return true;
}

bool GraphicsDevice::WaitForEventCompletion(int index)
{
	hr = fences[index].fence->SetEventOnCompletion(fences[index].fenceValue, fences[index].fenceEvent);
	if (FAILED(hr))
		return false;
	WaitForSingleObject(fences[index].fenceEvent, INFINITE);

	for (auto listener : commandList->CommandListFinished->listeners())
	{
		listener();
	}

	return true;
}
