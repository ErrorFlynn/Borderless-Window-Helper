#include "util.h"
#include <sstream> // to_hex_string()

wstring GetAppFolder()
{
	wstring progdirw;
	progdirw.assign(4096, '\0');
	GetModuleFileNameW(NULL, &progdirw.front(), 4096);
	return progdirw.substr(0, progdirw.rfind(L'\\'));
}


std::filesystem::path AppPath()
{
	wstring progdirw;
	progdirw.resize(4096);
	DWORD len = GetModuleFileNameW(NULL, &progdirw.front(), 4096);
	progdirw.resize(len);
	return progdirw;
}


wstring MakeTempFolder(wstring folder)
{
	wstring temp_path(1234, '\0');
	GetTempPathW(temp_path.size(), &temp_path.front());
	temp_path.resize(temp_path.find(L'\0'));
	wstring tp(temp_path+folder);
	if(!std::filesystem::exists(tp))
	{
		if(CreateDirectoryW(tp.data(), NULL))
			temp_path = tp + L'\\';
	}
	else temp_path = tp + L'\\';
	return temp_path;
}


string to_hex_string(HWND i)
{
	stringstream ss;
	ss << hex << i;
	return "0x" + ss.str();
}


