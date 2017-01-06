#include "Application.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;

	_pVertexBufferTri = nullptr;
	_pIndexBufferTri = nullptr;

	keyState = 0;
	shiftCamera = false;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
	//makes the window
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	XMStoreFloat4x4(&_world2, XMMatrixIdentity());

	eyex = 0.0f;
	eyey = 0.0f;
	eyez = -10.0f;

	atx = 0.0f;
	aty = 0.0f;
	atz = 0.0f;

	eyex2 = 0.0f;
	eyey2 = 0.0f;
	eyez2 = -10.0f;

	atx2 = 0.0f;
	aty2 = 0.0f;
	atz2 = 0.0f;

    // Initialize the view matrix
	Eye = XMVectorSet(eyex, eyey, eyez, 0.0f);
	At = XMVectorSet(atx, aty, atz, 0.0f);
	Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, //3 sets of 32 bit float data
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, //Normalised vertexes
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0} //Texel Coordinate data
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		// Front Face
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) }, //0
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) }, //1
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) }, //2
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 1.0f) }, //3

		// Right Face
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) }, //4
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(1.0f, 0.0f) }, //5
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) }, //6
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f) }, //7

		// Back Face
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f) }, //8
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(1.0f, 0.0f) }, //9
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT2(0.0f, 1.0f) }, //10
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 1.0f) }, //11

		// Left Face
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f) }, //12
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) }, //13
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) }, //14
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 1.0f) }, //15

		// Top Face
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f) }, //16
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(1.0f, 0.0f) }, //17
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) }, //18
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT2(1.0f, 1.0f) }, //19

		// Bottom Face
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) }, //20
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) }, //21
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) }, //22
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f) }, //23
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

