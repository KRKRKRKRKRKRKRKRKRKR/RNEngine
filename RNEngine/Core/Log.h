//ログ出力(出力ウィンドウとlog.txt)
#pragma once
#include <windows.h>

class Log
{
public:
	enum class Level { Info, Warn, Error };					//重要度

	static void Init(const char* path);						//ファイルを開く
	static void Shutdown();									//ファイルを閉じる
	static void Info(const char* fmt, ...);					//普通の報告
	static void Warn(const char* fmt, ...);					//動くが怪しい
	static void Error(const char* fmt, ...);				//失敗
	static void FailHR(HRESULT hr, const char* expr, const char* file, int line);	//CHECK_HR用

private:
	static void Write(Level level, const char* fmt, va_list args);	//共通の出力処理
};

//HRESULTが失敗なら場所を記録して停止
#define CHECK_HR(expr) \
	do { \
		HRESULT hr_ = (expr);								/*式は1回だけ実行*/ \
		if (FAILED(hr_)) Log::FailHR(hr_, #expr, __FILE__, __LINE__); \
	} while (0)
