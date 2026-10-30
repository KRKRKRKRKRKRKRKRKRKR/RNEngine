//どのファイルでも共通で使うクラス
#pragma once								//二重読み込み防止

#include <windows.h>
#include <d3d12.h>							//Dx12本体
#include <dxgi1_6.h>						//Factory,SwapChain
#include <d3dcompiler.h>					//シェーダーのコンパイル
#include <wrl.h>							//ComPtr
#include "Core/Log.h"						//ログ,CHECK_HR

#pragma comment(lib, "d3d12.lib")			//Dx12本体
#pragma comment(lib, "dxgi.lib")			//Factory,SwapChain
#pragma comment(lib, "d3dcompiler.lib")		//シェーダーのコンパイル
#pragma comment(lib, "dxguid.lib")	
using Microsoft::WRL::ComPtr;

//HRESULTが失敗なら停止
inline void CheckHR(HRESULT hr)
{
	if (FAILED(hr))
	{
		MessageBoxA(nullptr, "HRESULT failed", "Error", MB_OK);
		__debugbreak();
	}
}