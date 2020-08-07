#include "DXR_ACS_TOP.h"
#include "GraphicsDevice.h"
#include "Buffer.h"
#include "CommandQueue.h"
#include "Fence.h"
#include "SwapChain.h"
#include "CommandAllocator.h"

DXR_ACS_TOP::DXR_ACS_TOP(int numberOfRays2)
{
    numberOfRays = numberOfRays2;
    ACS_buffer = new AccelerationStructureBuffers();
    HasBeenBuilt = false;
}

bool DXR_ACS_TOP::Add_ACS_BOT(DXR_ACS_BOT* acs, mat4 transformation, LPCWSTR* hitGroups2)
{
    trans.push_back(transformation);
    ACS_buffers.push_back(acs);
    hitGroups.push_back(hitGroups2);

    return true;
}

bool DXR_ACS_TOP::Build(GraphicsDevice* device)
{
    if (HasBeenBuilt)
    {
        ACS_buffer->pScratch->buffer->Release();
        ACS_buffer->pResult->buffer->Release();
        ACS_buffer->pInstanceDesc->buffer->Release();
    }

    // First, get the size of the TLAS buffers and create them
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    inputs.NumDescs = trans.size();
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

    device->device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    // Create the buffers
    ACS_buffer->pScratch = new Buffer(device, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT);
    ACS_buffer->pResult = new Buffer(device, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_HEAP_TYPE_DEFAULT);

    // The instance desc should be inside a buffer, create and map the buffer
    ACS_buffer->pInstanceDesc = new Buffer(device, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * inputs.NumDescs, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
    ACS_buffer->pInstanceDesc->buffer->Map(0, nullptr, (void**)&pInstanceDesc);
    ZeroMemory(pInstanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * inputs.NumDescs);
    int length = trans.size();
    int curindex = 0;
    for (int i = 0; i < length; ++i)
    {
        pInstanceDesc[i].InstanceID = i; // This value will be exposed to the shader via InstanceID()
        pInstanceDesc[i].InstanceContributionToHitGroupIndex = curindex; // This is the offset inside the shader-table. We only have a single geometry, so the offset 0
        pInstanceDesc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        mat4 m = transpose(trans[i]); // GLM is column major, the INSTANCE_DESC is row major
        memcpy(pInstanceDesc[i].Transform, &m, sizeof(pInstanceDesc[i].Transform));
        pInstanceDesc[i].AccelerationStructure = ACS_buffers[i]->ACS_buffer->GetGPUVirtualAddress();
        pInstanceDesc[i].InstanceMask = 0xFF;
        curindex += numberOfRays * ACS_buffers[i]->geomDesc.size();
    }

    // Unmap
    ACS_buffer->pInstanceDesc->buffer->Unmap(0, nullptr);

    // Create the TLAS
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
    asDesc.Inputs = inputs;
    asDesc.Inputs.InstanceDescs = ACS_buffer->pInstanceDesc->buffer->GetGPUVirtualAddress();
    asDesc.DestAccelerationStructureData = ACS_buffer->pResult->buffer->GetGPUVirtualAddress();
    asDesc.ScratchAccelerationStructureData = ACS_buffer->pScratch->buffer->GetGPUVirtualAddress();

    device->commandList->commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

    // We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
    D3D12_RESOURCE_BARRIER uavBarrier = {};
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = ACS_buffer->pResult->buffer;
    device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

    HasBeenBuilt = true;

    return true;
}

bool DXR_ACS_TOP::Update(GraphicsDevice* device)
{
    if (!HasBeenBuilt)
        return false;

    // First, get the size of the TLAS buffers and create them
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    inputs.NumDescs = trans.size();
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

    device->device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    // If this a request for an update, then the TLAS was already used in a DispatchRay() call. We need a UAV barrier to make sure the read operation ends before updating the buffer
    D3D12_RESOURCE_BARRIER uavBarrier = {};
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = ACS_buffer->pResult->buffer;
    device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

    D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
    ACS_buffer->pInstanceDesc->buffer->Map(0, nullptr, (void**)&pInstanceDesc);
    ZeroMemory(pInstanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * inputs.NumDescs);
    int length = trans.size();
    for (int i = 0; i < length; ++i)
    {
        pInstanceDesc[i].InstanceID = i; // This value will be exposed to the shader via InstanceID()
        pInstanceDesc[i].InstanceContributionToHitGroupIndex = i * numberOfRays; // This is the offset inside the shader-table. We only have a single geometry, so the offset 0
        pInstanceDesc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        mat4 m = transpose(trans[i]); // GLM is column major, the INSTANCE_DESC is row major
        memcpy(pInstanceDesc[i].Transform, &m, sizeof(pInstanceDesc[i].Transform));
        pInstanceDesc[i].AccelerationStructure = ACS_buffers[i]->ACS_buffer->GetGPUVirtualAddress();
        pInstanceDesc[i].InstanceMask = 0xFF;
    }

    // Unmap
    ACS_buffer->pInstanceDesc->buffer->Unmap(0, nullptr);

    // Create the TLAS
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
    asDesc.Inputs = inputs;
    asDesc.Inputs.InstanceDescs = ACS_buffer->pInstanceDesc->buffer->GetGPUVirtualAddress();
    asDesc.DestAccelerationStructureData = ACS_buffer->pResult->buffer->GetGPUVirtualAddress();
    asDesc.ScratchAccelerationStructureData = ACS_buffer->pScratch->buffer->GetGPUVirtualAddress();
    asDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    asDesc.SourceAccelerationStructureData = ACS_buffer->pResult->buffer->GetGPUVirtualAddress();

    device->commandList->commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

    // We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
    uavBarrier = {};
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = ACS_buffer->pResult->buffer;
    device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

    return true;
}