//--
HRESULT Application::InitVertexBufferTri()
{
	HRESULT hr;

	static SimpleVertex vertices[4] =
	{
		// Top Face
		{ XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, //0
		{ XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) }, //1
		{ XMFLOAT3(-2.0f, -2.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) }, //2
		{ XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) }, //3
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferTri);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBufferTri()
{
	HRESULT hr;

	WORD indices[] =
	{
		//floor
		0, 1, 2,
		2, 1, 3,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferTri);

	if (FAILED(hr))
		return hr;

	return S_OK;
}
//--

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
		//front
        0, 1, 2,
		2, 1, 3,
		// right
		4, 5, 6,
		6, 5, 7,
		//back
		8, 9, 10,
		10, 9, 11,
		//left
		12, 13, 14,
		14, 13, 15,
		//top
		16, 17, 18,
		18, 17, 19,
		//bottom
		20, 21, 22,
		20, 23, 21,
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 1920, 1080};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd; //allows to render to a back buffer off screen and then swap with front screen
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView); //render target view is a texture that you can render to
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	//--
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);
	//--

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitIndexBuffer();

	InitVertexBufferTri();
	InitIndexBufferTri();

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);


	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));

	wfdesc.FillMode = D3D11_FILL_SOLID;
	//wfdesc.FillMode = D3D11_FILL_WIREFRAME;

	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	_pImmediateContext->RSSetState(_wireFrame);


    if (FAILED(hr))
        return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
	//--
	if (_pVertexBufferTri) _pVertexBufferTri->Release();
	if (_pIndexBufferTri) _pIndexBufferTri->Release();
	//--
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();

	//--
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	//--
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

	if (keyState == 0 || keyState == 1)
	{
		XMVECTOR Eye = XMVectorSet(eyex, eyey, eyez, 0.0f);
		XMVECTOR At = XMVectorSet(atx, aty, atz, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));
	}
	if (keyState == 2 || keyState == 3)
	{
		XMVECTOR Eye2 = XMVectorSet(eyex2, eyey2, eyez2, 0.0f);
		XMVECTOR At2 = XMVectorSet(atx2, aty2, atz2, 0.0f);
		XMVECTOR Up2 = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye2, At2, Up2));
	}

	float moveX = atx - eyex;
	float moveY = aty - eyey;
	float moveZ = atz - eyez;
	float move = (moveX * moveX) + (moveY * moveY) + (moveZ * moveZ);
	float mover = sqrt(move);
	moveX = moveX / (mover * 100);
	moveY = moveY / (mover * 100);
	moveZ = moveZ / (mover * 100);

	float moveX2 = atx2 - eyex2;
	float moveY2 = aty2 - eyey;
	float moveZ2 = atz2 - eyez;
	float move2 = (moveX2 * moveX2) + (moveY2 * moveY2) + (moveZ2 * moveZ2);
	float mover2 = sqrt(move2);
	moveX2 = moveX2 / (mover2 * 100);
	moveY2 = moveY2 / (mover2 * 100);
	moveZ2 = moveZ2 / (mover2 * 100);


	if (GetAsyncKeyState(VK_UP))
	{
		if (keyState == 0 || keyState == 1 || keyState == 3)
		{
			eyez = eyez + moveZ;
			eyex = eyex + moveX;
			atz = atz + moveZ;
			atx = atx + moveX;
		}
		if (keyState == 2)
		{
			aty2 = aty2 + 0.05f;
		}
	}

	if (GetAsyncKeyState(VK_DOWN))
	{
		if (keyState == 0 || keyState == 1 || keyState == 3)
		{
			eyez = eyez - moveZ;
			eyex = eyex - moveX;
			atz = atz - moveZ;
			atx = atx - moveX;
		}
		if (keyState == 2)
		{
			aty2 = aty2 - 0.05f;
		}
	}

	if (GetAsyncKeyState(VK_RIGHT))
	{
		if (keyState == 0 || keyState == 1 || keyState == 3)
		{
			if ((atz >= eyez) && (atx <= eyex))
			{
				atx += 0.1f;
				atz += 0.1f;
			}
			if ((atz <= eyez) && (atx <= eyex))
			{
				atx -= 0.1f;
				atz += 0.1f;
			}
			if ((atz <= eyez) && (atx >= eyex))
			{
				atx -= 0.1f;
				atz -= 0.1f;
			}
			if ((atz >= eyez) && (atx >= eyex))
			{
				atx += 0.1f;
				atz -= 0.1f;
			}
		}
		if (keyState == 2)
		{
			if ((atz2 >= eyez2) && (atx2 <= eyex2))
			{
				atx2 += 0.1f;
				atz2 += 0.1f;
			}
			if ((atz <= eyez) && (atx2 <= eyex))
			{
				atx2 -= 0.1f;
				atz2 += 0.1f;
			}
			if ((atz2 <= eyez) && (atx2 >= eyex))
			{
				atx2 -= 0.1f;
				atz2 -= 0.1f;
			}
			if ((atz2 >= eyez) && (atx2 >= eyex))
			{
				atx2 += 0.1f;
				atz2 -= 0.1f;
			}
		}
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		if (keyState == 0 || keyState == 1 || keyState == 3)
		{
			if ((atz >= eyez) && (atx <= eyex))
			{
				atx -= 0.1f;
				atz -= 0.1f;
			}
			if ((atz <= eyez) && (atx <= eyex))
			{
				atx += 0.1f;
				atz -= 0.1f;
			}
			if ((atz <= eyez) && (atx >= eyex))
			{
				atx += 0.1f;
				atz += 0.1f;
			}
			if ((atz >= eyez) && (atx >= eyex))
			{
				atx -= 0.1f;
				atz += 0.1f;
			}
		}
		if (keyState == 2)
		{
			if ((atz2 >= eyez) && (atx2 <= eyex))
			{
				atx2 -= 0.1f;
				atz2 -= 0.1f;
			}
			if ((atz2 <= eyez) && (atx2 <= eyex))
			{
				atx2 += 0.1f;
				atz2 -= 0.1f;
			}
			if ((atz2 <= eyez) && (atx2 >= eyex))
			{
				atx2 += 0.1f;
				atz2 += 0.1f;
			}
			if ((atz2 >= eyez) && (atx2 >= eyex))
			{
				atx2 -= 0.1f;
				atz2 += 0.1f;
			}
		}
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		if (keyState == 2)
		{
			eyex2 = eyex2 + moveX2;
			atx2 = atx2 + moveX2;
			eyey2 = eyey2 + moveY2;
			aty2 = aty2 + moveY2;
			eyez2 = eyez2 + moveZ2;
			atz2 = atz2 + moveZ2;
		}
	}

	if (GetAsyncKeyState(VK_NUMPAD0))
	{
		keyState = 0;
		if (eyey < -0.1f)
		{
			eyey = eyey + 1.5f;
			eyex = eyex - (moveX / 10);
			eyez = eyez - (moveZ / 10);
			aty = aty + 0.05f;
			atx = atx - (moveX / 10);
			atz = atz - (moveZ / 10);
		}
	}
	if (GetAsyncKeyState(VK_NUMPAD1))
	{
		keyState = 1;
		if (eyey > -0.1f)
		{
			eyey = eyey - 1.5f;
			eyex = eyex + (moveX / 10);
			eyez = eyez + (moveZ / 10);
			aty = aty - 0.05f;
			atx = atx + (moveX / 10);
			atz = atz + (moveZ / 10);
		}
	}
	if (GetAsyncKeyState(VK_NUMPAD2))
	{
		keyState = 2;
	}
	if (GetAsyncKeyState(VK_NUMPAD3))
	{
		keyState = 3;
		eyex2 = eyex;
		eyey2 = eyey;
		eyez2 = eyez;
	}

	if (keyState == 3)
	{
		XMStoreFloat4x4(&_world, XMMatrixTranslation(eyex, eyey - 1.5f, eyez));
	}
	if (keyState == 0)
	{
		XMStoreFloat4x4(&_world, XMMatrixTranslation(eyex, eyey - 1.5f, eyez));
	}
	if (keyState == 1)
	{
		XMStoreFloat4x4(&_world, XMMatrixTranslation(eyex - (moveX * 200), eyey, eyez - (moveZ * 200)));
	}
	XMStoreFloat4x4(&_world2, XMMatrixScaling(10.0f, 1.0f, 10.0f));

	/*CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV);*/

	CreateDDSTextureFromFile(_pd3dDevice, L"asphalt.dds", nullptr, &_pTextureRV);

	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);

	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);
    
	//--
	//light direction
	lightDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);

	//diffuse material properties
	diffuseMaterial = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	//diffuse light colour
	diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//ambient material
	ambientMaterial = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0);

	//ambient light
	ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0);
	
	//specular light
	specularLight = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	//specular material
	specularMaterial = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	//specular power
	specularPower = FLOAT(5.0f);
	//--

	
	
	//
    // Update variables
    //
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
	//--
	cb.DiffuseMtrl = diffuseMaterial;
	cb.DiffuseLight = diffuseLight;
	cb.LightVecW = lightDirection;

	cb.AmbientLight = ambientLight;
	cb.AmbientMaterial = ambientMaterial;

	cb.SpecularLight = specularLight;
	cb.SpecularMtrl = specularMaterial;
	cb.SpecularPower = specularPower;
	cb.EyePosW = XMFLOAT3(0.0f, 0.0f, -6.0f);
	//--

	//copies the constant buffer to shaders
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

	_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //
    // Renders a triangle
    //
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);        

	//--
	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferTri, &stride, &offset);

	_pImmediateContext->IASetIndexBuffer(_pIndexBufferTri, DXGI_FORMAT_R16_UINT, 0);

	//object 2
	world = XMLoadFloat4x4(&_world2);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(18, 0, 0);
    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
	//--
}
