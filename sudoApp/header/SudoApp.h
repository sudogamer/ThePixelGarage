#pragma once
#include "factoryUtils/Window.h"
#include "factoryUtils/SudoTimer.h"
#include "..\sudogamerfactory\common\MathHelper.h"
#include "..\sudogamerfactory\common\UploadBuffer.h"
#include "..\sudogamerfactory\common\SudoGraphicsCore.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class SudoApp : public SudoGraphicsCore
{
public:
	SudoApp(unsigned int wWidth, unsigned int wHeight);
	SudoApp(const SudoApp& rhs) = delete;
	SudoApp& operator=(const SudoApp& rhs) = delete;
	~SudoApp() = default;
	// master frame / message loop
	int Go();
private:
	void DoFrame();
	void Update(const SudoTimer& gt) override;
	void Draw(const SudoTimer& gt) override;
	void OnResize() override;
private:
	Window wnd;
	
	//Performance stats
	SudoTimer timer;
	string fps;
	string frameTime;

	//To handle user input
	void HandleInputs();

	//HR
	HRESULT hr;

	//App member funcitons
	void BuildConstantBufferDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();
	float AspectRatio()const;
	//App members
	
	//Constant Buffer object
	std::unique_ptr<UploadBuffer<ObjectConstants>> constantBuffer = nullptr;

	//Constant buffer root signature 
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;

	XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;

	int wWidth, wHeight;
};