#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
#endif

#include <dinput.h>
#include <algorithm>
#include "D3D12_Engine.h"
#include "resource.h"
#include <windows.h>
#include <wincodec.h>
#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <math.h>
#include "Engine/Framework.h"
#include "happly.h"
#include <sstream>
#include <iomanip>
#include <sstream>
#include <fstream>

#define PI 3.14159265359

//#include "Engine/glm/glm.hpp"

using namespace glm;

using namespace happly;


using namespace DirectX; // we will be using the directxmath library

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

float degree2rad(float angle_in_degrees) {
	return (float)(angle_in_degrees * (PI / 180.0f));
}

struct Vertex {
	Vertex(float x, float y, float z, float r, float g, float b) : pos(x, y, z), color(r, g, b) {}
	Vertex() : pos(0, 0, 0), color(0, 0, 0) {}
	XMFLOAT3 pos;
	XMFLOAT3 color;
};

struct Vertex_Pos {
	Vertex_Pos(float x, float y, float z) : pos(x, y, z) {}
	Vertex_Pos() : pos(0, 0, 0) {}
	XMFLOAT3 pos;
};

struct CameraMatrix {
    XMFLOAT4 offset;
    XMFLOAT4X4 rot;
    uint32_t counter;
};

struct CS_camera {
    XMFLOAT4 offset;
    XMFLOAT4X4 rot;
    XMFLOAT4 oldoffset;
    XMFLOAT4X4 oldrot_inv;
    XMFLOAT4X4 oldrot;
};

struct ObjectMatrix {
    mat4 mat;
};

// Handle to the window
//HWND hwnd = NULL;

// name of the window (not the title)
LPCTSTR WindowName = L"BzTutsApp";

// title of the window
LPCTSTR WindowTitle = L"Bz Window";

// width and height of the window
int Width = 2560 / 1;
int Height = 1440 / 1;

// is window full screen?
bool FullScreen = false;

Window* mainWindow;

// create a window
bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    bool fullscreen);

// main application loop
void mainloop();
bool InitDirectInput();
void DetectInput(double time);

// callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

// Input Stuff
char* keyboardstate;
char* oldkeyboardstate;

float cameraspeed;
XMFLOAT3 camrot;
POINT cameramousepos, mousepointpos;
POINT mousepos, oldmousepos;
XMFLOAT2 mouserotationbuffer;
XMVECTOR camerapos_vec, oldcamerapos_vec;
bool camerabewegen;


D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.

D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

bool IsRunning = true;

GraphicsDevice* graphicsDevice;
RootSignature* rootSignature;
PipelineState* pipelineState;
DescriptorHeap* mainHeap;

VertexBuffer<Vertex>* vertexBuffer;
VertexBuffer<Vertex>* vertexBuffer2;
VertexBuffer<XMFLOAT4>* emptyVB;
IndexBuffer<DWORD>* indexBuffer;

CameraMatrix cameraWVP;
CS_camera CScamera;
//ConstantBuffer<wvpMatrix>* constantBuffer;

Texture2D* cubetex;
uint64_t ACC_SIZE = 0;
DXR_PipelineState* DXR_COL_pipelineState, *DXR_AO_pipelineState;
DXR_ShaderTable* DXR_COL_shadertable, *DXR_AO_shadertable;
 
Texture2D* outputtex_RT, *outputAOtex_RT, *outputtex_CS, *posOutput, *oldposOutput, *AOtex_old, *AOtex_OUT, *normaltex, *AOtex_vertical, *AOtex_F, *TS_strength;

 

//ID3D12PipelineState* compute_PSO;
PipelineState* compute_PSO;
DXR_RootSignature* compute_RSD;
//DescriptorHeap* compute_DH;

// Bilateral Filter
DXR_RootSignature* BFV_RS, *BF_RS;
PipelineState* BFV_PS, *BF_PS;



IDxcBlob* m_library;

ID3D12RootSignature* m_rayGenSignature;
ID3D12RootSignature* m_missSignature;
ID3D12RootSignature* m_hitSignature;
ID3D12Resource* mpTopLevelAS;
//ID3D12Resource** mpBottomLevelAS;

ID3D12StateObject* stateobject;

Buffer** mpConstantBuffer;

DescriptorHeap* mpSrvUavHeap, *mpSrvUavHeap2;
ID3D12Resource* mpOutputResource;

ConstantBuffer<CameraMatrix>* CB_cameraWVP;
Buffer* CB_cameraWVP2, *CB_CScamera;
XMMATRIX oldrot;

Buffer** CB_objectmatrices;

DXR_ACS_BOT* ACS_BOT_cube;
DXR_ACS_OBJ* ACS_OBJ_plane;
vector<DXR_ACS_OBJ*> ACS_OBJ_cubes;
DXR_ACS_TOP* ACS_top;

float cuberotation = 0;
int counter = 0;



typedef struct CubeColor {
    DirectX::XMFLOAT4 colorMultiplier;
}CubeColor;

CubeColor cbColorMultiplierData; // this is the constant buffer data we will send to the gpu 


bool InitD3D(); // initializes direct3d 12

void Update(); // update the game logic

void UpdatePipeline(); // update the direct3d pipeline (update command lists)

void Render(); // execute the command list

void Cleanup(); // release com ojects and clean up memory


XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
XMMATRIX cameraViewMat, cameraRotMat; // this will store our view matrix

XMFLOAT4 camerapos; // this is our cameras position vector
XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
XMFLOAT4 cameraUp; // the worlds up vector

XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
XMFLOAT4 cube1Position; // our first cubes position in space

XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

int numCubeIndices; // the number of indices to draw the cube


int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);

DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);