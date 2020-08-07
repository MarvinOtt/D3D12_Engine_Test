#include "GraphicsDevice.h"
#include "PipelineState.h"
#include "DXR_RootSignature.h"
#include "Shader.h"
#include "SwapChain.h"



PipelineState::PipelineState()
{

}

bool PipelineState::SetShaders(Shader* vs, Shader* ps)
{
	VS = vs;
	PS = ps;
	return true;
}

bool PipelineState::AddRTV(DXGI_FORMAT format)
{
	rtv_formats.push_back(format);
	return true;
}

bool PipelineState::Create(GraphicsDevice* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
{
    // create the pso
    hr = device->device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
    if (FAILED(hr))
        return false;

    return true;
}

bool PipelineState::Create(GraphicsDevice* device, D3D12_INPUT_ELEMENT_DESC* IE_desc, int IE_desc_size, DXR_RootSignature* RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE top_type)
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = IE_desc_size / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = IE_desc;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.pRootSignature = RS->RS;
	psoDesc.PS = PS->GetByteCode();
	psoDesc.VS = VS->GetByteCode();
	psoDesc.PrimitiveTopologyType = top_type;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	for (int i = 0; i < rtv_formats.size(); ++i)
	{
		psoDesc.RTVFormats[i] = rtv_formats[i];
	}
	psoDesc.NumRenderTargets = rtv_formats.size();
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.SampleDesc = device->swapChain->sampleDesc;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	hr = device->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
	if (FAILED(hr)) 
		return false;

	return true;
}

