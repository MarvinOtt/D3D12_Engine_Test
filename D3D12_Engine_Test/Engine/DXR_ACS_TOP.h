#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include "glm/glm.hpp"
#include "DXR_ACS_BOT.h"

using namespace std; 
using namespace glm;

class GraphicsDevice;
class Buffer;

struct AccelerationStructureBuffers
{
    Buffer* pScratch;
    Buffer* pResult;
    Buffer* pInstanceDesc;    // Used only for top-level AS
};

class DXR_ACS_TOP
{
public:
    AccelerationStructureBuffers* ACS_buffer;
    vector<mat4> trans;
    vector<DXR_ACS_BOT*> ACS_buffers;
    vector<LPCWSTR*> hitGroups;
    bool HasBeenBuilt;
    int numberOfRays;

private:
    HRESULT hr;

public:
    DXR_ACS_TOP(int numberOfRays2);
    bool Add_ACS_BOT(DXR_ACS_BOT* acs, mat4 transformation, LPCWSTR* hitGroups);
    bool Build(GraphicsDevice*);
    bool Update(GraphicsDevice*);
};

