#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <dinput.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"


using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 AmbientMaterial;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	FLOAT SpecularPower;
	XMFLOAT3 EyePosW;
	XMFLOAT3 LightVecW;
};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;

	ID3D11Buffer*           _pVertexBufferTri;
	ID3D11Buffer*           _pIndexBufferTri;

	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world;
	XMFLOAT4X4              _world2;
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;

	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;
	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;
	XMFLOAT4 specularLight;
	XMFLOAT4 specularMaterial;
	FLOAT specularPower;

	XMVECTOR Eye;
	float eyex;
	float eyey;
	float eyez;

	float eyex2;
	float eyey2;
	float eyez2;

	XMVECTOR At;
	float atx;
	float aty;
	float atz;

	float atx2;
	float aty2;
	float atz2;

	XMVECTOR Up;

	int keyState;
	bool shiftCamera;

	/*ID3D11Texture2D* texture;*/
	ID3D11ShaderResourceView * _pTextureRV = nullptr;
	ID3D11SamplerState * _pSamplerLinear = nullptr;


private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	//--
	HRESULT InitVertexBufferTri();
	HRESULT InitIndexBufferTri();

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;
	//--

	ID3D11RasterizerState* _wireFrame;

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

