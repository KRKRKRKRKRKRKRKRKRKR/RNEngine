#include "Core/Log.h"
#include <cstdio>
#include <cstdarg>
#include <share.h>											//_SH_DENYWR

static FILE* s_file = nullptr;								//log.txt(このcpp内だけ)
static HANDLE s_console = nullptr;							//コンソールの出力先

void Log::Init(const char* path)
{
	s_file = _fsopen(path, "w", _SH_DENYWR);				//毎回上書き,実行中も他から読める
#ifdef _DEBUG
	AllocConsole();											//コンソール窓を作る(Debugのみ)
	s_console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

void Log::Shutdown()
{
	if (s_file)
	{
		fclose(s_file);
		s_file = nullptr;
	}
	if (s_console)
	{
		FreeConsole();										//コンソール窓を閉じる
		s_console = nullptr;
	}
}

void Log::Info(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);									//可変長引数の取り出し開始
	Write(Level::Info, fmt, args);
	va_end(args);
}

void Log::Warn(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Write(Level::Warn, fmt, args);
	va_end(args);
}

void Log::Error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Write(Level::Error, fmt, args);
	va_end(args);
}

void Log::Write(Level level, const char* fmt, va_list args)
{
	static const char* names[] = { "INFO ", "WARN ", "ERROR" };
	char msg[1024];
	vsnprintf(msg, sizeof(msg), fmt, args);					//書式を展開
	char line[1100];
	snprintf(line, sizeof(line), "[%s] %s\n", names[(int)level], msg);

	wchar_t wline[1100];
	MultiByteToWideChar(CP_UTF8, 0, line, -1, wline, 1100);	//UTF-8→UTF-16(日本語対策)
	OutputDebugStringW(wline);								//VSの出力ウィンドウ
	if (s_console)
	{
		WriteConsoleW(s_console, wline, lstrlenW(wline), nullptr, nullptr);	//コンソール(UTF-16で直接)
	}

	if (s_file)
	{
		fputs(line, s_file);								//UTF-8のまま書く
		fflush(s_file);										//落ちても残るよう即書き込み
	}
}

void Log::FailHR(HRESULT hr, const char* expr, const char* file, int line)
{
	wchar_t wtext[256] = L"";
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, 0, wtext, 256, nullptr);				//hrを文章に変換
	char text[512] = "";
	WideCharToMultiByte(CP_UTF8, 0, wtext, -1, text, sizeof(text), nullptr, nullptr);	//UTF-16→UTF-8

	Error("%s(%d) %s\n  hr=0x%08X %s", file, line, expr, (unsigned)hr, text);
	MessageBoxW(nullptr, L"エラー。log.txtを確認", L"Error", MB_OK);
	__debugbreak();											//デバッガで止める
}
