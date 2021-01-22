#include "inifile.h"
#include <fstream>
#include <shlobj.h>

using namespace std;

std::filesystem::path IniFile::save_fname;

IniFile::IniFile(const std::filesystem::path &fname)
: fname(fname)
{
	static bool first = true;

	if(first)
	{
		first = false;
		std::filesystem::path appdata = GetSysFolderLocation(CSIDL_APPDATA);
		if(!appdata.empty())
			save_fname = appdata / fname.filename();
	}

	LoadData();
}

void IniFile::LoadData()
{
	bool b = std::filesystem::exists(save_fname);
	if(!b && !std::filesystem::exists(fname))
		return;

	ifstream file(b ? save_fname.c_str() : fname.c_str(), ios::ate);
	if(!file.is_open()) return;
	size_t size = (size_t)file.tellg();
	if(!size) return;
	stringstream data;
	string data_temp(size, '\0');
	file.seekg(0);
	file.read(&data_temp.front(), size);
	data << data_temp;
	data_temp.clear();

	string line, section_name, entry_name, entry_data;
	size_t section_index;

	while(!data.eof())
	{
		getline(data, line);
		if(line.size() < 3 || line[0] != '[') continue;
		section_name = line.substr(1, line.find(']', 1)-1);

		bool found = false;
		for(section_index = 0; section_index<sections.size(); section_index++)
			if(sections[section_index].name() == section_name) { found = true; break; }
		if(!found) 
		{
			sections.emplace_back(section_name);
			section_index = sections.size() - 1;
		}

		while(!data.eof() && data.peek() != '[')
		{
			getline(data, line);
			size_t pos = line.find('=');
			if(pos == string::npos) continue;
			entry_name = line.substr(0, pos);
			entry_name = entry_name.substr(0, entry_name.find(' '));
			size_t p = line.find_first_not_of(' ', pos+1);
			entry_data = p == string::npos ? "" : line.substr(p);
			sections[section_index].add_entry(entry_name, entry_data);
		}
	}
}

void IniFile::SaveData()
{
	if(sections.empty() || sections[0].entries.empty()) return;
	bool b = std::filesystem::exists(save_fname);
	ofstream file(b ? save_fname.c_str() : fname.c_str(), ios::trunc);
	if(!file.is_open())
	{
		file.open(save_fname.c_str(), ios::trunc);
		if(!file.is_open()) return;
	}
	if(sort_sections) sort(sections.begin(), sections.end(), [](section a, section b) {
		return lexicographical_compare(a.name().begin(), a.name().end(), b.name().begin(), b.name().end());
	});

	for(auto &section : sections)
	{
		file << '[' << section.name() << "]\n";
		if(sort_entries) sort(section.entries.begin(), section.entries.end(), [](section::entry a, section::entry b) {
			return lexicographical_compare(a.name().begin(), a.name().end(), b.name().begin(), b.name().end());
		});

		for(auto &entry : section.entries)
			file << entry.name() << (nospaces ? "=" : " = ") << entry.data() << '\n';

		if(section.name() != sections.back().name()) file << '\n';
	}
}

void IniFile::section::add_entry(const string &name, const string &data)
{
	bool hasentry = false;
	size_t n;

	for(n=0; n<entries.size(); n++)
		if(entries[n].name() == name) { hasentry = true; break; }

	if(hasentry) entries[n].setdata(data);
	else entries.emplace_back(name, data);
}

void IniFile::WriteInt(const string &section_name, const string& entry_name, int value)
{
	bool hassection = false;
	size_t n = 0;

	for(; n<sections.size(); n++)
		if(sections[n].name() == section_name) { hassection = true; break; }

	if(hassection) sections[n].add_entry(entry_name, to_string(value));
	else 
	{
		sections.emplace_back(section_name);
		sections.back().add_entry(entry_name, to_string(value));
	}
}

void IniFile::WriteUInt(const string &section_name, const string& entry_name, unsigned value)
{
	bool hassection = false;
	size_t n = 0;

	for(; n<sections.size(); n++)
		if(sections[n].name() == section_name) { hassection = true; break; }

	if(hassection) sections[n].add_entry(entry_name, to_string(value));
	else
	{
		sections.emplace_back(section_name);
		sections.back().add_entry(entry_name, to_string(value));
	}
}

void IniFile::WriteString(const string &section_name, const string& entry_name, const string &value)
{
	bool hassection = false;
	size_t n = 0;

	for(; n<sections.size(); n++)
		if(sections[n].name() == section_name) { hassection = true; break; }

	if(hassection) sections[n].add_entry(entry_name, value);
	else 
	{
		sections.emplace_back(section_name);
		sections.back().add_entry(entry_name, value);
	}
}

int IniFile::ReadInt(const string &section_name, const string &entry_name, int default_value)
{
	for(auto &section : sections)
		if(section.name() == section_name)
			for(auto &entry : section.entries)
				if(entry.name() == entry_name)
					return stoi(entry.data());

	return default_value;
}

unsigned IniFile::ReadUInt(const string &section_name, const string &entry_name, unsigned default_value)
{
	for(auto &section : sections)
		if(section.name() == section_name)
			for(auto &entry : section.entries)
				if(entry.name() == entry_name)
					return stoul(entry.data());

	return default_value;
}

string IniFile::ReadString(const string &section_name, const string &entry_name, const string &default_value)
{
	for(auto &section : sections)
		if(section.name() == section_name)
			for(auto &entry : section.entries)
				if(entry.name() == entry_name)
					return entry.data();

	return default_value;
}