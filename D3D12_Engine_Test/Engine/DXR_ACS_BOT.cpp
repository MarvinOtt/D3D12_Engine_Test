#include "DXR_ACS_BOT.h"
#include "GraphicsDevice.h"
#include "VertexBuffer.h"
#include "Buffer.h"

struct AccelerationStructureBuffers
{
    Buffer* pScratch;
    Buffer* pResult;
    Buffer* pInstanceDesc;    // Used only for top-level AS
};

DXR_ACS_BOT::DXR_ACS_BOT()
{

}

bool DXR_ACS_BOT::Build(GraphicsDevice* device)
{
    // Get the size requirements for the scratch and AS buffers
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    inputs.NumDescs = geomDesc.size();
    inputs.pGeometryDescs = geomDesc.data();
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
    device->device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    // Create the buffers. They need to support UAV, and since we are going to immediately use them, we create them with an unordered-access state
    AccelerationStructureBuffers ACC_BUF_BOT;
    ACC_BUF_BOT.pScratch = new Buffer(device, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT);
    ACC_BUF_BOT.pResult = new Buffer(device, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_HEAP_TYPE_DEFAULT);

    // Create the bottom-level AS
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
    asDesc.Inputs = inputs;
    asDesc.DestAccelerationStructureData = ACC_BUF_BOT.pResult->buffer->GetGPUVirtualAddress();
    asDesc.ScratchAccelerationStructureData = ACC_BUF_BOT.pScratch->buffer->GetGPUVirtualAddress();

    device->commandList->commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

    // We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
    D3D12_RESOURCE_BARRIER uavBarrier = {};
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = ACC_BUF_BOT.pResult->buffer;
    device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

    ACS_buffer = ACC_BUF_BOT.pResult->buffer;

    return true;
}

