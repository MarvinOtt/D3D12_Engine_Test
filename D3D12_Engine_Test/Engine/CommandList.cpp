#include "CommandList.h"
#include "GraphicsDevice.h"
#include "CommandAllocator.h"
#include "PipelineState.h"
#include "CommandQueue.h"


CommandList::CommandList()
{

}

bool CommandList::Create(GraphicsDevice* device, CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	hr = device->device->CreateCommandList(0, type, commandAllocator->commandAllocator, NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
		return false;

	return true;
}

bool CommandList::Reset(CommandAllocator* commandAllocator, PipelineState* pso)
{
	hr = commandList->Reset(commandAllocator->commandAllocator, pso->pipelineState);
	if (FAILED(hr))
		return false;

	return true;
}

bool CommandList::CloseAndExecute(CommandQueue* queue)
{
    hr = commandList->Close();
    if (FAILED(hr))
        return false;
    ID3D12CommandList* ppCommandLists[] = { commandList };
    queue->commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    return true;
}

