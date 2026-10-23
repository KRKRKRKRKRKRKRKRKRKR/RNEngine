#include "UI/ImGuiLayer.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

ImGuiLayer::~ImGuiLayer()
{
	if (m_initialized)
	{
		ImGui_ImplDX12_Shutdown();			//GPU側の後始末
		ImGui_ImplWin32_Shutdown();			//OS側の後始末
		ImGui::DestroyContext();
	}
}

void ImGuiLayer::Init(HWND hwnd, GraphicsDevice& gfx, DXGI_FORMAT rtvFormat, UINT frameCount)
{
	//SRVヒープ(シェーダーから見える)
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;			//フォント1枚分
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CheckHR(gfx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap)));

	//ImGui本体
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	//ドッキング有効

	ImGui_ImplWin32_Init(hwnd);				//入力はWin32から受け取る

	ImGui_ImplDX12_InitInfo info = {};
	info.Device = gfx.GetDevice();
	info.CommandQueue = gfx.GetCommandQueue();
	info.NumFramesInFlight = frameCount;
	info.RTVFormat = rtvFormat;				//SwapChainと一致
	info.DSVFormat = DXGI_FORMAT_UNKNOWN;	//深度なし
	info.SrvDescriptorHeap = m_srvHeap.Get();
	info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* i, D3D12_CPU_DESCRIPTOR_HANDLE* cpu, D3D12_GPU_DESCRIPTOR_HANDLE* gpu)
		{
			*cpu = i->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	//1個だけ使う
			*gpu = i->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		};
	info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {};
	ImGui_ImplDX12_Init(&info);

	m_initialized = true;
}

void ImGuiLayer::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);	//中央は透明(3D画面が見える)
}

void ImGuiLayer::Render(ID3D12GraphicsCommandList* cmd)
{
	ImGui::Render();						//描画データ確定
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	cmd->SetDescriptorHeaps(1, heaps);		//ImGuiが使うヒープ
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);
}