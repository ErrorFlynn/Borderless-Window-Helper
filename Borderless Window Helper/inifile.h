#include "util.h"
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

class IniFile
{
	wstring fname;
	static wstring safe_fname;
	bool sort_sections, sort_entries, nospaces;

	void LoadData();
	void SaveData();

	class section
	{
		string _name;

		class entry
		{
			string _name;
			string _data;

		public:
			entry(const string &name, const string &data) : _name(name), _data(data) {};
			const string& name() { return _name; }
			const string& data() { return _data; }
			void setdata(const string& d) { _data = d; }
		};
		
		vector<entry> entries;

	public:
		friend class IniFile;
		section(const string &name) : _name(name) {};
		const string& name() { return _name; }
		void add_entry(const string&, const string&);
	};
	
	vector<section> sections;

public:

	IniFile(const wstring &fname);
	~IniFile() { SaveData(); }

	void Clear() { sections.clear(); }
	void Refresh() { Clear(); LoadData(); }
	void SortSections() { sort_sections = true; }
	void SortEntries() { sort_entries = true; }
	void Sort() { SortSections(); SortEntries(); }
	void NoSpaces() { nospaces = true; }
	
	int ReadInt(const string &section_name, const string &entry_name, int default_value);
	unsigned ReadUInt(const string &section_name, const string &entry_name, unsigned default_value);
	string ReadString(const string &section_name, const string &entry_name, const string &default_value);
	void WriteInt(const string &section_name, const string &entry_name, int value);
	void WriteUInt(const string &section_name, const string &entry_name, unsigned value);
	void WriteString(const string &section_name, const string &entry_name, const string &value);
};

namespace ini_file
{
	static wstring GetSysFolderLocation(int csidl)
	{
		LPITEMIDLIST pidl;
		if(SHGetFolderLocation(NULL, csidl, NULL, NULL, &pidl) == S_OK)
		{
			WCHAR path[_MAX_PATH];
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
}