#include "Shader.h"
#include <D3Dcompiler.h>
#include "d3dx12.h"

Shader::Shader()
{
	
}

bool Shader::Load(LPCWSTR filename, LPCSTR entryPoint, LPCSTR shaderType)
{
	hr = D3DCompileFromFile(filename,
		nullptr,
		nullptr,
		entryPoint,
		shaderType,
		D3DCOMPILE_OPTIMIZATION_LEVEL3,// D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shaderBlob,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	return true;
}

D3D12_SHADER_BYTECODE Shader::GetByteCode()
{
	return CD3DX12_SHADER_BYTECODE(shaderBlob);
}

