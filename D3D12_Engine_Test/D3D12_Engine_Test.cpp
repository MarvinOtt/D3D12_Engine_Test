#include "D3D12_Engine_Test.h"
#include <sstream>
#include <fstream>
#include <sstream>

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

struct AccelerationStructureBuffers2
{
    Buffer* pScratch;
    Buffer* pResult;
    Buffer* pInstanceDesc;    // Used only for top-level AS
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{


    //create the window
	mainWindow = new Window(hInstance);
	mainWindow->Create(FullScreen, L"DXR_Test", Width, Height, 2560 * 1);
	mainWindow->Show(nShowCmd);
    // initialize direct3d
    if (!InitD3D())
    {
        MessageBox(0, L"Failed to initialize direct3d 12", L"Error", MB_OK);
        Cleanup();
        return 1;
    }

    if (!InitDirectInput(hInstance))
    {
        MessageBox(0, L"Direct Input Initialization - Failed", L"Error", MB_OK);
        return 0;
    }

    // start the main loop
    mainloop();

    // close the fence event
    CloseHandle(Fence::fenceEvent);

    // clean up everything
    Cleanup();

    return 0;
}


void mainloop() {
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
        else {
            // run game code
            Update(); // update the game logic
            if (!IsRunning)
                return;
            Render(); // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
            if (!IsRunning)
                return;
        }
    }
}

float degree2rad(float angle_in_degrees) {
    return angle_in_degrees * (PI / 180.0f);
}

bool InitDirectInput(HINSTANCE hInstance) {
    keyboardstate = new char[256];
    oldkeyboardstate = new char[256];
    return true;
}

