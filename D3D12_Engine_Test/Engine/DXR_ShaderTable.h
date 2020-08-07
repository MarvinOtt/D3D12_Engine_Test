#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

using namespace std;

class GraphicsDevice;
class Buffer;
class DXR_PipelineState;
class DXR_ACS_TOP;

class DXR_ShaderTable
{
public:
    Buffer* shaderTable, *tempShaderTable;
    LPCWSTR* missShaders;
    UINT64** missShaders_data;
    int* missShaders_data_count;

    LPCWSTR rayGenName;
    UINT64* rayGenData;
    int rayGenData_count;

    UINT64** geomData;
    int shaderTableEntrySize;

private:
    HRESULT hr;
    DXR_ACS_TOP* acs_top;
    vector<D3D12_ROOT_PARAMETER> rootParams;
    int missIndex, hitIndex, entryNumber;
    GraphicsDevice* device;

public:
    DXR_ShaderTable(DXR_ACS_TOP* acs_top);
    bool SetRayGenShader(LPCWSTR name, UINT64* data, int datasize);
    bool SetMissShader(int index, LPCWSTR name, UINT64* data, int datasize);
    bool Create(GraphicsDevice* device, DXR_PipelineState* pipelineState);
    bool SetGeomData(int geomindex, int rayindex, UINT64* data);
    D3D12_DISPATCH_RAYS_DESC CreateRayDesc();

private:
    void UploadingFinished();
};

