#include "main.h"

#include <Psapi.h>
#include <shlobj.h>

#include <map>
#include <nana/gui.hpp>
#include <nana/gui/notifier.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/listbox.hpp>

#include "inifile.h"
#include "nana_subclassing.h"

using namespace std;
using namespace nana;

namespace fs = std::filesystem;

fs::path inifile;
HWND hwnd;

void display_info(listbox &list1, listbox &list2, label &info);

struct processNameComp
{
    bool operator()(const std::string &s1, const std::string &s2) const
    {
        return _stricmp(s1.c_str(), s2.c_str()) < 0;
    }
};

map<string, monwin, processNameComp> monwins;

map<string, enumwin, processNameComp> windows;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
#ifdef _DEBUG
    AllocConsole();
    freopen("conout$", "w", stdout);
#endif

    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    LocalFree(argv);

    inifile = AppPath().parent_path() / L"bwh.ini";
    if (!fs::exists(inifile))
        inifile = GetSysFolderLocation(CSIDL_APPDATA) / L"Borderless-Window-Helper\\bwh.ini";

    CreateMutexW(NULL, FALSE, TITLEW);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxW(NULL, L"An instance of the program is already running! Press OK to exit.", TITLEW,
                    MB_ICONEXCLAMATION);
        return 0;
    }

    CoInitialize(NULL);
    LoadSettings();
    RunGUI(argc == 1);
    SaveSettings();
    CoUninitialize();
    return 0;
}

enum MENUIDS
{
    IDSHOW = 2000,
    IDAUTOSTART,
    IDSTARTMENU,
    IDEXIT
};

