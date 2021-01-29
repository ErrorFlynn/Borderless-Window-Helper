#include <filesystem>
#include <string>
#include <windows.h>

#pragma warning(disable : 4800 4267 4996)
#pragma comment(linker,                                                                                                \
                "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define TITLE "Borderless Window Helper 1.3"
#define TITLEW L"Borderless Window Helper 1.3"

struct monwin
{
    LONG_PTR style = 0;
    LONG_PTR exstyle = 0;
    bool active = false;
    std::string pname;             // process name as displayed in the list (not lowercased)
    std::filesystem::path modpath; // module path necessary for getting icon from module
};

struct enumwin
{
    DWORD procid = 0;
    std::string pname; // process name as displayed in the list (not lowercased)
    HWND hwnd = nullptr;
    HMONITOR monitor = nullptr;
    std::wstring captionw;
    bool borderless = false;
    std::filesystem::path modpath;
};

namespace nana
{
class listbox;
} // namespace nana

void LoadSettings();
void SaveSettings();
void RunGUI(bool show);
void enum_windows();
void enum_timer_fn(nana::listbox &list1, nana::listbox &list2);
void mon_timer_fn();

std::ostream &operator<<(std::ostream &os, const std::wstring &s);
