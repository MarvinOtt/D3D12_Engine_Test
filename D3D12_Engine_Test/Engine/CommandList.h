#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "..\Events.h"

class GraphicsDevice;
class CommandAllocator;
class PipelineState;
class CommandQueue;

class CommandList
{
private:
    using VoidDelegate = std::function<void()>;

public:
	ID3D12GraphicsCommandList4* commandList;
    Event<VoidDelegate>* CommandListFinished = new Event<VoidDelegate>();

private:
	HRESULT hr;

public:
	CommandList();
	bool Create(GraphicsDevice*, CommandAllocator*, D3D12_COMMAND_LIST_TYPE type);
	bool Reset(CommandAllocator*, PipelineState*);
    bool CloseAndExecute(CommandQueue* queue);
};

