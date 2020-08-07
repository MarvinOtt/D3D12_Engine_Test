#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

using namespace std;

class GraphicsDevice;
#include "VertexBuffer.h"
#include "IndexBuffer.h"


class DXR_ACS_BOT
{
public:
    ID3D12Resource* ACS_buffer;
    vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDesc;

private:
    HRESULT hr;

public:
    DXR_ACS_BOT();
    template <typename T>
    bool AddGeometry(VertexBuffer<T>* vertexBuffer);
    template <typename T, typename U>
    bool AddGeometryIndices(VertexBuffer<T>* vertexBuffer, IndexBuffer<U>* indexBuffer);
    bool Build(GraphicsDevice*);
};

template <typename T>
bool DXR_ACS_BOT::AddGeometry(VertexBuffer<T>* vertexBuffer)
{
    D3D12_RAYTRACING_GEOMETRY_DESC newGeomDesc = {};
    newGeomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    newGeomDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->buffer->GetGPUVirtualAddress();
    newGeomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(T);
    newGeomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    newGeomDesc.Triangles.VertexCount = vertexBuffer->numVertices;
    newGeomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    geomDesc.push_back(newGeomDesc);

    return true;
}

template <typename T, typename U>
bool DXR_ACS_BOT::AddGeometryIndices(VertexBuffer<T>* vertexBuffer, IndexBuffer<U>* indexBuffer)
{
    D3D12_RAYTRACING_GEOMETRY_DESC newGeomDesc = {};
    newGeomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    newGeomDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->buffer->GetGPUVirtualAddress();
    newGeomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(T);
    newGeomDesc.Triangles.IndexBuffer = indexBuffer->buffer->GetGPUVirtualAddress(); 
    newGeomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    newGeomDesc.Triangles.IndexCount = indexBuffer->numIndices;
    newGeomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    newGeomDesc.Triangles.VertexCount = vertexBuffer->numVertices;
    newGeomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    geomDesc.push_back(newGeomDesc);

    return true;
} 

