#include "GPU/GraphicsDevice.h"

GraphicsDevice::~GraphicsDevice()
{
	if (m_fence)
	{
		WaitForGPU();						//GPU使用中に破棄しない
	}
	if (m_fenceEvent)
	{
		CloseHandle(m_fenceEvent);
	}
}

void GraphicsDevice::Init()
{
	//デバッグレイヤー
	UINT factoryFlags = 0;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		debug->EnableDebugLayer();			//エラーを出力に表示
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	//Factory
	CheckHR(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));

	//Device
	CheckHR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));	//nullptr=既定GPU

	//CommandQueue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	//汎用(描画可)
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CheckHR(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	//CommandAllocator
	CheckHR(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	//CommandList
	CheckHR(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	CheckHR(m_commandList->Close());		//作成直後は記録中のため

	//Fence
	CheckHR(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;

	//Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		CheckHR(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void GraphicsDevice::ResetCommandList(ID3D12PipelineState* pso)
{
	CheckHR(m_commandAllocator->Reset());	//前フレームの命令を破棄
	CheckHR(m_commandList->Reset(m_commandAllocator.Get(), pso));
}

void GraphicsDevice::ExecuteCommandList()
{
	CheckHR(m_commandList->Close());		//記録終了
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, lists);
}

void GraphicsDevice::WaitForGPU()
{
	const UINT64 value = m_fenceValue;
	CheckHR(m_commandQueue->Signal(m_fence.Get(), value));	//終わったらvalueにして
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < value)				//まだ終わってない
	{
		CheckHR(m_fence->SetEventOnCompletion(value, m_fenceEvent));	//終わったらイベント
		WaitForSingleObject(m_fenceEvent, INFINITE);					//鳴るまで待機
	}
}