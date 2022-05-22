#include "inifile.h"
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

namespace fs = std::filesystem;

IniFile::IniFile(const fs::path &fname) : fname(fname)
{
    LoadData();
}

void IniFile::LoadData()
{
    if (!fs::exists(fname))
        return;

    ifstream file(fname);
    if (!file.is_open())
        return;

    string line, section_name, entry_name, entry_data;
    size_t section_index;

    while (!file.eof())
    {
        getline(file, line);
        if (line.size() < 3 || line[0] != '[')
            continue;
        section_name = line.substr(1, line.find(']', 1) - 1);

        bool found = false;
        for (const auto &section : sections)
        {
            if (section.name() == section_name)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sections.emplace_back(section_name);
        }

        while (!file.eof() && file.peek() != '[')
        {
            getline(file, line);
            size_t pos = line.find('=');
            if (pos == string::npos)
                continue;
            entry_name = line.substr(0, pos);
            entry_name = entry_name.substr(0, entry_name.find(' '));
            size_t p = line.find_first_not_of(' ', pos + 1);
            entry_data = p == string::npos ? "" : line.substr(p);
            sections.back().add_entry(entry_name, entry_data);
        }
    }
}

void IniFile::SaveData()
{
    auto sectionComparator = [](const section &a, const section &b) {
        return lexicographical_compare(a.name().begin(), a.name().end(), b.name().begin(), b.name().end());
    };

    if (sections.empty() || sections[0].entries.empty())
        return;
    ofstream file(fname, ios::trunc);
    if (!file.is_open())
        return;
    if (sort_sections)
        std::sort(sections.begin(), sections.end(), sectionComparator);

    for (auto &section : sections)
    {
        file << '[' << section.name() << "]\n";
        if (sort_entries)
            std::sort(sections.begin(), sections.end(), sectionComparator);

        for (auto &entry : section.entries)
            file << entry.name() << (nospaces ? "=" : " = ") << entry.data() << '\n';

        if (section.name() != sections.back().name())
            file << '\n';
    }
}

void IniFile::section::add_entry(const string &name, const string &data)
{
    auto it = std::find_if(entries.begin(), entries.end(), [&name](auto &entry) { return entry.name() == name; });

    if (it != entries.end())
        it->setdata(data);
    else
        entries.emplace_back(name, data);
}

void IniFile::WriteInt(const string &section_name, const string &entry_name, int value)
{
    auto it = std::find_if(sections.begin(), sections.end(),
                           [&section_name](auto &section) { return section.name() == section_name; });

    if (it != sections.end())
        it->add_entry(entry_name, to_string(value));
    else
    {
        sections.emplace_back(section_name);
        sections.back().add_entry(entry_name, to_string(value));
    }
}

void IniFile::WriteUInt(const string &section_name, const string &entry_name, unsigned value)
{
    auto it = std::find_if(sections.begin(), sections.end(),
                           [&section_name](auto &section) { return section.name() == section_name; });

    if (it != sections.end())
        it->add_entry(entry_name, to_string(value));
    else
    {
        sections.emplace_back(section_name);
        sections.back().add_entry(entry_name, to_string(value));
    }
}

void IniFile::WriteString(const string &section_name, const string &entry_name, const string &value)
{
    auto it = std::find_if(sections.begin(), sections.end(),
                           [&section_name](auto &section) { return section.name() == section_name; });

    if (it != sections.end())
        it->add_entry(entry_name, value);
    else
    {
        sections.emplace_back(section_name);
        sections.back().add_entry(entry_name, value);
    }
}

long long IniFile::ReadLongLong(const string &section_name, const string &entry_name, long long default_value)
{
    for (auto &section : sections)
        if (section.name() == section_name)
            for (auto &entry : section.entries)
                if (entry.name() == entry_name)
                    return stoll(entry.data(), nullptr, 0);

    return default_value;
}

int IniFile::ReadInt(const string &section_name, const string &entry_name, int default_value)
{
    for (auto &section : sections)
        if (section.name() == section_name)
            for (auto &entry : section.entries)
                if (entry.name() == entry_name)
                    return stoi(entry.data(), nullptr, 0);

    return default_value;
}

unsigned IniFile::ReadUInt(const string &section_name, const string &entry_name, unsigned default_value)
{
    for (auto &section : sections)
        if (section.name() == section_name)
            for (auto &entry : section.entries)
                if (entry.name() == entry_name)
                    return stoul(entry.data(), nullptr, 0);

    return default_value;
}

string IniFile::ReadString(const string &section_name, const string &entry_name, const string &default_value)
{
    for (auto &section : sections)
        if (section.name() == section_name)
            for (auto &entry : section.entries)
                if (entry.name() == entry_name)
                    return entry.data();

    return default_value;
}