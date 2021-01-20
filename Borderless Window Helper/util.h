#pragma once
#include <Windows.h>
#include <string>
#include <chrono>
#include <filesystem>

#pragma warning( disable : 4800 4267 4996)

using namespace std;


std::filesystem::path AppPath();
wstring MakeTempFolder(wstring);
HRESULT createShortcut(const std::filesystem::path& linkFileName, const std::filesystem::path& targetPath, const std::wstring& arguments,
const std::wstring& description);
std::filesystem::path GetModuleFileNameExPath(HANDLE hProcess);
std::filesystem::path GetSysFolderLocation(int csidl);
std::wstring GetClassNameString(HWND hWnd);
std::wstring GetWindowTextString(HWND hWnd);

class chronometer
{
	long long start, elapsed_;
	bool stopped;
	struct time { long long h, m, s; };

public:
	chronometer() { reset(); }
	void stop() { stopped = true; elapsed_ = (chrono::system_clock::now().time_since_epoch().count()-start)/10000; }
	void reset() { stopped = false; elapsed_ = 0; start = chrono::system_clock::now().time_since_epoch().count(); }

	long long elapsed_ms()
	{ 
		if(stopped) return elapsed_;
		else return (chrono::system_clock::now().time_since_epoch().count()-start)/10000;
	}

	long long elapsed_s() { return elapsed_ms()/1000; }

	time elapsed()
	{
		auto es = elapsed_s();
		return { es/3600, (es%3600)/60, (es%3600)%60 };
	}
};