void RunGUI(bool show)
{
    form fm(API::make_center(645, 800), appear::decorate<appear::minimize, appear::sizable>());
    fm.bgcolor(colors::white);
    fm.caption(TITLE);
    fm.icon(paint::image(AppPath()));

    hwnd = (HWND)fm.native_handle();

    notifier ntfr(fm);
    ntfr.text(TITLE);
    ntfr.icon(AppPath().string());
    ntfr.events().dbl_click([&fm] {
        if (fm.visible())
            fm.hide();
        else
        {
            fm.show();
            SetForegroundWindow(hwnd);
        }
    });
    ntfr.events().mouse_up([&fm](const arg_mouse &e) {
        if (e.right_button)
        {
            static fs::path stlink = GetSysFolderLocation(CSIDL_APPDATA) /
                                     L"microsoft\\windows\\start "
                                     L"menu\\programs\\startup\\Borderless Window Helper.lnk";
            static fs::path menu_link = GetSysFolderLocation(CSIDL_APPDATA) /
                                        L"microsoft\\windows\\start menu\\programs\\Borderless Window Helper.lnk";
            HMENU hpop = CreatePopupMenu();
            POINT pt;
            GetCursorPos(&pt);
            int pos = 0;
            InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING, IDSHOW,
                        fm.visible() ? L"Hide interface" : L"Show interface");
            InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING | (fs::exists(stlink) ? MF_CHECKED : MF_UNCHECKED),
                        IDAUTOSTART, L"Start with Windows");
            InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING | (fs::exists(menu_link) ? MF_CHECKED : MF_UNCHECKED),
                        IDSTARTMENU, L"Start Menu");
            InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING, IDEXIT, L"Exit");
            SetForegroundWindow(hwnd);
            WORD cmd = TrackPopupMenu(hpop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y,
                                      0, hwnd, NULL);
            DestroyMenu(hpop);
            if (cmd == IDSHOW)
            {
                if (fm.visible())
                    fm.hide();
                else
                    fm.show();
            }
            else if (cmd == IDAUTOSTART)
            {
                if (fs::exists(stlink))
                    fs::remove(stlink);
                else
                    createShortcut(stlink, AppPath(), L"tray", L"");
            }
            else if (cmd == IDSTARTMENU)
            {
                if (fs::exists(menu_link))
                    fs::remove(menu_link);
                else
                    createShortcut(menu_link, AppPath(), L"tray", L"");
            }
            else if (cmd == IDEXIT)
                API::exit();
        }
    });

    subclass sc(fm);
    sc.make_before(WM_SYSCOMMAND, [&fm](UINT, WPARAM wparam, LPARAM, LRESULT *) {
        if (wparam == SC_CLOSE)
        {
            API::exit();
            return false;
        }
        if (wparam == SC_MINIMIZE)
        {
            fm.hide();
            return false;
        }
        return true;
    });

    sc.make_before(WM_KEYUP, [&fm](UINT, WPARAM wparam, LPARAM, LRESULT *) {
        if (wparam == VK_ESCAPE)
        {
            fm.hide();
            return false;
        }
        return true;
    });

    sc.make_before(WM_ENDSESSION, [](UINT, WPARAM wparam, LPARAM, LRESULT *) {
        if (wparam == TRUE)
            SaveSettings();
        return true;
    });

    auto plc = place(fm);
    plc.div(R"(<form margin=15 vertical arrange=86
                << monitored vertical arrange = [ 15, variable ] margin =
                [ 0, 7, 0, 0 ] >
                <running vertical arrange = [ 15, variable ] margin = [ 0, 0, 0, 8 ]> >
                <weight = 15> >)");

    const int padding = 15;

    label lb1(fm);
    lb1.fgcolor(color_rgb(0x555555));
    lb1.caption("Processes with windows being monitored:");

    auto itemComparator = [](const std::string &s1, nana::any *, const std::string &s2, nana::any *, bool reverse) {
        int res = _stricmp(s1.c_str(), s2.c_str());
        return reverse ? res > 0 : res < 0;
    };

    listbox list1(fm);
    list1.bgcolor(color_rgb(0xfbfbfb));
    list1.fgcolor(color_rgb(0x909090));
    list1.show_header(false);
    list1.append_header("Process Name");
    list1.scheme().item_selected = color_rgb(0xdcefe8);
    list1.scheme().item_highlighted = color_rgb(0xeaf0ef);
    list1.set_sort_compare(0, itemComparator);

    plc.field("monitored") << lb1 << list1;

    listbox list2(fm);
    list2.bgcolor(color_rgb(0xfbfbfb));
    list2.fgcolor(color_rgb(0x111111));
    list2.show_header(false);
    list2.enable_single(true, false);
    list2.append_header("Process Name");
    list2.scheme().item_selected = color_rgb(0xdcefe8);
    list2.scheme().item_highlighted = color_rgb(0xeaf0ef);
    list2.set_sort_compare(0, itemComparator);

    label lb2(fm);
    lb2.fgcolor(color_rgb(0x555555));
    lb2.caption("Processes currently running that have visible windows:");

    plc.field("running") << lb2 << list2;

    group grp(fm);
    grp.scheme().background = color_rgb(0xfbfbfb);
    grp.div("<form margin=5>");

    label lbinfo(grp);
    lbinfo.bgcolor(color_rgb(0xfbfbfb));
    lbinfo.fgcolor(color_rgb(0x555555));
    lbinfo.format(true);

    plc.field("form") << grp;
    grp["form"] << lbinfo;

    list2.events().dbl_click([&list1, &list2](const arg_mouse &arg) {
        auto lb = list2.at(0);
        auto selection = list2.selected();
        if (selection.size() == 1)
        {
            string seltext;
            seltext = lb.at(selection[0].item).text(0);
            for (const auto &[procname, monwin] : monwins)
                if (monwin.pname == seltext)
                    return;
            const auto &win = windows.at(seltext);
            LONG_PTR style = 0, exstyle = 0;
            if (!win.borderless)
            {
                style = GetWindowLongPtrW(win.hwnd, GWL_STYLE);
                exstyle = GetWindowLongPtrW(win.hwnd, GWL_EXSTYLE);
            }
            monwins[seltext] = {style, exstyle, false, seltext, win.modpath};
            list1.auto_draw(false);
            list1.at(0).push_back(seltext);
            list1.at(0).back().icon(paint::image(win.modpath));
            list1.auto_draw(true);
            list1.column_at(0).fit_content();
            mon_timer_fn();
            PostMessageW(win.hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            SetForegroundWindow(hwnd);
        }
    });

    list2.events().selected([&list1, &list2, &lbinfo](const arg_listbox &arg) {
        display_info(list1, list2, lbinfo);
        if (IsWindowVisible(hwnd) && !IsIconic(hwnd))
        {
            auto lb = list2.at(0);
            auto selection = list2.selected();
            if (selection.size() == 1)
            {
                string seltext = lb.at(selection[0].item).text(0);
                lbinfo.text_align(align::left);
                lbinfo.typeface(paint::font("Segoe UI", 10, detail::font_style(0)));
                auto it = windows.find(seltext);
                if (it != windows.end())
                {
                    const auto &win = it->second;
                    std::stringstream caption;
                    caption << R"(<font=" Segoe UI Semibold ">PID:</> )" << win.procid
                            << "  |  <font=\"Segoe UI Semibold\">Window handle:</> " << std::hex << win.hwnd
                            << "  |  Window ";
                    if (win.borderless)
                    {
                        auto it = monwins.find(seltext);
                        if (it != monwins.end() && it->second.style != 0)
                            caption << "border has been removed";
                        else
                            caption << "doesn't have a border";
                    }
                    else
                        caption << "has a border (monitoring will remove it)";
                    caption << "\n\n<font=\"Segoe UI Semibold\">Window title:</> " << escape(win.captionw);
                    lbinfo.caption(caption.str());
                }
                else
                {
                    lbinfo.caption("Unexpected error - can't find window!");
                }
            }
        }
    });

    list1.events().selected([&list1, &list2, &lbinfo](const arg_listbox &arg) {
        display_info(list1, list2, lbinfo);
        if (IsWindowVisible(hwnd) && !IsIconic(hwnd))
        {
            auto lb = list1.at(0);
            auto selection = list1.selected();
            if (selection.size() == 1)
            {
                string seltext = lb.at(selection[0].item).text(0);
                lbinfo.text_align(align::center, align_v::center);
                lbinfo.typeface(paint::font("Segoe UI", 10, detail::font_style(0, true)));
                std::stringstream caption;
                const auto &monwin = monwins.at(seltext);
                if (windows.find(seltext) == windows.end())
                    caption << "The process is not currently running."
                            << (monwin.style != 0 ? " Its window has a border, which will "
                                                    "be removed when it is run."
                                                  : "");
                if (caption.str().empty())
                {
                    caption << "The process is currently running. Its window is being "
                               "monitored and "
                               "will be minimized while not in focus.";
                    if (monwin.style != 0)
                        caption << " The border has been removed, and the window "
                                   "has been resized to fill the screen.";
                }
                string modpath = monwin.modpath.string();
                if (!modpath.empty())
                {
                    if (monwin.style != 0)
                        caption << "\n";
                    caption << "\n<color=0x117011>" << modpath << "</>";
                }
                lbinfo.caption(caption.str());
            }
        }
    });

    list1.events().key_release([&list1](const arg_keyboard &arg) {
        if (arg.key == 0x7f) // DEL ASCII code
        {
            auto lb = list1.at(0);
            auto selection = list1.selected();
            for (auto &selitem : selection)
            {
                string seltext = lb.at(selitem.item).text(0);
                auto it = windows.find(seltext);
                if (it != windows.end())
                {
                    const enumwin &win = it->second;
                    const auto &monwin = monwins.at(seltext);
                    if (monwin.style != 0)
                        SetWindowLongPtrW(win.hwnd, GWL_STYLE, monwin.style);
                    if (monwin.exstyle != 0)
                        SetWindowLongPtrW(win.hwnd, GWL_EXSTYLE, monwin.exstyle);
                }
                monwins.erase(seltext);
            }
            list1.erase(selection);
            list1.column_at(0).fit_content();
        }
    });

    list1.events().dbl_click([&list1](const arg_mouse &arg) {
        auto lb = list1.at(0);
        auto selection = list1.selected();
        if (!selection.empty())
        {
            string seltext = lb.at(selection[0].item).text(0);
            auto it = windows.find(seltext);
            if (it != windows.end())
            {
                const enumwin &win = it->second;
                const auto &monwin = monwins.at(seltext);
                if (monwin.style != 0)
                    SetWindowLongPtrW(win.hwnd, GWL_STYLE, monwin.style);
                if (monwin.exstyle != 0)
                    SetWindowLongPtrW(win.hwnd, GWL_EXSTYLE, monwin.exstyle);
            }
            monwins.erase(seltext);
            list1.erase(selection);
            list1.column_at(0).fit_content();
        }
    });

    timer enum_timer;
    enum_timer.interval(1000ms);
    enum_timer.elapse([&list1, &list2, &lbinfo] { enum_timer_fn(list1, list2); });
    enum_timer.start();
    enum_timer_fn(list1, list2);

    timer mon_timer;
    mon_timer.interval(1000ms);
    mon_timer.elapse(mon_timer_fn);
    mon_timer.start();

    list1.auto_draw(false);
    for (auto &[procname, monwin] : monwins)
    {
        list1.at(0).push_back(monwin.pname);
        list1.at(0).back().icon(paint::image(monwin.modpath));
    }
    list1.auto_draw(true);
    list1.column_at(0).fit_content();
    display_info(list1, list2, lbinfo);

    plc.collocate();
    if (show)
        fm.show();
    nana::exec();
    ntfr.close();
}

