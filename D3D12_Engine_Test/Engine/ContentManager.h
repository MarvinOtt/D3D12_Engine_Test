#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class Texture2D;

class ContentManager
{
public:


private:
	HRESULT hr;
	ID3DBlob* errorBuff;

public:
	ContentManager();
	Texture2D* LoadTexture2D(LPCWSTR location);
};