bool InitD3D()
{
    HRESULT hr;

    graphicsDevice = new GraphicsDevice(mainWindow->hwnd, Width, Height);
    graphicsDevice->Create();


	//Load models
	int housesize = 272;
	VertexBuffer<Vertex>** curvertexbuffer = new VertexBuffer<Vertex>*[housesize];
	IndexBuffer<int>** curindexbuffer = new IndexBuffer<int>*[housesize];
	PLYData** plyIn = new PLYData*[housesize];


	for (int j = 0; j < housesize; ++j)
	{
		std::stringstream ss;
		ss << std::setw(3) << std::setfill('0') << (j + 0);
		std::string s = ss.str();

		PLYData* plyIn = new PLYData("C:\\Users\\marvi\\Documents\\DXR4\\models\\Mesh" + s + ".ply");

		std::vector<double> xPos = plyIn->getElement("vertex").getProperty<double>("x");
		std::vector<double> yPos = plyIn->getElement("vertex").getProperty<double>("y");
		std::vector<double> zPos = plyIn->getElement("vertex").getProperty<double>("z");

		Vertex* vPos = new Vertex[xPos.size()];

		for (size_t i = 0; i < xPos.size(); i++) {
			vPos[i] = Vertex(xPos[i], yPos[i], zPos[i], 1, 1, 1);
		}

		curvertexbuffer[j] = new VertexBuffer<Vertex>();
		curvertexbuffer[j]->Create(graphicsDevice, xPos.size() * sizeof(Vertex));
		curvertexbuffer[j]->SetData(graphicsDevice, vPos, xPos.size());

		std::vector<int> vFaces = plyIn->getElement("face").getListProperty2<int>("vertex_indices");

		curindexbuffer[j] = new IndexBuffer<int>();
		curindexbuffer[j]->Create(graphicsDevice, vFaces.size() * sizeof(int));
		curindexbuffer[j]->SetData(graphicsDevice, vFaces.data(), vFaces.size());
	}



    // a cube
    Vertex vList[] = {
        // front face
        { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
        {  0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
        {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f },

        // right side face
        {  0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },

        // left side face
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
    vertexBuffer->SetData(graphicsDevice, vList, sizeof(vList) / sizeof(Vertex));

    DWORD iList[] = {
        // ffront face
        0, 1, 2, // first triangle
        0, 3, 1, // second triangle

        // left face
        4, 5, 6, // first triangle
        4, 7, 5, // second triangle

        // right face
        8, 9, 10, // first triangle
        8, 11, 9, // second triangle

        // back face
        12, 13, 14, // first triangle
        12, 15, 13, // second triangle

        // top face
        16, 17, 18, // first triangle
        16, 19, 17, // second triangle

        // bottom face
        20, 21, 22, // first triangle
        20, 23, 21, // second triangle
    };

    numCubeIndices = sizeof(iList) / sizeof(DWORD);

    indexBuffer = new IndexBuffer<DWORD>();
    indexBuffer->Create(graphicsDevice, sizeof(iList));
    indexBuffer->SetData(graphicsDevice, iList, sizeof(iList) / sizeof(DWORD));

    // a plane
    Vertex vList2[] = {
        // bottom face
        { -100, -0.5f, -100, 0.0f, 0.0f, 0.0f },
        { -100, -0.5f,  100, 0.6f, 0.6f, 0.6f },
        {  100, -0.5f,  100, 0.6f, 0.6f, 0.6f },
        { -100, -0.5f, -100, 0.0f, 0.0f, 0.0f },
        {  100, -0.5f,  100, 0.6f, 0.6f, 0.6f },
        {  100, -0.5f, -100, 0.6f, 0.6f, 0.6f }
    };

    vertexBuffer2 = new VertexBuffer<Vertex>();
    vertexBuffer2->Create(graphicsDevice, sizeof(vList2));
    vertexBuffer2->SetData(graphicsDevice, vList2, sizeof(vList2) / sizeof(Vertex));

    // ACCELERATION STRUCTURES

    ACS_cube = new DXR_ACS_BOT();
    ACS_cube->AddGeometryIndices(vertexBuffer, indexBuffer);
    ACS_cube->Build(graphicsDevice);

    ACS_plane = new DXR_ACS_BOT();
    ACS_plane->AddGeometry(vertexBuffer2);
    ACS_plane->Build(graphicsDevice);

	DXR_ACS_BOT** ACS_house = new DXR_ACS_BOT*[housesize];
	for (int i = 0; i < housesize; ++i)
	{
		ACS_house[i] = new DXR_ACS_BOT();
		ACS_house[i]->AddGeometryIndices(curvertexbuffer[i], curindexbuffer[i]);
		ACS_house[i]->Build(graphicsDevice);
	}

    mat4 transformation[3];
    transformation[0] = mat4(); // Identity
    transformation[1] = translate(mat4(), vec3(-2, 100, 1));
    transformation[2] = translate(mat4(), vec3(2, 0.1f + 0.0f, 0)) * glm::eulerAngleX(0.3f);

	int curACScount = 0;
    ACS_top = new DXR_ACS_TOP(2);
    uint8_t* pData;
    CB_objectmatrices = new Buffer*[4 + 10*10 + housesize];
    LPCWSTR hitGroups[] = { L"PlaneHitGroup", L"ShadowHitGroup" };
    ACS_top->Add_ACS_BOT(ACS_plane, transformation[0], hitGroups);
    CB_objectmatrices[curACScount] = new Buffer(graphicsDevice, sizeof(ObjectMatrix), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
    d3d_call(CB_objectmatrices[curACScount]->buffer->Map(0, nullptr, (void**)&pData));
    memcpy(pData, &transformation[0], sizeof(ObjectMatrix));
    CB_objectmatrices[curACScount]->buffer->Unmap(0, nullptr);
	curACScount++;

    LPCWSTR triHitGroups[] = { L"HitGroup", L"ShadowHitGroup" };
    for (int i = 0; i < 3; ++i)
    {
		if(i == 0)
			ACS_top->Add_ACS_BOT(ACS_cube, translate(mat4(), vec3(1, 0.0f, 0)), triHitGroups);
		else
			ACS_top->Add_ACS_BOT(ACS_cube, transformation[i], triHitGroups);
        CB_objectmatrices[curACScount] = new Buffer(graphicsDevice, sizeof(ObjectMatrix), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
        d3d_call(CB_objectmatrices[curACScount]->buffer->Map(0, nullptr, (void**)&pData));
        memcpy(pData, &transformation[i], sizeof(ObjectMatrix));
        CB_objectmatrices[curACScount]->buffer->Unmap(0, nullptr);
		curACScount++;
    }

    for (int x = 0; x < 10; ++x)
    {
        for (int y = 0; y < 10; ++y)
        {
            mat4 trans = translate(mat4(), vec3(x * 1.1f + 3, 0.0f, y * 1.1f));
            ACS_top->Add_ACS_BOT(ACS_cube, trans, triHitGroups);
            CB_objectmatrices[curACScount] = new Buffer(graphicsDevice, sizeof(ObjectMatrix), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
            d3d_call(CB_objectmatrices[curACScount]->buffer->Map(0, nullptr, (void**)&pData));
            memcpy(pData, &trans, sizeof(ObjectMatrix));
            CB_objectmatrices[curACScount]->buffer->Unmap(0, nullptr);
			curACScount++;
        }
    }
	for (int i = 0; i < housesize; ++i)
	{
		ACS_top->Add_ACS_BOT(ACS_house[i], transformation[0], triHitGroups);
		CB_objectmatrices[curACScount] = new Buffer(graphicsDevice, sizeof(ObjectMatrix), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		d3d_call(CB_objectmatrices[curACScount]->buffer->Map(0, nullptr, (void**)&pData));
		memcpy(pData, &transformation[0], sizeof(ObjectMatrix));
		CB_objectmatrices[curACScount]->buffer->Unmap(0, nullptr);
		curACScount++;
	}

    ACS_top->Build(graphicsDevice);

    DXR_pipelineState = new DXR_PipelineState(2);
    DXR_RootSignature RS_RayGen, RS_TriangleHit, RS_PlaneHit;

    // Ray Gen
    std::vector<DescriptorRange> RS_RayGen_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 8, 3} };
    RS_RayGen.AddRP_DescriptorTable(RS_RayGen_DR);
    RS_RayGen.AddRP_CBV(0);
    RS_RayGen.Create();

    // Triangle Hit
    std::vector<DescriptorRange> RS_TriHit_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0} };
    RS_TriangleHit.AddRP_SRV(1);
    RS_TriangleHit.AddRP_SRV(2);
    RS_TriangleHit.AddRP_DescriptorTable(RS_TriHit_DR);
    RS_TriangleHit.AddRP_CBV(1);
    RS_TriangleHit.Create();

    // Plane Hit
    std::vector<DescriptorRange> RS_PlaneHit_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0} };
    RS_PlaneHit.AddRP_SRV(1);
    RS_PlaneHit.AddRP_DescriptorTable(RS_PlaneHit_DR);
    RS_PlaneHit.AddRP_CBV(1);
    RS_PlaneHit.Create();

    std::vector<LPCWSTR> shaders{ L"rayGen", L"chs", L"planeChs", L"shadowChs", L"miss", L"shadowMiss"};

    DXR_pipelineState->SetShaders(L"DXR_Shader.hlsl", shaders);
    DXR_pipelineState->AddHitGroup(nullptr, L"chs", L"HitGroup");
    DXR_pipelineState->AddHitGroup(nullptr, L"planeChs", L"PlaneHitGroup");
    DXR_pipelineState->AddHitGroup(nullptr, L"shadowChs", L"ShadowHitGroup");
    DXR_pipelineState->AddRootSignature2Shader(L"rayGen", RS_RayGen.RS_desc);
    DXR_pipelineState->AddRootSignature2Shader(L"chs", RS_TriangleHit.RS_desc, L"HitGroup");
    DXR_pipelineState->AddRootSignature2Shader(L"planeChs", RS_PlaneHit.RS_desc, L"PlaneHitGroup");
    DXR_pipelineState->Build(graphicsDevice);
    
    //Create and initialize Resources
    mpSrvUavHeap = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    mpSrvUavHeap->Create(graphicsDevice, 10);

    outputtex_RT = new Texture2D();
    outputtex_RT->Create(graphicsDevice, DXGI_FORMAT_R8G8B8A8_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    outputAOtex_RT = new Texture2D();
    outputAOtex_RT->Create(graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    outputtex_CS = new Texture2D();
    outputtex_CS->Create(graphicsDevice, DXGI_FORMAT_R8G8B8A8_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    posOutput = new Texture2D();
    posOutput->Create(graphicsDevice, DXGI_FORMAT_R32_FLOAT, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    oldposOutput = new Texture2D();
    oldposOutput->Create(graphicsDevice, DXGI_FORMAT_R32_FLOAT, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    AOtex_old = new Texture2D();
    AOtex_old->Create(graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    AOtex_OUT = new Texture2D();
    AOtex_OUT->Create(graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	normaltex = new Texture2D();
	normaltex->Create(graphicsDevice, DXGI_FORMAT_R8G8B8A8_SNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
	AOtex_vertical = new Texture2D();
	AOtex_vertical->Create(graphicsDevice, DXGI_FORMAT_R16_UNORM, graphicsDevice->bufferWidth, graphicsDevice->bufferHeight);
    graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outputtex_CS->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE));
    graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outputtex_RT->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(posOutput->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(oldposOutput->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normaltex->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(AOtex_vertical->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


    mpSrvUavHeap->SetTexture2D(graphicsDevice, outputtex_RT, 0);
    mpSrvUavHeap->SetTexture2D(graphicsDevice, outputtex_CS, 2);
    mpSrvUavHeap->SetTexture2D(graphicsDevice, posOutput, 3);
    mpSrvUavHeap->SetTexture2D(graphicsDevice, oldposOutput, 4);
    mpSrvUavHeap->SetTexture2D(graphicsDevice, outputAOtex_RT, 5);
    mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_old, 6);
    mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_OUT, 7);
	mpSrvUavHeap->SetTexture2D(graphicsDevice, normaltex, 8);
	mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_vertical, 9);


    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location = ACS_top->ACS_buffer->pResult->buffer->GetGPUVirtualAddress();
    mpSrvUavHeap->SetSRV(graphicsDevice, srvDesc, 1);

    CB_cameraWVP2 = new Buffer(graphicsDevice, sizeof(CameraMatrix), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

	int curgeomcount = 0;
    DXR_shaderTable = new DXR_ShaderTable(ACS_top);
    UINT64 rayGen_data[] = { mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_cameraWVP2->buffer->GetGPUVirtualAddress() };
    DXR_shaderTable->SetRayGenShader(L"rayGen", rayGen_data, arraysize(rayGen_data));
    DXR_shaderTable->SetMissShader(0, L"miss", nullptr, 0);
    DXR_shaderTable->SetMissShader(1, L"shadowMiss", nullptr, 0);

    UINT64 plane_data[] = { vertexBuffer2->buffer->GetGPUVirtualAddress(), mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_objectmatrices[curgeomcount]->buffer->GetGPUVirtualAddress() };
    DXR_shaderTable->SetGeomData(curgeomcount, 0, plane_data);
	curgeomcount++;
    
    for (int j = 0; j < 3; ++j)
    {
        UINT64* triangle_data = new UINT64[4]{ vertexBuffer->buffer->GetGPUVirtualAddress(), indexBuffer->buffer->GetGPUVirtualAddress(), mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_objectmatrices[curgeomcount]->buffer->GetGPUVirtualAddress() };
        DXR_shaderTable->SetGeomData(curgeomcount, 0, triangle_data);
		curgeomcount++;
    }

    for (int x = 0; x < 10; ++x)
    {
        for (int y = 0; y < 10; ++y)
        {
            UINT64* triangle_data = new UINT64[4]{ vertexBuffer->buffer->GetGPUVirtualAddress(), indexBuffer->buffer->GetGPUVirtualAddress(), mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_objectmatrices[curgeomcount]->buffer->GetGPUVirtualAddress() };
            DXR_shaderTable->SetGeomData(curgeomcount, 0, triangle_data);
			curgeomcount++;
        }
    }
	for (int i = 0; i < housesize; ++i)
	{
		UINT64* house000_data = new UINT64[4]{ curvertexbuffer[i]->buffer->GetGPUVirtualAddress(), curindexbuffer[i]->buffer->GetGPUVirtualAddress(), mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr, CB_objectmatrices[curgeomcount]->buffer->GetGPUVirtualAddress() };
		DXR_shaderTable->SetGeomData(curgeomcount, 0, house000_data);
		curgeomcount++;
	}

    DXR_shaderTable->Create(graphicsDevice, DXR_pipelineState);

	// Temporal Sampling

	emptyVB = new VertexBuffer<XMFLOAT4>();
	emptyVB->CreateFullScreen(graphicsDevice);

	CB_CScamera = new Buffer(graphicsDevice, sizeof(CS_camera), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

	Shader temporalSamplingShader;
	temporalSamplingShader.Load(L"TemporalSampling.hlsl", "main", "ps_5_0");
	Shader basic_VS;
	basic_VS.Load(L"VertexShader.hlsl", "main", "vs_5_0");

    compute_RSD = new DXR_RootSignature();
    std::vector<DescriptorRange> RS_compute_DR{ /*{D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 4},*/ {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 2}, /*{D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 3},*/ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 7, 6} };
    compute_RSD->AddRP_DescriptorTable(RS_compute_DR);
    compute_RSD->AddRP_CBV(0);
	compute_RSD->Add_Sampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0);
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
	std::vector<DescriptorRange> BFV_RS_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 3}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 9, 2} };
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
	std::vector<DescriptorRange> BF_RS_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 2}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 3}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 1} };
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

    

	// Initialize other variables
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
        float deltax, deltay;
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

    ACS_top->trans[1] = newrotmatrix;
    d3d_call(CB_objectmatrices[1]->buffer->Map(0, nullptr, (void**)&pData));
    memcpy(pData, &newrotmatrix, sizeof(ObjectMatrix));
    CB_objectmatrices[1]->buffer->Unmap(0, nullptr);

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
    //Sleep(100);
    HRESULT hr;
	//if (counter % 2 == 0)
	//{
	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, posOutput, 4);
	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, oldposOutput, 3);

	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_old, 7);
	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_OUT, 6);
	//}
	//else
	//{
	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, posOutput, 3);
	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, oldposOutput, 4);

	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_old, 6);
	//	mpSrvUavHeap->SetTexture2D(graphicsDevice, AOtex_OUT, 7);
	//}

	//graphicsDevice->WaitForEventCompletion(graphicsDevice->frameIndex);
    graphicsDevice->frameIndex = graphicsDevice->swapChain->swapChain->GetCurrentBackBufferIndex();


    d3d_call(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator->Reset());

    d3d_call(graphicsDevice->commandList->commandList->Reset(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator, nullptr));

    // Start recording commands into the command queue

    //ACS_top->Update(graphicsDevice);

    //D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    //srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //srvDesc.RaytracingAccelerationStructure.Location = ACS_top->ACS_buffer->pResult->buffer->GetGPUVirtualAddress();
    //mpSrvUavHeap->SetSRV(graphicsDevice, srvDesc, 1);



    ID3D12DescriptorHeap* heaps[] = { mpSrvUavHeap->descriptorHeap };
    graphicsDevice->commandList->commandList->SetDescriptorHeaps(arraysize(heaps), heaps);

    // Let's raytrace
    //graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outputtex->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


    //graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);

    //graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
    //graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue);
    //graphicsDevice->WaitForEventCompletion(graphicsDevice->frameIndex);
    //d3d_call(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator->Reset());

    //d3d_call(graphicsDevice->commandList->commandList->Reset(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator, nullptr));






    ////graphicsDevice->commandList->commandList->SetPipelineState(compute_PSO);
    ////graphicsDevice->commandList->commandList->SetComputeRootSignature(compute_RS);

    ////graphicsDevice->commandList->commandList->SetComputeRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
    ////graphicsDevice->commandList->commandList->SetComputeRootConstantBufferView(1, CB_CScamera->buffer->GetGPUVirtualAddress());


    ////graphicsDevice->commandList->commandList->Dispatch(768, 1, 1);



	graphicsDevice->commandList->commandList->CopyResource(oldposOutput->buffer, posOutput->buffer);
	graphicsDevice->commandList->commandList->CopyResource(AOtex_old->buffer, AOtex_OUT->buffer);


	//graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);

	//graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
	//graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue);
	//graphicsDevice->WaitForEventCompletion(graphicsDevice->frameIndex);
	//d3d_call(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator->Reset());

	//d3d_call(graphicsDevice->commandList->commandList->Reset(graphicsDevice->commandAllocator[graphicsDevice->frameIndex].commandAllocator, nullptr));

	//graphicsDevice->commandList->commandList->SetDescriptorHeaps(arraysize(heaps), heaps);

	//graphicsDevice->commandList->commandList->SetDescriptorHeaps(arraysize(heaps), heaps);

    //graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(oldposOutput->buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    //graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(AOtex_old->buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


    //graphicsDevice->commandList->commandList->CopyResource(oldposOutput->buffer, posOutput->buffer);

    D3D12_DISPATCH_RAYS_DESC raytraceDesc = DXR_shaderTable->CreateRayDesc();

    // Bind the empty root signature
    graphicsDevice->commandList->commandList->SetComputeRootSignature(DXR_pipelineState->emptyRootSig);

    // Dispatch
    graphicsDevice->commandList->commandList->SetPipelineState1(DXR_pipelineState->pipelineState);
    graphicsDevice->commandList->commandList->DispatchRays(&raytraceDesc);





	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(oldposOutput->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(AOtex_old->buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	graphicsDevice->commandList->commandList->SetPipelineState(compute_PSO->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(compute_RSD->RS);

	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	graphicsDevice->commandList->commandList->SetGraphicsRootConstantBufferView(1, CB_CScamera->buffer->GetGPUVirtualAddress());

	graphicsDevice->commandList->commandList->RSSetViewports(1, &viewport); // set the viewports
	graphicsDevice->commandList->commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	graphicsDevice->commandList->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphicsDevice->commandList->commandList->IASetVertexBuffers(0, 1, &emptyVB->bufferView);

	graphicsDevice->commandList->commandList->DrawInstanced(6, 1, 0, 0);





	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(oldposOutput->buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(AOtex_old->buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outputtex_CS->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));






	graphicsDevice->commandList->commandList->SetPipelineState(BFV_PS->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(BFV_RS->RS);

	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	graphicsDevice->commandList->commandList->RSSetViewports(1, &viewport); // set the viewports
	graphicsDevice->commandList->commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	graphicsDevice->commandList->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphicsDevice->commandList->commandList->IASetVertexBuffers(0, 1, &emptyVB->bufferView);

	graphicsDevice->commandList->commandList->DrawInstanced(6, 1, 0, 0);







	graphicsDevice->commandList->commandList->SetPipelineState(BF_PS->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(BF_RS->RS);

	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(0, mpSrvUavHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	graphicsDevice->commandList->commandList->RSSetViewports(1, &viewport); // set the viewports
	graphicsDevice->commandList->commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	graphicsDevice->commandList->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphicsDevice->commandList->commandList->IASetVertexBuffers(0, 1, &emptyVB->bufferView);

	graphicsDevice->commandList->commandList->DrawInstanced(6, 1, 0, 0);









	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outputtex_CS->buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST));
	graphicsDevice->commandList->commandList->CopyResource(graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex], outputtex_CS->buffer);
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT));

    
    
    

    bool hh = graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);

    graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
    graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue);
}
void Render()
{
    HRESULT hr;
    //Sleep(400);

    UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

    

    d3d_call(graphicsDevice->swapChain->swapChain->Present(1, 0));
}

