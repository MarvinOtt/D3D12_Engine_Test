#include "DescriptorHeap.h"
#include "GraphicsDevice.h"
#include "Texture2D.h"

DescriptorHeap::DescriptorHeap()
{
	descriptorType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
}

DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	descriptorType = type;
    descriptorFlags = flags;
}

bool DescriptorHeap::Create(GraphicsDevice* device, int num)
{
	descriptorNum = num;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = descriptorNum; // number of descriptors for this heap.
	rtvHeapDesc.Type = descriptorType; // Type

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = descriptorFlags;    
	hr = device->device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	if (FAILED(hr))
		return false;

	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
	descriptorSize = device->device->GetDescriptorHandleIncrementSize(descriptorType);
}

bool DescriptorHeap::SetTexture2D(GraphicsDevice* device, Texture2D* tex, int index)
{
    if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        return false;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE Handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    Handle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
    device->device->CreateUnorderedAccessView(tex->buffer, nullptr, &uavDesc, Handle);
    return true;
}

bool DescriptorHeap::SetSRV(GraphicsDevice* device, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, int index)
{
    if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        return false;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    srvHandle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
    device->device->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
    return true;
}