// monitors windows from list1 and minimizes them, removing borders if any
void mon_timer_fn()
{
    for (auto &[procname, monwin] : monwins)
    {
        auto it = windows.find(monwin.pname);
        if (it == windows.end())
            continue; // process not running
        enumwin &win = it->second;
        if (!win.borderless) // remove borders
        {
            MONITORINFO mi = {sizeof(mi)};
            GetMonitorInfoW(win.monitor, &mi);
            // TODO: store style and exstyle in win?
            SetWindowLongPtrW(win.hwnd, GWL_STYLE, (monwin.style & ~(WS_CAPTION | WS_THICKFRAME)) | WS_POPUP);
            SetWindowLongPtrW(win.hwnd, GWL_EXSTYLE, monwin.exstyle & ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE));
            SetWindowPos(win.hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            win.borderless = true;
        }
        HWND fghwnd = GetForegroundWindow();
        if (monwin.active)
        {
            if (fghwnd != win.hwnd && win.monitor == MonitorFromWindow(fghwnd, MONITOR_DEFAULTTOPRIMARY))
            {
                wstring fgclassname = GetClassNameString(fghwnd);
                if (fgclassname != L"TaskSwitcherWnd" && fgclassname != L"Ghost")
                {
                    monwin.active = false;
                    PostMessageW(win.hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
                }
            }
        }
        else if (fghwnd == win.hwnd)
            monwin.active = true;
    }
}

void display_info(listbox &list1, listbox &list2, label &info)
{
    auto selection1 = list1.selected(), selection2 = list2.selected();
    if (selection2.empty() && selection1.empty())
    {
        info.text_align(align::center, align_v::center);
        info.typeface(paint::font("Segoe UI", 10, detail::font_style(0, true)));
        info.caption("<color=0x666666>Select a process in the lists above to see some info about it "
                     "here.\n"
                     "Double-click a process in the right list to start monitoring its window.\n"
                     "Select one or more processes in the left list and press the \"Delete\" key to "
                     "remove "
                     "them.");
    }
}

// updates list2 from current data
void enum_timer_fn(listbox &list1, listbox &list2)
{
    enum_windows();
    auto lb1 = list1.at(0), lb2 = list2.at(0);
    list2.auto_draw(false);
    for (auto &item : lb2) // remove from list2 processes no longer running
    {
        if (windows.find(item.text(0)) == windows.end())
            list2.erase(item);
    }
    // add to list2 running processes that are not already in the list
    for (auto &[procname, win] : windows)
    {
        bool found = false;
        for (auto &item : lb2)
        {
            if (item.text(0) == win.pname)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            lb2.push_back(win.pname);
            lb2.back().icon(paint::image(win.modpath));
        }
    }

    for (auto &item : list2.at(0))
    {
        if (windows.at(item.text(0)).borderless)
            item.fgcolor(color_rgb(0x883311));
        else
            item.fgcolor(list2.fgcolor());
    }
    list2.auto_draw(true);
    list2.column_at(0).fit_content();
    if (IsWindowVisible(hwnd) && !IsIconic(hwnd))
    {
        list1.auto_draw(false);
        for (auto &item : list1.at(0))
        {
            if (windows.find(item.text(0)) == windows.end())
            {
                item.fgcolor(list1.fgcolor());
                item.bgcolor(list1.bgcolor());
            }
            else
            {
                item.fgcolor(color_rgb(0x663311));
                item.bgcolor(color_rgb(0xffffff));
            }
        }
        list1.auto_draw(true);
    }
}

void enum_windows()
{
    WNDENUMPROC enumfn = [](HWND hwnd, LPARAM lparam) -> BOOL {
        auto style = GetWindowLongPtrW(hwnd, GWL_STYLE);
        wstring caption = GetWindowTextString(hwnd);
        if ((style & WS_VISIBLE) && !caption.empty() && caption != L"Program Manager")
        {
            DWORD procid = 0;
            GetWindowThreadProcessId(hwnd, &procid);
            HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, procid);
            if (hproc != NULL)
            {
                auto procpath = GetModuleFileNameExPath(hproc);
                CloseHandle(hproc);
                if (procpath != AppPath())
                {
                    enumwin win;
                    win.procid = procid;
                    win.pname = procpath.filename().string();
                    win.hwnd = hwnd;
                    win.monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
                    win.captionw = caption;
                    if (!(style & (WS_CAPTION | WS_THICKFRAME)))
                        win.borderless = true;
                    string key = procpath.filename().string();
                    win.modpath = procpath;
                    windows[key] = win;
                }
            }
        }
        return TRUE;
    };

    windows.clear();
    EnumWindows(enumfn, 0);
}

