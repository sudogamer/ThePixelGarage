#include "SudoGraphics.h"
#include "dxerr.h"
namespace wrl = Microsoft::WRL;

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) SudoGraphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw SudoGraphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) SudoGraphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) SudoGraphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw SudoGraphics::InfoException( __LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) SudoGraphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) SudoGraphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif


SudoGraphics::SudoGraphics( HWND hWnd )
{
	this->hWnd = hWnd;

	InitDirect3D12();

	// Do the initial rendertarget view and depthrendertarget view setup.
	//Setup screen viewport and scissors
	OnResize();
}
 
void SudoGraphics::EndFrame()
{
	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	// swap the back and front buffers
	if( FAILED( hr = mSwapChain->Present( 1u,0u ) ) )
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(md3dDevice->GetDeviceRemovedReason() );
		} 
		else
		{
			throw GFX_EXCEPT( hr );
		}
	}
	else {
		mCurrentBackBufferIndex = (mCurrentBackBufferIndex + 1) % SWAPCHAINBUFFERCOUNT;
		// Wait until frame commands are complete.  This waiting is inefficient and is
		// done for simplicity.  Later we will show how to organize our rendering code
		// so we do not have to wait per frame.
		FlushCommandQueue();
	}
}

void SudoGraphics::ClearBuffer( float red,float green,float blue ) noexcept
{
	/*const float color[] = { red,green,blue,1.0f };
	mCommandListGraphics->ClearRenderTargetView( pTarget.Get(),color );*/
}

void SudoGraphics::Draw()
{
	HRESULT hr;
	// Reuse the memory associated with command recording.
// We can only reset when the associated command lists have finished execution on the GPU.
	GFX_THROW_INFO(mCommandAllocator->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	GFX_THROW_INFO(mCommandListGraphics->Reset(mCommandAllocator.Get(), nullptr));

	// Indicate a state transition on the resource usage.
	ID3D12Resource* currentRenderTarget = GetCurrentBackBuffer();
	D3D12_RESOURCE_BARRIER resourceBarrierBefore = CD3DX12_RESOURCE_BARRIER::Transition(currentRenderTarget,
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	mCommandListGraphics->ResourceBarrier(1, &resourceBarrierBefore);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	mCommandListGraphics->RSSetViewports(1, &mScreenViewport);
	mCommandListGraphics->RSSetScissorRects(1, &mScissorRect);

	// Clear the back buffer and depth buffer.
	mCommandListGraphics->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	mCommandListGraphics->ClearDepthStencilView(GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDescHandle = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilDescHandle = GetDepthStencilView();

	mCommandListGraphics->OMSetRenderTargets(1, &renderTargetDescHandle, true, &depthStencilDescHandle);

	D3D12_RESOURCE_BARRIER resourceBarrierAfter = CD3DX12_RESOURCE_BARRIER::Transition(currentRenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// Indicate a state transition on the resource usage.
	mCommandListGraphics->ResourceBarrier(1, &resourceBarrierAfter);

	// Done recording commands.
	GFX_THROW_INFO(mCommandListGraphics->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandListGraphics.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

// SudoGraphics exception stuff
SudoGraphics::HrException::HrException( int line,const char * file,HRESULT hr,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file ),
	hr( hr )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}

const char* SudoGraphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if( !info.empty() )
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* SudoGraphics::HrException::GetType() const noexcept
{
	return "SudoGamer SudoGraphics Exception";
}

HRESULT SudoGraphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string SudoGraphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString( hr );
}

std::string SudoGraphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription( hr,buf,sizeof( buf ) );
	return buf;
}

std::string SudoGraphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

const char* SudoGraphics::DeviceRemovedException::GetType() const noexcept
{
	return "SudoGamer SudoGraphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
SudoGraphics::InfoException::InfoException( int line,const char * file,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}

const char* SudoGraphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* SudoGraphics::InfoException::GetType() const noexcept
{
	return "SudoGamer SudoGraphics Info Exception";
}

std::string SudoGraphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}

bool SudoGraphics::InitDirect3D12()
{
	HRESULT hr;
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		GFX_THROW_INFO(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
	GFX_THROW_INFO(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&md3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		GFX_THROW_INFO(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		GFX_THROW_INFO(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice)));
	}

	GFX_THROW_INFO(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	mRTVDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDSVDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCBVDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mRenderTargetFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	GFX_THROW_INFO(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	return true;
}

void SudoGraphics::CreateRtvAndDsvDescriptorHeaps()
{
	HRESULT hr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SWAPCHAINBUFFERCOUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	GFX_THROW_INFO(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRenderTargetViewHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	GFX_THROW_INFO(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDepthStencilTargetViewHeap.GetAddressOf())));
}
void SudoGraphics::OnResize()
{
	HRESULT hr;
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mCommandAllocator);

	// Flush before changing any resources.
	FlushCommandQueue();

	GFX_THROW_INFO(mCommandListGraphics->Reset(mCommandAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SWAPCHAINBUFFERCOUNT; ++i)
		mRenderTargets[i].Reset();
	mDepthStencilTarget.Reset();

	// Resize the swap chain.
	GFX_THROW_INFO(mSwapChain->ResizeBuffers(
		SWAPCHAINBUFFERCOUNT,
		mScreenWidth, mScreenHeight,
		mRenderTargetFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrentBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SWAPCHAINBUFFERCOUNT; i++)
	{
		GFX_THROW_INFO(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));
		md3dDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRTVDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mScreenWidth;
	depthStencilDesc.Height = mScreenHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilTargetFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	
	auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	GFX_THROW_INFO(md3dDevice->CreateCommittedResource(
		&heapType,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilTarget.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilTargetFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mDepthStencilTarget.Get(), &dsvDesc, GetDepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilTarget.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	mCommandListGraphics->ResourceBarrier(1, &resourceBarrier);

	// Execute the resize commands.
	GFX_THROW_INFO(mCommandListGraphics->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandListGraphics.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mScreenWidth);
	mScreenViewport.Height = static_cast<float>(mScreenHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mScreenWidth, mScreenHeight };
}
void SudoGraphics::CreateCommandObjects()
{
	HRESULT hr;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	GFX_THROW_INFO(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	GFX_THROW_INFO(md3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mCommandAllocator.GetAddressOf())));

	GFX_THROW_INFO(md3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCommandAllocator.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(mCommandListGraphics.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	mCommandListGraphics->Close();
}

void SudoGraphics::CreateSwapChain()
{
	HRESULT hr;
	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mScreenWidth;
	sd.BufferDesc.Height = mScreenHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mRenderTargetFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SWAPCHAINBUFFERCOUNT;
	sd.OutputWindow = hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	GFX_THROW_INFO(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));
}

void SudoGraphics::FlushCommandQueue()
{
	HRESULT hr;
	// Advance the fence value to mark commands up to this fence point.
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	GFX_THROW_INFO(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		// Fire event when GPU hits current fence.  
		GFX_THROW_INFO(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void SudoGraphics::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugStringW(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void SudoGraphics::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugStringW(text.c_str());

		LogOutputDisplayModes(output, mRenderTargetFormat);

		ReleaseCom(output);

		++i;
	}
}

void SudoGraphics::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugStringW(text.c_str());
	}
}
