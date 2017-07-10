#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/label.hpp>

#include "util.h"
#include <string>
#include <map>
#include <unordered_map>

#pragma warning( disable : 4800 4267 4996)
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define TITLE "Borderless Window Helper 1.3"
#define TITLEW L"Borderless Window Helper 1.3"

using namespace std;
using namespace nana;

string last;
wstring inifile;
filepath self_path;
HWND hwnd;
paint::image iconapp;
listbox *list1(nullptr);

struct monwin
{
	int style = 0;
	bool active = false;
	string pname; // process name as displayed in the list (not lowercased)
	filepath modpath; // module path necessary for getting icon from module
	monwin() {}
	monwin(int st, bool a, string pn, filepath mpath = ""s) { style = st, active = a; pname = pn; modpath = mpath; }
};

map<string, monwin> monwins; // key is lowercase process name

struct enumwin
{
	DWORD procid = 0;
	string pname; // process name as displayed in the list (not lowercased)
	HWND hwnd = nullptr;
	HMONITOR monitor = nullptr;
	wstring captionw;
	bool borderless = false;
};

map<string, enumwin> windows; // key is lowercase process name

unordered_map<string, paint::image> icons;


void LoadSettings();
void SaveSettings();
void RunGUI(bool show);
void enum_windows();
void enum_timer_fn(listbox &list1, listbox &list2, label &info);
void mon_timer_fn();
bool am_i_already_running();
void sort_list(listbox&);