void Cleanup()
{

}

//
//// get the dxgi format equivilent of a wic format
//DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
//{
//    if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
//
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
//    else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
//    else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
//    else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;
//
//    else return DXGI_FORMAT_UNKNOWN;
//}
//
//// get a dxgi compatible wic format from another wic format
//WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
//{
//    if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
//    else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
//    else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
//    else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
//    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
//    else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
//    else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
//    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
//    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
//    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
//
//#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
//#endif
//
//    else return GUID_WICPixelFormatDontCare;
//}
//
//// get the number of bits per pixel for a dxgi format
//int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
//{
//    if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
//    else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
//    else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
//    else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
//    else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
//    else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
//    else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;
//
//    else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
//    else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
//    else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
//    else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
//    else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
//    else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
//    else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
//    else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;
//}
//
//// load and decode image from file
//int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow)
//{
//    HRESULT hr;
//
//    // we only need one instance of the imaging factory to create decoders and frames
//    static IWICImagingFactory *wicFactory;
//
//    // reset decoder, frame and converter since these will be different for each image we load
//    IWICBitmapDecoder *wicDecoder = NULL;
//    IWICBitmapFrameDecode *wicFrame = NULL;
//    IWICFormatConverter *wicConverter = NULL;
//
//    bool imageConverted = false;
//
//    if (wicFactory == NULL)
//    {
//        // Initialize the COM library
//        CoInitialize(NULL);
//
//        // create the WIC factory
//        hr = CoCreateInstance(
//            CLSID_WICImagingFactory,
//            NULL,
//            CLSCTX_INPROC_SERVER,
//            IID_PPV_ARGS(&wicFactory)
//        );
//        if (FAILED(hr)) return 0;
//    }
//
//    // load a decoder for the image
//    hr = wicFactory->CreateDecoderFromFilename(
//        filename,                        // Image we want to load in
//        NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
//        GENERIC_READ,                    // We want to read from this file
//        WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
//        &wicDecoder                      // the wic decoder to be created
//    );
//    if (FAILED(hr)) return 0;
//
//    // get image from decoder (this will decode the "frame")
//    hr = wicDecoder->GetFrame(0, &wicFrame);
//    if (FAILED(hr)) return 0;
//
//    // get wic pixel format of image
//    WICPixelFormatGUID pixelFormat;
//    hr = wicFrame->GetPixelFormat(&pixelFormat);
//    if (FAILED(hr)) return 0;
//
//    // get size of image
//    UINT textureWidth, textureHeight;
//    hr = wicFrame->GetSize(&textureWidth, &textureHeight);
//    if (FAILED(hr)) return 0;
//
//    // we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
//    // out how to implement the support yourself
//
//    // convert wic pixel format to dxgi pixel format
//    DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);
//
//    // if the format of the image is not a supported dxgi format, try to convert it
//    if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
//    {
//        // get a dxgi compatible wic format from the current image format
//        WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);
//
//        // return if no dxgi compatible format was found
//        if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;
//
//        // set the dxgi format
//        dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);
//
//        // create the format converter
//        hr = wicFactory->CreateFormatConverter(&wicConverter);
//        if (FAILED(hr)) return 0;
//
//        // make sure we can convert to the dxgi compatible format
//        BOOL canConvert = FALSE;
//        hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
//        if (FAILED(hr) || !canConvert) return 0;
//
//        // do the conversion (wicConverter will contain the converted image)
//        hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
//        if (FAILED(hr)) return 0;
//
//        // this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
//        imageConverted = true;
//    }
//
//    int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
//    bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
//    int imageSize = bytesPerRow * textureHeight; // total image size in bytes
//
//    // allocate enough memory for the raw image data, and set imageData to point to that memory
//    *imageData = (BYTE*)malloc(imageSize);
//
//    // copy (decoded) raw image data into the newly allocated memory (imageData)
//    if (imageConverted)
//    {
//        // if image format needed to be converted, the wic converter will contain the converted image
//        hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
//        if (FAILED(hr)) return 0;
//    }
//    else
//    {
//        // no need to convert, just copy data from the wic frame
//        hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
//        if (FAILED(hr)) return 0;
//    }
//
//    // now describe the texture with the information we have obtained from the image
//    resourceDescription = {};
//    resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//    resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
//    resourceDescription.Width = textureWidth; // width of the texture
//    resourceDescription.Height = textureHeight; // height of the texture
//    resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
//    resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
//    resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
//    resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
//    resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
//    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
//    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags
//
//    // return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
//    return imageSize;
//}