void LoadSettings()
{
    IniFile ini(inifile);
    monwins.clear();
    int n = 0;
    LONG_PTR style = 0, exstyle = 0;
    string pname;
    do
    {
        pname = ini.ReadString(to_string(n), "p", "");
        if (!pname.empty())
        {
            style = ini.ReadInt(to_string(n), "s", 0);
            exstyle = ini.ReadInt(to_string(n), "exstyle", 0);
            if (pname.find('\\') != string::npos)
            {
                fs::path p(pname);
                string key = p.filename().string();
                if (fs::exists(p))
                    monwins[key] = {style, exstyle, false, p.filename().string(), p};
                else
                    monwins[key] = {style, exstyle, false, p.filename().string()};
            }
            else
                monwins[pname] = {style, exstyle, false, pname};
        }
        n++;
    } while (!pname.empty());
}

void SaveSettings()
{
    fs::remove(inifile);
    IniFile ini(inifile);
    int idx = 0;
    for (auto &[procname, monwin] : monwins)
    {
        string s = to_string(idx++);
        const fs::path &modpath = monwin.modpath;
        ini.WriteString(s, "p", modpath.empty() ? monwin.pname : modpath.string());
        ini.WriteInt(s, "s", monwin.style);
        ini.WriteInt(s, "exstyle", monwin.exstyle);
    }
}

std::ostream &operator<<(std::ostream &os, const std::wstring &s)
{
    os << charset(s).to_bytes(unicode::utf8);
    return os;
}
