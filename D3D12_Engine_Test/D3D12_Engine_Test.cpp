#include "D3D12_Engine_Test.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    //create the window
	mainWindow = new Window(hInstance);
	mainWindow->Create(FullScreen, L"DXR_Test", Width, Height, 2560 * 0);
	mainWindow->Show(nShowCmd);

    // initialize direct3d
	InitD3D();
	InitDirectInput();

    // Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{
			Update();
			if (!IsRunning)
				break;
			Render();
			if (!IsRunning)
				break;
		}
	}

    // close the fence event
    CloseHandle(Fence::fenceEvent);

    return 0;
}

bool InitDirectInput() {
    keyboardstate = new char[256];
    oldkeyboardstate = new char[256];
    return true;
}

bool InitD3D()
{
    graphicsDevice = new GraphicsDevice(mainWindow->hwnd, Width, Height);
    graphicsDevice->Create();

	//Load models
	int housesize = 272;
	VertexBuffer<Vertex>** curvertexbuffer = new VertexBuffer<Vertex>*[housesize];
	IndexBuffer<int>** curindexbuffer = new IndexBuffer<int>*[housesize];

	string path = string("C:\\Users\\marvi\\Documents\\DXR4\\models\\Mesh");
	if (GetFileAttributes(L"C:\\Users\\marvi\\Documents\\DXR4\\models") == INVALID_FILE_ATTRIBUTES)
	{
		path = string("models\\Mesh");
	}

	for (int j = 0; j < housesize; ++j)
	{
		std::stringstream ss;
		ss << std::setw(3) << std::setfill('0') << (j + 0);

		PLYData* plyIn = new PLYData(path + ss.str() + ".ply");

		std::vector<double> vPosRaw = plyIn->getVertexPositions();
		std::vector<Vertex> vPos = std::vector<Vertex>(vPosRaw.size() / 3);

		for (size_t i = 0; i < vPos.size(); i++) {
			vPos[i] = Vertex((float)vPosRaw[i * 3], (float)vPosRaw[i * 3 + 1], (float)vPosRaw[i * 3 + 2], 1, 1, 1);
		}
		curvertexbuffer[j] = new VertexBuffer<Vertex>();
		curvertexbuffer[j]->Create(graphicsDevice, vPos.size() * sizeof(Vertex));
		curvertexbuffer[j]->SetData(graphicsDevice, vPos.data(), vPos.size(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		std::vector<int> vFaces = plyIn->getElement("face").getListProperty2<int>("vertex_indices");

		curindexbuffer[j] = new IndexBuffer<int>();
		curindexbuffer[j]->Create(graphicsDevice, vFaces.size() * sizeof(int));
		curindexbuffer[j]->SetData(graphicsDevice, vFaces.data(), vFaces.size(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

    // a cube
    Vertex vList[] = {
        // front face
        { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
        {  0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
        {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f },

        // right face
        {  0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },

        // left face
        { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
        { -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f },

        // back face
        {  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
        { -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
        {  0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
        { -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f },

        // top face
        { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f },
        {  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f },
        {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f },
        { -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f },

        // bottom face
        {  0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f },
        {  0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f },
    };

    vertexBuffer = new VertexBuffer<Vertex>();
    vertexBuffer->Create(graphicsDevice, sizeof(vList));
    vertexBuffer->SetData(graphicsDevice, vList, sizeof(vList) / sizeof(Vertex), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    DWORD iList[] = {
        // front face
        0, 1, 2, 0, 3, 1,
        // left face
        4, 5, 6, 4, 7, 5,
        // right face
        8, 9, 10, 8, 11, 9,
        // back face
        12, 13, 14, 12, 15, 13,
        // top face
        16, 17, 18, 16, 19, 17,
        // bottom face
        20, 21, 22, 20, 23, 21,
    };

    numCubeIndices = sizeof(iList) / sizeof(DWORD);
    indexBuffer = new IndexBuffer<DWORD>();
    indexBuffer->Create(graphicsDevice, sizeof(iList));
    indexBuffer->SetData(graphicsDevice, iList, sizeof(iList) / sizeof(DWORD), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    // a plane
    Vertex vList2[] = {
        { -100, -0.5f, -100, 0.0f, 0.0f, 0.0f },
        { -100, -0.5f,  100, 0.6f, 0.6f, 0.6f },
        {  100, -0.5f,  100, 0.6f, 0.6f, 0.6f },
        { -100, -0.5f, -100, 0.0f, 0.0f, 0.0f },
        {  100, -0.5f,  100, 0.6f, 0.6f, 0.6f },
        {  100, -0.5f, -100, 0.6f, 0.6f, 0.6f }
    };

    vertexBuffer2 = new VertexBuffer<Vertex>();
    vertexBuffer2->Create(graphicsDevice, sizeof(vList2));
    vertexBuffer2->SetData(graphicsDevice, vList2, sizeof(vList2) / sizeof(Vertex), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    // ACCELERATION STRUCTURES

	mat4 transformation[2];
	transformation[0] = translate(mat4(), vec3(-2, 100, 1));
	transformation[1] = translate(mat4(), vec3(2, 0.1f + 0.0f, 0)) * glm::eulerAngleX(0.3f);

    ACS_BOT_cube = new DXR_ACS_BOT(graphicsDevice, vertexBuffer, indexBuffer);

	ACS_OBJ_plane = new DXR_ACS_OBJ(graphicsDevice, vertexBuffer2, mat4(), true);
	for (int i = 0; i < 3; ++i)
	{
		if (i == 0)
			ACS_OBJ_cubes.push_back(new DXR_ACS_OBJ(graphicsDevice, ACS_BOT_cube, translate(mat4(), vec3(0, 9.0f, 0)), true));
		else
			ACS_OBJ_cubes.push_back(new DXR_ACS_OBJ(graphicsDevice, ACS_BOT_cube, transformation[i - 1], true));
	}
	for (int x = 0; x < 10; ++x)
	{
		for (int y = 0; y < 10; ++y)
		{
			ACS_OBJ_cubes.push_back(new DXR_ACS_OBJ(graphicsDevice, ACS_BOT_cube, translate(mat4(), vec3(x * 1.1f + 3, 0.0f, y * 1.1f)), true));
		}
	}

	DXR_ACS_OBJ** ACS_OBJ_house = new DXR_ACS_OBJ*[housesize];
	for (int i = 0; i < housesize; ++i)
	{
		ACS_OBJ_house[i] = new DXR_ACS_OBJ(graphicsDevice, curvertexbuffer[i], curindexbuffer[i], mat4(), true);
	}

	int curACScount = 0;
    ACS_top = new DXR_ACS_TOP(1);
	ACS_top->Add_ACS_OBJ_multiple(ACS_OBJ_cubes.data(), ACS_OBJ_cubes.size());
	ACS_top->Add_ACS_OBJ(ACS_OBJ_plane);
	ACS_top->Add_ACS_OBJ_multiple(ACS_OBJ_house, housesize / 1);
    ACS_top->Build(graphicsDevice);

	DXR_COL_pipelineState = new DXR_PipelineState(1);
    DXR_RootSignature RS_RayGen, RS_TriangleHit, RS_PlaneHit;

    // Ray Gen
    std::vector<DescriptorRange> RS_RayGen_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 8, 3}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 11, 4}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 7, 5}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 3}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 4} };
    RS_RayGen.AddRP_DescriptorTable(RS_RayGen_DR);
    RS_RayGen.AddRP_CBV(0);
	RS_RayGen.Add_Sampler(D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0);
    RS_RayGen.Create();

    // Triangle Hit
    RS_TriangleHit.AddRP_SRV(1);
    RS_TriangleHit.AddRP_SRV(2);
    RS_TriangleHit.AddRP_CBV(1);
    RS_TriangleHit.Create();

    // Plane Hit
    RS_PlaneHit.AddRP_SRV(1);
    RS_PlaneHit.AddRP_CBV(1);
    RS_PlaneHit.Create();

    std::vector<LPCWSTR> shaders{ L"rayGen", L"chs", L"planeChs", L"miss"};

    DXR_COL_pipelineState->SetShaders(L"DXR_Shader.hlsl", shaders);
    DXR_COL_pipelineState->AddHitGroup(nullptr, L"chs", L"HitGroup");
    DXR_COL_pipelineState->AddHitGroup(nullptr, L"planeChs", L"PlaneHitGroup");
    DXR_COL_pipelineState->AddRootSignature2Shader(L"rayGen", RS_RayGen.RS_desc);
    DXR_COL_pipelineState->AddRootSignature2Shader(L"chs", RS_TriangleHit.RS_desc, L"HitGroup");
    DXR_COL_pipelineState->AddRootSignature2Shader(L"planeChs", RS_PlaneHit.RS_desc, L"PlaneHitGroup");
    DXR_COL_pipelineState->Build(graphicsDevice);

	

	DXR_AO_pipelineState = new DXR_PipelineState(1);
	RS_RayGen = RS_TriangleHit = RS_PlaneHit = {};

	// Ray Gen
	RS_RayGen_DR = { {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 4}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 5}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11, 3} };
	RS_RayGen.AddRP_DescriptorTable(RS_RayGen_DR);
	RS_RayGen.AddRP_CBV(0);
	RS_RayGen.Create();

	std::vector<LPCWSTR> shaders2{ L"rayGen", L"shadowChs", L"shadowMiss" };

	DXR_AO_pipelineState->SetShaders(L"DXR_Shader_AO.hlsl", shaders2);
	DXR_AO_pipelineState->AddHitGroup(nullptr, L"shadowChs", L"ShadowHitGroup");
	DXR_AO_pipelineState->AddRootSignature2Shader(L"rayGen", RS_RayGen.RS_desc);
	DXR_AO_pipelineState->Build(graphicsDevice);
    
    //Create and initialize Resources
    mpSrvUavHeap = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    mpSrvUavHeap->Create(graphicsDevice, 12);


	NewTexture2D_Create(outputtex_RT, graphicsDevice, DXGI_FORMAT_R8G8B8A8_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(outputAOtex_RT, graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(outputtex_CS, graphicsDevice, DXGI_FORMAT_R8G8B8A8_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(posOutput, graphicsDevice, DXGI_FORMAT_R32_FLOAT, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(oldposOutput, graphicsDevice, DXGI_FORMAT_R32_FLOAT, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(AOtex_old, graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(AOtex_OUT, graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(normaltex, graphicsDevice, DXGI_FORMAT_R8G8B8A8_SNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(AOtex_vertical, graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(AOtex_F, graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	NewTexture2D_Create(TS_strength, graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);

    mpSrvUavHeap->SetTexture2D(outputtex_RT, 0);
    mpSrvUavHeap->SetTexture2D(outputtex_CS, 2);
    mpSrvUavHeap->SetTexture2D(posOutput, 3);
    mpSrvUavHeap->SetTexture2D(oldposOutput, 4);
    mpSrvUavHeap->SetTexture2D(outputAOtex_RT, 5);
    mpSrvUavHeap->SetTexture2D(AOtex_old, 6);
    mpSrvUavHeap->SetTexture2D(AOtex_OUT, 7);
	mpSrvUavHeap->SetTexture2D(normaltex, 8);
	mpSrvUavHeap->SetTexture2D(AOtex_vertical, 9);
	mpSrvUavHeap->SetTexture2D(AOtex_F, 10);
	mpSrvUavHeap->SetTexture2D(TS_strength, 11);

    mpSrvUavHeap->SetSRV(ACS_top->srvDesc, 1);

    CB_cameraWVP2 = new Buffer(graphicsDevice, sizeof(CameraMatrix), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
	CB_CScamera = new Buffer(graphicsDevice, sizeof(CS_camera), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
    
	DXR_COL_shadertable = new DXR_ShaderTable(ACS_top);
    UINT64 rayGen_data[] = { mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_CScamera->buffer->GetGPUVirtualAddress() };
	DXR_COL_shadertable->SetRayGenShader(L"rayGen", rayGen_data, arraysize(rayGen_data));
	DXR_COL_shadertable->SetMissShader(0, L"miss", nullptr, 0);

    UINT64 plane_data[] = { vertexBuffer2->buffer->GetGPUVirtualAddress(), ACS_OBJ_plane->transBuf->buffer->GetGPUVirtualAddress() };
	DXR_COL_shadertable->SetGeomData(ACS_OBJ_plane, 0, plane_data, L"PlaneHitGroup");
    
    for (int i = 0; i < ACS_OBJ_cubes.size(); ++i)
    {
        UINT64* triangle_data = new UINT64[4]{ vertexBuffer->buffer->GetGPUVirtualAddress(), indexBuffer->buffer->GetGPUVirtualAddress(), ACS_OBJ_cubes[i]->transBuf->buffer->GetGPUVirtualAddress() };
		DXR_COL_shadertable->SetGeomData(ACS_OBJ_cubes[i], 0, triangle_data, L"HitGroup");
    }

	for (int i = 0; i < housesize / 1; ++i)
	{
		UINT64* house000_data = new UINT64[4]{ curvertexbuffer[i]->buffer->GetGPUVirtualAddress(), curindexbuffer[i]->buffer->GetGPUVirtualAddress(), ACS_OBJ_house[i]->transBuf->buffer->GetGPUVirtualAddress() };
		DXR_COL_shadertable->SetGeomData(ACS_OBJ_house[i], 0, house000_data, L"HitGroup");
	}

	DXR_COL_shadertable->Create(graphicsDevice, DXR_COL_pipelineState);

	// Ambient Occlusion
	DXR_AO_shadertable = new DXR_ShaderTable(ACS_top);
	UINT64 rayGen_data2[] = { mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_cameraWVP2->buffer->GetGPUVirtualAddress() };
	DXR_AO_shadertable->SetRayGenShader(L"rayGen", rayGen_data2, arraysize(rayGen_data2));
	DXR_AO_shadertable->SetMissShader(0, L"shadowMiss", nullptr, 0);

	for (int i = 0; i < ACS_top->ACS_OBJ_buffers.size(); ++i)
	{
		DXR_AO_shadertable->SetGeomData(ACS_top->ACS_OBJ_buffers[i], 0, nullptr, L"ShadowHitGroup");
	}

	DXR_AO_shadertable->Create(graphicsDevice, DXR_AO_pipelineState);

	// Temporal Sampling

	emptyVB = new VertexBuffer<XMFLOAT4>();
	emptyVB->CreateFullScreen(graphicsDevice);

	Shader temporalSamplingShader;
	temporalSamplingShader.Load(L"TemporalSampling.hlsl", "main", "ps_5_0");
	Shader basic_VS;
	basic_VS.Load(L"VertexShader.hlsl", "main", "vs_5_0");
    
    compute_RSD = new DXR_RootSignature();
    std::vector<DescriptorRange> RS_compute_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 1},  {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6, 0} };
    compute_RSD->AddRP_DescriptorTable(RS_compute_DR);
    compute_RSD->CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	compute_RSD->Build(graphicsDevice);

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	compute_PSO = new PipelineState();
	compute_PSO->AddRTV(DXGI_FORMAT_R8G8B8A8_UNORM);
	compute_PSO->SetShaders(&basic_VS, &temporalSamplingShader);
	compute_PSO->Create(graphicsDevice, inputLayout, (int)sizeof(inputLayout), compute_RSD, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// BilateralFilter

	//vertical
	Shader BSV_shader;
    
	BSV_shader.Load(L"BilateralFilter_vertical.hlsl", "main", "ps_5_0");

	BFV_RS = new DXR_RootSignature();
	std::vector<DescriptorRange> BFV_RS_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 3}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 9, 2} };
	BFV_RS->AddRP_DescriptorTable(BFV_RS_DR);
	BFV_RS->CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	BFV_RS->Build(graphicsDevice);

	BFV_PS = new PipelineState();
	BFV_PS->AddRTV(DXGI_FORMAT_R8G8B8A8_UNORM);
	BFV_PS->SetShaders(&basic_VS, &BSV_shader);
	BFV_PS->Create(graphicsDevice, inputLayout, (int)sizeof(inputLayout), BFV_RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	//final
	Shader BS_shader;
	BS_shader.Load(L"BilateralFilter.hlsl", "main", "ps_5_0");
	

	BF_RS = new DXR_RootSignature();
	std::vector<DescriptorRange> BF_RS_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 3}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 7, 2} };
	BF_RS->AddRP_DescriptorTable(BF_RS_DR);
	BF_RS->CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	BF_RS->Build(graphicsDevice);

	BF_PS = new PipelineState();
	BF_PS->AddRTV(DXGI_FORMAT_R8G8B8A8_UNORM);
	BF_PS->SetShaders(&basic_VS, &BS_shader);
	BF_PS->Create(graphicsDevice, inputLayout, (int)sizeof(inputLayout), BF_RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);
	graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
	d3d_call(graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue));

    

	viewport = { 0.f, 0.f, (float)Width, (float)Height, 0.0f, 1.0f };
	scissorRect = { 0, 0, Width, Height };
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)Width / (float)Height, 0.001f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);
    
    return true;
}

void DetectInput()
{
    GetKeyboardState((PBYTE)keyboardstate);

    if (keyboardstate[0x41] & 0x80) // A
    {
        XMFLOAT4 dif = XMFLOAT4((float)-cos(camrot.x) * 20 * cameraspeed, 0, (float)-sin(camrot.x) * 20 * cameraspeed, 0);
        camerapos_vec += XMLoadFloat4(&dif);
    }
    if (keyboardstate[0x44] & 0x80) // D
    {
        XMFLOAT4 dif = XMFLOAT4((float)cos(camrot.x) * 20 * cameraspeed, 0, (float)sin(camrot.x) * 20 * cameraspeed, 0);
        camerapos_vec += XMLoadFloat4(&dif);
    }
    if (keyboardstate[VK_SPACE] & 0x80)
    {
        XMFLOAT4 dif = XMFLOAT4(0, 20 * cameraspeed, 0, 0);
        camerapos_vec += XMLoadFloat4(&dif);
    }
    if (keyboardstate[VK_LCONTROL] & 0x80)
    {
        XMFLOAT4 dif = XMFLOAT4(0, 20 * cameraspeed, 0, 0);
        camerapos_vec -= XMLoadFloat4(&dif);
    }
    cameraspeed = ((keyboardstate[VK_LSHIFT] & 0x80) ? 0.3f : 0.03f) * 0.05f;
    GetCursorPos((LPPOINT)&mousepos);
    if (camerabewegen)
    {
        int changed = 0;
        LONG deltax, deltay;
        deltax = mousepos.x - cameramousepos.x;
        deltay = mousepos.y - cameramousepos.y;
        mouserotationbuffer.x += 0.003f * deltax;
        mouserotationbuffer.y += 0.003f * deltay;
        if (mouserotationbuffer.y < degree2rad(-88))
            mouserotationbuffer.y = mouserotationbuffer.y - (mouserotationbuffer.y - degree2rad(-88));
        if (mouserotationbuffer.y > degree2rad(88))
            mouserotationbuffer.y = mouserotationbuffer.y - (mouserotationbuffer.y - degree2rad(88));
        if (cameramousepos.x != mousepos.x || cameramousepos.y != mousepos.y)
            changed = 1;
        camrot = XMFLOAT3(-mouserotationbuffer.x, -mouserotationbuffer.y, 0);
        if (changed)
            SetCursorPos(mousepointpos.x, mousepointpos.y);
    }
    if ((GetKeyState(VK_RBUTTON) & 0x100))
    {
        if (camerabewegen == false)
        {
            camerabewegen = true;
            cameramousepos = mousepos;
            mousepointpos.x = mousepos.x;
            mousepointpos.y = mousepos.y;
        }
    }
    if (!(GetKeyState(VK_RBUTTON) & 0x100) && camerabewegen)
        camerabewegen = false;

    XMMATRIX rotXMat = XMMatrixRotationX(0.0001f);
    XMFLOAT4 t = XMFLOAT4(0, 0, 1000, 0);

    XMMATRIX rotationMatrix = XMMatrixRotationX(-camrot.y) * XMMatrixRotationY(-camrot.x);
    XMVECTOR transformedReference = XMVector4Transform(XMLoadFloat4(&t), rotationMatrix);
    XMVECTOR cameraLookat = camerapos_vec + transformedReference;

    if (keyboardstate[0x57] & 0x80) // W
    {
        XMVECTOR camerablickrichtung = camerapos_vec - cameraLookat;
        camerablickrichtung = camerablickrichtung / XMVector4Length(camerablickrichtung);
        XMVECTOR dif = camerablickrichtung * 20 * cameraspeed;
        camerapos_vec -= dif;
        cameraLookat -= dif;
    }
    if (keyboardstate[0x53] & 0x80) // S
    {
        XMVECTOR camerablickrichtung = camerapos_vec - cameraLookat;
        camerablickrichtung = camerablickrichtung / XMVector4Length(camerablickrichtung);
        XMVECTOR dif = camerablickrichtung * 20 * cameraspeed;
        camerapos_vec += dif;
        cameraLookat += dif;
    }
    XMVECTOR cUp = XMLoadFloat4(&cameraUp);
    cameraRotMat = rotationMatrix;
    cameraViewMat = XMMatrixLookAtLH(camerapos_vec, cameraLookat, cUp);


    char* buf = oldkeyboardstate;
    oldkeyboardstate = keyboardstate;
    keyboardstate = oldkeyboardstate;

    POINT buf2 = oldmousepos;
    oldmousepos = mousepos;
    mousepos = buf2;
}

void Update()
{
	graphicsDevice->WaitForEventCompletion(graphicsDevice->frameIndex);

    DetectInput();

    XMStoreFloat4(&(cameraWVP.offset), camerapos_vec); 
    XMStoreFloat4x4(&(cameraWVP.rot), cameraRotMat);
    cameraWVP.counter = counter;

    uint8_t* pData;
    d3d_call(CB_cameraWVP2->buffer->Map(0, nullptr, (void**)&pData));
    memcpy(pData, &cameraWVP, sizeof(cameraWVP));
    CB_cameraWVP2->buffer->Unmap(0, nullptr);

    cuberotation += 0.002f;
    mat4 newrotmatrix = glm::translate(vec3(-1, 0, 0)) * glm::eulerAngleX(cuberotation) * glm::eulerAngleY(cuberotation * 0.7f) * glm::eulerAngleY(cuberotation * 0.3f) * glm::translate(vec3(2, 0, 0));
	ACS_OBJ_cubes[1]->ChangeTrans(newrotmatrix);

    XMStoreFloat4(&(CScamera.offset), camerapos_vec);
    XMStoreFloat4x4(&(CScamera.rot), cameraRotMat);
    XMStoreFloat4(&(CScamera.oldoffset), oldcamerapos_vec);
    XMStoreFloat4x4(&(CScamera.oldrot_inv), XMMatrixInverse(nullptr, oldrot));
    XMStoreFloat4x4(&(CScamera.oldrot), oldrot);
    

    d3d_call(CB_CScamera->buffer->Map(0, nullptr, (void**)&pData));
    memcpy(pData, &CScamera, sizeof(CScamera));
    CB_CScamera->buffer->Unmap(0, nullptr);

    

    counter++;
    counter = counter % 128;
    oldrot = cameraRotMat; 
    oldcamerapos_vec = camerapos_vec;
}

void UpdatePipeline()
{
    graphicsDevice->frameIndex = graphicsDevice->swapChain->swapChain->GetCurrentBackBufferIndex();

	// Init
    d3d_call(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator->Reset())
    d3d_call(graphicsDevice->commandList->commandList->Reset(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator, nullptr))

	ACS_top->Update(graphicsDevice);

	graphicsDevice->commandList->SetDescriptorHeaps({ mpSrvUavHeap->descriptorHeap });

	graphicsDevice->commandList->commandList->CopyResource(oldposOutput->buffer, posOutput->buffer);
	graphicsDevice->commandList->commandList->CopyResource(AOtex_old->buffer, AOtex_OUT->buffer);

	graphicsDevice->commandList->SetViewPortScissorRect(viewport, scissorRect);



	// Raytracing
    D3D12_DISPATCH_RAYS_DESC raytraceDesc = DXR_COL_shadertable->CreateRayDesc();
    graphicsDevice->commandList->commandList->SetComputeRootSignature(DXR_COL_pipelineState->emptyRootSig);
    graphicsDevice->commandList->commandList->SetPipelineState1(DXR_COL_pipelineState->pipelineState);
    graphicsDevice->commandList->commandList->DispatchRays(&raytraceDesc);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = outputtex_RT->buffer;
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &uavBarrier);


	raytraceDesc = DXR_AO_shadertable->CreateRayDesc();
	graphicsDevice->commandList->commandList->SetComputeRootSignature(DXR_AO_pipelineState->emptyRootSig);
	graphicsDevice->commandList->commandList->SetPipelineState1(DXR_AO_pipelineState->pipelineState);
	graphicsDevice->commandList->commandList->DispatchRays(&raytraceDesc);

	uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = outputAOtex_RT->buffer;
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &uavBarrier);
	

	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { oldposOutput->buffer, AOtex_old->buffer });

	//D3D12_RESOURCE_BARRIER uavBarrier = {};
	//uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	//uavBarrier.UAV.pResource = oldposOutput->buffer;
	//graphicsDevice->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	//uavBarrier = {};
	//uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	//uavBarrier.UAV.pResource = AOtex_old->buffer;
	//graphicsDevice->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	//Temporal Sampling
	graphicsDevice->commandList->commandList->SetPipelineState(compute_PSO->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(compute_RSD->RS);
	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	graphicsDevice->commandList->DrawInstanced(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, emptyVB, 6);

	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, { oldposOutput->buffer, AOtex_old->buffer });
	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, outputtex_CS->buffer);


	//BF_vertical
	graphicsDevice->commandList->commandList->SetPipelineState(BFV_PS->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(BFV_RS->RS);
	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	graphicsDevice->commandList->DrawInstanced(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, emptyVB, 6);


	//BF_final
	graphicsDevice->commandList->commandList->SetPipelineState(BF_PS->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(BF_RS->RS);
	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	graphicsDevice->commandList->DrawInstanced(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, emptyVB, 6);


	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, outputtex_CS->buffer);
	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST, graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex]);
	graphicsDevice->commandList->commandList->CopyResource(graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex], outputtex_CS->buffer);
	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT, graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex]);
    

    bool hh = graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);

    graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
    graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue);
}
void Render()
{
	// Sleep(500);
    UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

    d3d_call(graphicsDevice->swapChain->swapChain->Present(0, 0)); // Present current frame
}

void Cleanup()
{

}