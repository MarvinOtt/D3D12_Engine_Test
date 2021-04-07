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
	template <typename T>
	DXR_ACS_BOT(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer);
	template <typename T, typename U>
	DXR_ACS_BOT(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer, IndexBuffer<U>* indexBuffer);
	bool Build(GraphicsDevice* graphicsDevice);
};

template <typename T>
DXR_ACS_BOT::DXR_ACS_BOT(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer)
{
	D3D12_RAYTRACING_GEOMETRY_DESC newGeomDesc = {};
	newGeomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	newGeomDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->buffer->GetGPUVirtualAddress();
	newGeomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(T);
	newGeomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	newGeomDesc.Triangles.VertexCount = vertexBuffer->numVertices;
	newGeomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	geomDesc.push_back(newGeomDesc);

	Build(graphicsDevice);
}

template <typename T, typename U>
DXR_ACS_BOT::DXR_ACS_BOT(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer, IndexBuffer<U>* indexBuffer)
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

	Build(graphicsDevice);
}

