#pragma once
#include <Windows.h>
#include <string>
#include <chrono>

#pragma warning( disable : 4800 4267 4996)

#ifndef FileExist
#define FileExist(x) (GetFileAttributesA(x) != -1)
#endif
#define FileExistW(x) (GetFileAttributesW(x) != -1)

using namespace std;


class filepath
{
private:
	wstring pathw_, dirw_, namew_, extw_, fullnamew_;
	string path_, dir_, name_, ext_, fullname_;

public:
	filepath() {}
	filepath(string);
	filepath(wstring);
	string path() { return path_; }
	wstring pathw() { return pathw_; }
	string dir() { return dir_; }
	wstring dirw() { return dirw_; }
	string name() { return name_; }
	wstring namew() { return namew_; }
	string ext() { return ext_; }
	wstring extw() { return extw_; }
	string fullname() { return fullname_; }
	wstring fullnamew() { return fullnamew_; }
};


// UTF8 conversion of wide character string (std::wstring) to multibyte string (std::string)
static void wctomb(const wstring &wcstr, string &mbstr, unsigned cp = CP_UTF8)
{
	if(wcstr.empty()) return;
	int len = WideCharToMultiByte(cp, NULL, wcstr.data(), -1, nullptr, 0, NULL, NULL);
	mbstr.assign(len-1, '\0');
	WideCharToMultiByte(cp, NULL, wcstr.data(), -1, &mbstr.front(), len-1, NULL, NULL);
}

// UTF8 conversion of multibyte string (std::string) to wide character string (std::wstring)
static void mbtowc(const string &mbstr, wstring &wcstr, unsigned cp = CP_UTF8)
{
	if(mbstr.empty()) return;
	int len = MultiByteToWideChar(cp, NULL, mbstr.data(), -1, nullptr, 0);
	wcstr.assign(len-1, '\0');
	MultiByteToWideChar(cp, NULL, mbstr.data(), -1, &wcstr.front(), len-1);
}

static void strlower(string &s) { for(auto &c : s) c = tolower(c); }
static void strlower(wstring &s) { for(auto &c : s) c = tolower(c); }

LONGLONG GetFileSize(LPCWSTR);
string GetLastErrorStr();
wstring GetLastErrorStrW();
string to_hex_string(unsigned);
// NO trailing backslash
wstring GetAppFolder();
filepath AppPath();
wstring MakeTempFolder(wstring);


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
