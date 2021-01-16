#include "util.h"
#include <sstream> // to_hex_string()

wstring GetAppFolder()
{
	auto appPath = AppPath();
	return appPath.parent_path().wstring();
}


std::filesystem::path AppPath()
{
  std::wstring filename;
  filename.resize(MAX_PATH);
  boolean truncated;
  do {
    DWORD nRet = ::GetModuleFileNameW(NULL, &filename[0], filename.size());
    if (nRet == 0)
      throw std::system_error(GetLastError(), std::system_category());
    truncated = nRet == filename.size();
    filename.resize(truncated ? filename.size() * 2 : nRet);
  } while (truncated);
  return filename;
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


