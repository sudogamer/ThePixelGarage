#pragma once

#include "SudoWin.h"
#include "SudoException.h"
#include "..\D3DHeaders.h"
#include "DxgiInfoManager.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

//d3d12 pragmas
#pragma comment(lib,"D3D12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

class SudoGraphics
{
public:
	class Exception : public SudoException
	{
		using SudoException::SudoException;
	};
	class HrException : public Exception
	{
	public:
		HrException( int line,const char* file,HRESULT hr,std::vector<std::string> infoMsgs = {} ) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public Exception
	{
	public:
		InfoException( int line,const char* file,std::vector<std::string> infoMsgs ) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
public:
	SudoGraphics( HWND hWnd );
	SudoGraphics( const SudoGraphics& ) = delete;
	SudoGraphics& operator=( const SudoGraphics& ) = delete;
	~SudoGraphics() = default;
	void EndFrame();
	void ClearBuffer( float red,float green,float blue ) noexcept;
	void Draw();
private:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	//Window
	HWND hWnd = NULL;

	//D3D12 fields
	static const int SWAPCHAINBUFFERCOUNT = 2;
	UINT mRTVDescriptorSize = 0;
	UINT mDSVDescriptorSize = 0;
	UINT mCBVDescriptorSize = 0;
	int mScreenWidth = 800;
	int mScreenHeight = 600;
	
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilTargetFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//D3D12 com objects for rendering setup
	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
	
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandListGraphics;

	//Resources & Descriptor Heaps. RenderTaget and DepthStencilTargets
	Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[SWAPCHAINBUFFERCOUNT];
	int mCurrentBackBufferIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilTarget;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDepthStencilTargetViewHeap;
	
	//Viewports and Scissor objects
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

protected :
	bool InitDirect3D12();
	void CreateCommandObjects();
	void CreateSwapChain();
	void FlushCommandQueue();
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void CreateRtvAndDsvDescriptorHeaps();
	void OnResize();

	ID3D12Resource* GetCurrentBackBuffer()const
	{
		return mRenderTargets[mCurrentBackBufferIndex].Get();
	}
	
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrentBackBufferIndex,
			mRTVDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView()const
	{
		return mDepthStencilTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	}
};