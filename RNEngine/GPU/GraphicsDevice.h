#pragma once
#include "Common.h"

//GPUに命令を送って待つ
class GraphicsDevice
{
public:
	~GraphicsDevice();

	void Init();
	void ResetCommandList(ID3D12PipelineState* pso);	//記録開始
	void ExecuteCommandList();							//記録終了+GPUへ送信
	void WaitForGPU();									//GPUの完了を待つ

	ID3D12Device* GetDevice() const { return m_device.Get(); }
	IDXGIFactory4* GetFactory() const { return m_factory.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

private:
	ComPtr<IDXGIFactory4> m_factory;					//DXGI工場
	ComPtr<ID3D12Device> m_device;						//GPUの窓口
	ComPtr<ID3D12CommandQueue> m_commandQueue;			//命令の投入口
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;	//命令用メモリ
	ComPtr<ID3D12GraphicsCommandList> m_commandList;	//命令を書く道具
	ComPtr<ID3D12Fence> m_fence;						//GPU進捗カウンタ
	UINT64 m_fenceValue = 0;							//次に待つ値
	HANDLE m_fenceEvent = nullptr;						//待機用イベント
};