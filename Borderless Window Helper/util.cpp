#include "util.h"
#include <psapi.h>
#include <sstream> // to_hex_string()
#include <shlobj.h>

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

std::filesystem::path GetModuleFileNameExPath(HANDLE hProcess)
{
  std::wstring filename;
  filename.resize(MAX_PATH);
  boolean truncated;
  do {
    DWORD nRet = ::GetModuleFileNameExW(hProcess, NULL, &filename[0], filename.size());
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


HRESULT createShortcut(const std::wstring& linkFileName, const std::filesystem::path& targetPath, const std::wstring& arguments,
const std::wstring& description) {
	HRESULT hres;
	IShellLink *psl;
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if(SUCCEEDED(hres))
	{
		IPersistFile *ppf;
		psl->SetPath(targetPath.wstring().c_str());
		psl->SetArguments(arguments.c_str());
		psl->SetDescription(description.c_str());
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if(SUCCEEDED(hres))
		{
			hres = ppf->Save(linkFileName.c_str(), TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	return hres;
}

wstring GetSysFolderLocation(int csidl)
{
	LPITEMIDLIST pidl;
	if(SHGetFolderLocation(NULL, csidl, NULL, 0, &pidl) == S_OK)
	{
		WCHAR path[MAX_PATH];
		BOOL ret = SHGetPathFromIDListW(pidl, path);
		LPMALLOC pMalloc;
		if(SHGetMalloc(&pMalloc) == S_OK)
		{
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
		if(ret) return path;
	}
	return L"";
}
