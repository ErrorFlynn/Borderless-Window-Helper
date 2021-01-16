#pragma once
#include <Windows.h>
#include <string>
#include <chrono>
#include <filesystem>

#pragma warning( disable : 4800 4267 4996)

using namespace std;


static string strlower(string s) { for(auto &c : s) c = tolower(c); return s; }
static wstring strlower(wstring s) { for(auto &c : s) c = tolower(c); return s; }

LONGLONG GetFileSize(LPCWSTR);
string to_hex_string(HWND);
// NO trailing backslash
wstring GetAppFolder();
std::filesystem::path AppPath();
wstring MakeTempFolder(wstring);
template<typename T> bool FileExist(const T &fname) { return filesystem::exists(fname); }


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
