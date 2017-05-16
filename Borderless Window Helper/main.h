#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/label.hpp>

#include "util.h"
#include <string>
#include <cassert>
#include <map>

#pragma warning( disable : 4800 4267 4996)
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define TITLE "Borderless Window Helper 1.0"
#define TITLEW L"Borderless Window Helper 1.0"

using namespace std;
using namespace nana;

string last;
wstring inifile;
filepath self_path;
HWND hwnd, balloon(0);
bool mintray(true);

struct monwin
{
	int style = 0;
	bool active = false;
	monwin() {}
	monwin(int st, bool a) {style = st, active = a; }
};

map<string, monwin> monwins;

struct enumwin
{
	DWORD procid = 0; wstring procnamew; HWND hwnd; wstring captionw; bool borderless = false;
};

map<string, enumwin> windows;


void LoadSettings();
void SaveSettings();
void RunGUI(bool show);
void enum_windows();
void enum_timer_fn(listbox &list1, listbox &list2, label &info);
void mon_timer_fn();
bool am_i_already_running();
void sort_list(listbox&);