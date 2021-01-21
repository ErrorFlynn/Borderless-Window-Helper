#include "util.h"
#include <string>
#include <vector>

class IniFile
{
	std::wstring fname;
	static std::wstring save_fname;
	bool sort_sections, sort_entries, nospaces;

	void LoadData();
	void SaveData();

	class section
	{
		std::string _name;

		class entry
		{
			std::string _name;
			std::string _data;

		public:
			entry(const std::string &name, const std::string &data) : _name(name), _data(data) {};
			const std::string& name() { return _name; }
			const std::string& data() { return _data; }
			void setdata(const std::string& d) { _data = d; }
		};
		
		std::vector<entry> entries;

	public:
		friend class IniFile;
		section(const std::string &name) : _name(name) {};
		const std::string& name() { return _name; }
		void add_entry(const std::string&, const std::string&);
	};
	
	std::vector<section> sections;

public:

	IniFile(const std::wstring &fname);
	~IniFile() { SaveData(); }

	void Clear() { sections.clear(); }
	void Refresh() { Clear(); LoadData(); }
	void SortSections() { sort_sections = true; }
	void SortEntries() { sort_entries = true; }
	void Sort() { SortSections(); SortEntries(); }
	void NoSpaces() { nospaces = true; }
	
	int ReadInt(const std::string &section_name, const std::string &entry_name, int default_value);
	unsigned ReadUInt(const std::string &section_name, const std::string &entry_name, unsigned default_value);
	std::string ReadString(const std::string &section_name, const std::string &entry_name, const std::string &default_value);
	void WriteInt(const std::string &section_name, const std::string &entry_name, int value);
	void WriteUInt(const std::string &section_name, const std::string &entry_name, unsigned value);
	void WriteString(const std::string &section_name, const std::string &entry_name, const std::string &value);
};

