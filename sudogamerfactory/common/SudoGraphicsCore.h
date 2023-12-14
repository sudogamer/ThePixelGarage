#pragma once

#include "..\Utils\include\factoryUtils\SudoWin.h"
#include "..\Utils\include\factoryUtils\SudoException.h"
#include "..\Utils\include\D3DHeaders.h"
#include "..\Utils\include\factoryUtils\DxgiInfoManager.h"
#include "..\Utils\include\factoryUtils\SudoTimer.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

//d3d12 pragmas
#pragma comment(lib,"D3D12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")


// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) SudoGraphicsCore::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw SudoGraphicsCore::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) SudoGraphicsCore::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) SudoGraphicsCore::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw SudoGraphicsCore::InfoException( __LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) SudoGraphicsCore::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) SudoGraphicsCore::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

class SudoGraphicsCore
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
	SudoGraphicsCore() = default;
	SudoGraphicsCore( const SudoGraphicsCore& ) = delete;
	SudoGraphicsCore& operator=( const SudoGraphicsCore& ) = delete;
	virtual ~SudoGraphicsCore();

protected:
	virtual void OnResize();
	virtual void Update(const SudoTimer& gt);
	virtual void Draw(const SudoTimer& gt);

//Protected member fields
protected:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	//D3D12 fields
	static const int SWAPCHAINBUFFERCOUNT = 2;
	UINT mRTVDescriptorSize = 0;
	UINT mDSVDescriptorSize = 0;
	UINT mCBVDescriptorSize = 0;
	
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
	UINT64 mCurrentFence;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandListGraphics;

	//Resources & Descriptor Heaps. RenderTaget and DepthStencilTargets
	Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[SWAPCHAINBUFFERCOUNT];
	int mCurrentBackBufferIndex;

	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilTarget;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDepthStencilTargetViewHeap;
	
	//Viewports and Scissor objects
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled
	unsigned int wndWidth;
	unsigned int wndHeight;
	float clearColor[4];

	// Used to keep track of the “delta-time” and game time (§4.4).
	SudoTimer mTimer;

//Protected member funcitons
protected :
	void InitializeSudoGraphicsCore(HWND hWnd, unsigned int wndWidth, unsigned int wndHeight);
	bool InitDirect3D12();
	void CreateCommandObjects();
	void CreateSwapChain();
	void FlushCommandQueue();
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void ResetViewportAndScissorRectangle();
	void CreateRtvAndDsvDescriptorHeaps();
	void EndFrame();

	void SetClearColor(float color[]);
	void CalculateFrameStats(std::string* fps, std::string* frameTime);

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