#include "inifile.h"
#include "main.h"
#include "nana_subclassing.h"
#include <Psapi.h>
#include <nana/gui/notifier.hpp>
#include <map>
#include <unordered_map>

string last;
std::filesystem::path inifile;
std::filesystem::path self_path;
HWND hwnd;
paint::image iconapp;

map<string, monwin> monwins; // key is lowercase process name

map<string, enumwin> windows; // key is lowercase process name

unordered_map<string, paint::image> icons;


int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
#ifdef _DEBUG
	AllocConsole();
	freopen("conout$", "w", stdout);
#endif

	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	LocalFree(argv);

	self_path = AppPath();
	inifile = self_path.parent_path() / L"bwh.ini";

	CreateMutexW(NULL, FALSE, TITLEW);
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBoxW(NULL, L"An instance of the program is already running! Press OK to exit.", TITLEW, MB_ICONEXCLAMATION);
		return 0;
	}

	CoInitialize(NULL);
	LoadSettings();
	RunGUI(argc == 1);
	SaveSettings();
	CoUninitialize();
	return 0;
}


void RunGUI(bool show)
{
	form fm(API::make_center(645, 800), appear::decorate<appear::minimize, appear::sizable>());
	fm.bgcolor(colors::white);
	fm.caption(TITLE);
	fm.icon(paint::image(self_path));

	hwnd = (HWND)fm.native_handle();

	notifier ntfr(fm);
	ntfr.text(TITLE);
	ntfr.icon(self_path.string());
	ntfr.events().dbl_click([&fm] { if(fm.visible()) fm.hide(); else { fm.show(); SetForegroundWindow(hwnd); } });
	ntfr.events().mouse_up([&fm](const arg_mouse &e)
	{
		if(e.right_button)
		{
			static std::filesystem::path stlink = GetSysFolderLocation(CSIDL_APPDATA) /
			L"microsoft\\windows\\start menu\\programs\\startup\\Borderless Window Helper.lnk";
			HMENU hpop = CreatePopupMenu();
			POINT pt;
			GetCursorPos(&pt);
			int pos(0), ID(2000);
			InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING, ID, fm.visible() ? L"Hide interface" : L"Show interface");
			InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING | (std::filesystem::exists(stlink) ? MF_CHECKED : 0), ID+1, L"Start with Windows");
			InsertMenuW(hpop, pos++, MF_BYPOSITION | MF_STRING, ID+2, L"Exit");
			SetForegroundWindow(hwnd);
			WORD cmd = TrackPopupMenu(hpop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
			DestroyMenu(hpop);
			if(cmd == ID)
			{
				if(fm.visible()) fm.hide();
				else fm.show();
			}
			else if(cmd == ID+1)
			{
				if(std::filesystem::exists(stlink)) std::filesystem::remove(stlink);
				else
				{
					createShortcut(stlink, self_path, L"tray", L"This shortcut has been created by Borderless Window Helper, because you "
			"selected \"Start with Windows\" from the program's tray menu.");
				}
			}
			else if(cmd == ID+2) API::exit();
		}
	});

	subclass sc(fm);
	sc.make_before(WM_SYSCOMMAND, [&fm](UINT, WPARAM wparam, LPARAM, LRESULT*)
	{
		if(wparam == SC_CLOSE)
		{
			API::exit();
			return false;
		}
		if(wparam == SC_MINIMIZE) {
			fm.hide();
			return false;
		}
		return true;
	});

	sc.make_before(WM_KEYUP, [&fm](UINT, WPARAM wparam, LPARAM, LRESULT*)
	{
		if(wparam == VK_ESCAPE)
		{
			fm.hide();
			return false;
		}
		return true;
	});

	sc.make_before(WM_ENDSESSION, [](UINT, WPARAM wparam, LPARAM, LRESULT*)
	{
		if(wparam == TRUE) SaveSettings();
		return true;
	});

	auto plc = place(fm);
	plc.div(R"(<form margin=15 vertical arrange=86
					<<monitored vertical arrange=[15,variable] margin=[0,7,0,0]>
				     <running   vertical arrange=[15,variable] margin=[0,0,0,8]>>
					<weight=15>>)");

	const int padding = 15;

	label lb1(fm);
	lb1.fgcolor(color_rgb(0x555555));
	lb1.caption("Processes with windows being monitored:");

	auto itemComparator = []( const std::string& s1, nana::any*, const std::string& s2, nana::any*, bool reverse) {
		int res = _stricmp(s1.c_str(), s2.c_str());
		return reverse ? res > 0 : res < 0;
	};

	listbox list1(fm);
	list1.bgcolor(color_rgb(0xfbfbfb));
	list1.fgcolor(color_rgb(0x909090));
	list1.show_header(false);
	list1.append_header("Process Name");
	list1.column_at(0).fit_content();
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
	list2.column_at(0).fit_content();
	list2.scheme().item_selected = color_rgb(0xdcefe8);
	list2.scheme().item_highlighted = color_rgb(0xeaf0ef);
	list2.set_sort_compare(0, itemComparator);

	label lb2(fm);
	lb2.fgcolor(color_rgb(0x555555));
	lb2.caption("Processes currently running that have visible windows:");

	plc.field("running") << lb2 << list2;

	label lbinfo(fm);
	lbinfo.bgcolor(color_rgb(0xfbfbfb));
	lbinfo.fgcolor(color_rgb(0x555555));
	lbinfo.format(true);

	plc.field("form") << lbinfo;

	drawing dw(fm);
	dw.draw([&lbinfo](paint::graphics& graph)
	{
		graph.round_rectangle(rectangle(lbinfo.pos().x-1, lbinfo.pos().y-1, lbinfo.size().width+2, lbinfo.size().height+2),
			2, 2, color_rgb(0xd5d5d5), false, colors::red);
		graph.round_rectangle(rectangle(lbinfo.pos().x-2, lbinfo.pos().y-2, lbinfo.size().width+4, lbinfo.size().height+4),
			2, 2, color_rgb(0xefefef), false, colors::red);
		graph.round_rectangle(rectangle(lbinfo.pos().x-3, lbinfo.pos().y-3, lbinfo.size().width+6, lbinfo.size().height+6),
			2, 2, color_rgb(0xf8f8f8), false, colors::red);
		graph.round_rectangle(rectangle(lbinfo.pos().x-4, lbinfo.pos().y-4, lbinfo.size().width+8, lbinfo.size().height+8),
			3, 3, color_rgb(0xfcfcfc), false, colors::red);
	});
	dw.update();

	list2.events().dbl_click([&list1, &list2]
	{
		auto lb = list2.at(0);
		auto selection = list2.selected();
		if(selection.size() == 1)
		{
			string seltext;
			seltext = lb.at(selection[0].item).text(0);
			for(auto &monwin : monwins) if(monwin.second.pname == seltext) return;
			auto &win = windows.at(strlower(seltext));
			int style(0);
			if(!win.borderless) style = GetWindowLongPtr(win.hwnd, GWL_STYLE);
			monwins[strlower(seltext)] = {style, false, seltext};
			list1.auto_draw(false);
			list1.at(0).push_back(seltext);
			list1.column_at(0).fit_content();
			list1.auto_draw(true);
			mon_timer_fn();
			PostMessage(win.hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			SetForegroundWindow(hwnd);
		}
	});

	list2.events().mouse_down([&list1, &list2, &lbinfo](const arg_mouse &arg)
	{
		auto lb = list2.at(0);
		auto hovered = list2.cast(point(arg.pos.x, arg.pos.y));
		if(!hovered.empty())
		{
			hovered.item = lb.index_cast(hovered.item, true);
			for(unsigned n(0); n<lb.size(); n++)
				if(n == hovered.item) lb.at(n).select(true);
				else lb.at(n).select(false);
		}
		auto selection = list2.selected();
		if(selection.size() != 1) { enum_timer_fn(list1, list2, lbinfo); last.clear(); }
	});

	list2.events().selected([&list1, &list2, &lbinfo](const arg_listbox &arg)
	{
		if(IsWindowVisible(hwnd) && !IsIconic(hwnd))
		{
			if(list1.selected().size()) last.clear();
			list1.at(0).select(false);
			auto lb = list2.at(0);
			auto selection = list2.selected();
			if(selection.size() == 1)
			{
				string seltext;
				seltext = strlower(lb.at(selection[0].item).text(0));
				if(last != seltext)
				{
					lbinfo.text_align(align::left);
					lbinfo.typeface(paint::font("Segoe UI", 10, detail::font_style(0)));
					auto it = windows.find(seltext);
					if (it != windows.end())
					{
						const auto &win = it->second;
						string caption = R"(<font="Segoe UI Semibold">PID:</> )" + to_string(win.procid);
						caption += "  |  <font=\"Segoe UI Semibold\">Window handle:</> " + to_hex_string(win.hwnd);
						caption += "  |  Window ";
						if(win.borderless)
						{
							if(monwins.find(seltext) != monwins.end() && monwins[seltext].style) 
								caption += "border has been removed";
							else caption += "doesn't have a border";
						}
						else caption += "has a border (monitoring will remove it)";
						caption += "\n\n<font=\"Segoe UI Semibold\">Window title:</> " + charset(win.captionw).to_bytes(unicode::utf8);
						lbinfo.caption(caption);
					}
					else { lbinfo.caption("Unexpected error - can't find window!"); }
					last = seltext;
				}
			}
		}
	});

	list1.events().selected([&list1, &list2, &lbinfo](const arg_listbox &arg)
	{
		if(IsWindowVisible(hwnd) && !IsIconic(hwnd))
		{
			if(list2.selected().size()) last.clear();
			list2.at(0).select(false);
			auto lb = list1.at(0);
			auto selection = list1.selected();
			if(selection.size() == 1)
			{
				string seltext = strlower(lb.at(selection[0].item).text(0));
				if(last != seltext)
				{
					lbinfo.text_align(align::center, align_v::center);
					lbinfo.typeface(paint::font("Segoe UI", 10, detail::font_style(0, true)));
					string caption;
					if(windows.find(seltext) == windows.end())
						caption += string("The process is not currently running.") + (monwins.at(seltext).style ?
							" Its window has a border, which will be removed when it is run." : "");
					if(caption.empty())
					{
						caption = "The process is currently running. Its window is being monitored and "
							"will be minimized while not in focus.";
						if(monwins.at(seltext).style) caption += " The border has been removed, and the window "
							"has been resized to fill the screen.";
					}
					string modpath = monwins.at(seltext).modpath.string();
					if(!modpath.empty())
					{
						if(caption.find("has been removed") == string::npos) caption += "\n";
						caption += "\n<color=0x117011>" + modpath + "</>";
					}
					lbinfo.caption(caption);
					last = seltext;
				}
			}
		}
	});

	list1.events().mouse_down([&list1, &list2, &lbinfo](const arg_mouse &arg)
	{
		if(!arg.shift && !arg.ctrl)
		{
			auto lb = list1.at(0);
			auto hovered = list1.cast(point(arg.pos.x, arg.pos.y));
			if(!hovered.empty())
			{
				hovered.item = lb.index_cast(hovered.item, true);
				for(unsigned n(0); n<lb.size(); n++)
					if(n == hovered.item) lb.at(n).select(true);
					else lb.at(n).select(false);
			}
		}
		auto selection = list1.selected();
		if(selection.empty()) { enum_timer_fn(list1, list2, lbinfo); last.clear(); }
	});

	list1.events().key_release([&list1](const arg_keyboard &arg)
	{
		if(arg.key == VK_DELETE)
		{
			auto lb = list1.at(0);
			auto selection = list1.selected();
			int deleted(0);
			for(auto &selitem : selection)
			{
				string seltext = strlower(lb.at(selitem.item-deleted).text(0));
				auto it = windows.find(seltext);
				if (it != windows.end()) {
					const enumwin& win = it->second;
					if(monwins.at(seltext).style) SetWindowLongPtr(win.hwnd, GWL_STYLE, monwins.at(seltext).style);
				}
				monwins.erase(seltext);
				list1.erase(lb.at(selitem.item-deleted++));
				list1.column_at(0).fit_content();
			}
		}
	});

	list1.events().dbl_click([&list1]
	{
		auto lb = list1.at(0);
		auto selection = list1.selected();
		if(selection.size() == 1)
		{
			string seltext = strlower(lb.at(selection[0].item).text(0));
			auto it = windows.find(seltext);
			if (it != windows.end())
			{
				const enumwin& win = it->second;
				if(monwins.at(seltext).style) SetWindowLongPtr(win.hwnd, GWL_STYLE, monwins.at(seltext).style);
			}
			monwins.erase(seltext);
			list1.erase(lb.at(selection[0].item));
			list1.column_at(0).fit_content();
			last.clear();
		}
	});

	timer enum_timer;
	enum_timer.interval(1000ms);
	enum_timer.elapse([&list1, &list2, &lbinfo] { enum_timer_fn(list1, list2, lbinfo); });
	enum_timer.start();
	enum_timer_fn(list1, list2, lbinfo);

	timer mon_timer;
	mon_timer.interval(250ms);
	mon_timer.elapse(mon_timer_fn);
	mon_timer.start();

	list1.auto_draw(false);
	for(auto &monwin : monwins) list1.at(0).push_back(monwin.second.pname);
	list1.column_at(0).fit_content();
	list1.auto_draw(true);
	
	sc.make_before(WM_ACTIVATE, [&list1, &list2, &lbinfo](UINT, WPARAM wparam, LPARAM, LRESULT*)
	{
		if(LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE)
		{
			for(auto &monwin : monwins)
			{
				wstring modpath = monwin.second.modpath;
				if(modpath.size() && !std::filesystem::exists(modpath))
				{
					monwin.second.modpath = "";
					for(auto &item : list1.at(0))
					{
						if(monwin.second.pname == item.text(0))
						{
							item.icon(iconapp);
							break;
						}
					}
				}
			}
			last.clear();
			enum_timer_fn(list1, list2, lbinfo);
			auto selection = list1.selected();
			if(!selection.empty())
			{
				list1.at(0).at(selection[0].item).select(false);
				list1.at(0).at(selection[0].item).select(true);
			}
			selection = list2.selected();
			if(!selection.empty())
			{
				list2.at(0).at(selection[0].item).select(false);
				list2.at(0).at(selection[0].item).select(true);
			}
		}
		return true;
	});

	if(show)
	{
		plc.collocate();
		fm.show();
	}
	nana::exec();
	ntfr.close();
}

// monitors windows from list1 and minimizes them, removing borders if any
void mon_timer_fn()
{
	for(auto &monwin : monwins)
	{
		auto it = windows.find(strlower(monwin.second.pname));
		if (it == windows.end()) continue; // process not running
		enumwin& win = it->second;
		if(!win.borderless) // remove borders
		{
			MONITORINFO mi = {sizeof(mi)};
			GetMonitorInfoW(win.monitor, &mi);
			SetWindowLongPtr(win.hwnd, GWL_STYLE, (monwin.second.style & ~(WS_CAPTION|WS_THICKFRAME)) | WS_POPUP);
			SetWindowPos(win.hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			win.borderless = true;
		}
		HWND fghwnd = GetForegroundWindow();
		if(monwin.second.active)
		{
			if(fghwnd != win.hwnd && win.monitor == MonitorFromWindow(fghwnd, MONITOR_DEFAULTTOPRIMARY))
			{
				wstring fgclassname = GetClassNameString(fghwnd);
				if(fgclassname != L"TaskSwitcherWnd" && fgclassname != L"Ghost")
				{
					monwin.second.active = false;
					PostMessage(win.hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				}
			}
		}
		else if(fghwnd == win.hwnd) monwin.second.active = true;
	}
}

// updates list2 from current data
void enum_timer_fn(listbox &list1, listbox &list2, label &info)
{
	enum_windows(list1);
	auto lb1 = list1.at(0), lb2 = list2.at(0);
	auto selection1 = list1.selected(), selection2 = list2.selected();
	string seltext1, seltext2;
	if(selection2.size() == 1) 
		seltext2 = lb2.at(selection2[0].item).text(0);
	else if(selection1.size() == 0 && IsWindowVisible(hwnd) && !IsIconic(hwnd))
	{
		info.text_align(align::center, align_v::center);
		info.typeface(paint::font("Segoe UI", 10, detail::font_style(0, true)));
		info.caption("<color=0x666666>Select a process in the lists above to see some info about it here.\n"
			"Double-click a process in the right list to start monitoring its window.\n"
			"Select one or more processes in the left list and press the \"Delete\" key to remove them.");
	}
	list2.auto_draw(false);
	for(auto &item : lb2) // remove from list2 processes no longer running
	{
		if(windows.find(strlower(item.text(0))) == windows.end()) 
			list2.erase(item);
	}
	for(auto &win : windows) // add to list2 running processes that are not already in the list
	{
		bool found(false);
		for(auto &item : lb2)
			if(item.text(0) == win.second.pname)
			{
				found = true;
				break;
			}
		if(!found) lb2.push_back(win.second.pname);
	}

	for(auto &item : list2.at(0))
	{
		if(item.text(0) == seltext2) item.select(true);
		if(windows.at(strlower(item.text(0))).borderless)
			item.fgcolor(color_rgb(0x883311));
		else item.fgcolor(list2.fgcolor());
	}
	list2.column_at(0).fit_content();
	list2.auto_draw(true);
	if(IsWindowVisible(hwnd) && !IsIconic(hwnd))
	{
		list1.auto_draw(false);
		for(auto &item : list1.at(0))
		{
			if(windows.find(strlower(item.text(0))) == windows.end())
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


void enum_windows(const listbox& list1)
{
	WNDENUMPROC enumfn = [](HWND hwnd, LPARAM lparam) -> BOOL
	{
		auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
		wstring caption = GetWindowTextString(hwnd);
		if((style & WS_VISIBLE) && !caption.empty() && caption != L"Program Manager")
		{
			DWORD procid(0);
			GetWindowThreadProcessId(hwnd, &procid);
			HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, 0, procid);
			auto procpath = GetModuleFileNameExPath(hproc);
			if(procpath != self_path)
			{
				enumwin win;
				win.procid = procid;
				win.pname = procpath.filename().string();
				win.hwnd = hwnd;
				win.monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
				win.captionw = caption;
				if(!(style & (WS_CAPTION|WS_THICKFRAME))) win.borderless = true;
				string key = strlower(procpath.filename().string());
				windows[key] = win;
				if(icons.find(key) == icons.end())
					icons[key] = paint::image(procpath);
				auto it = monwins.find(key);
				if (it != monwins.end())
				{
					if(it->second.modpath.empty())
					{
						it->second.modpath = procpath;
						listbox* list1 = (listbox*)lparam;
						for(auto &item : list1->at(0))
							if(item.text(0) == procpath.filename())
							{
								item.icon(icons[key]);
								break;
							}
					}
				}
			}
		}
		return TRUE;
	};

	windows.clear();
	EnumWindows(enumfn, (LPARAM)&list1);
}


void LoadSettings()
{
	IniFile ini(inifile);
	monwins.clear();
	int n(0), style(0);
	string pname;
	do
	{
		pname = ini.ReadString(to_string(n), "p", "");
		if(pname.size())
		{
			style = ini.ReadInt(to_string(n++), "s", 0);
			if(pname.find('\\') != string::npos)
			{
				std::filesystem::path p(pname);
				string key = strlower(p.filename().string());
				if(std::filesystem::exists(pname))
				{
					icons[key] = paint::image(pname);
					monwins[key] = {style, false, p.filename().string(), p};
				}
				else monwins[key] = {style, false, p.filename().string()};
			}
			else monwins[strlower(pname)] = {style, false, pname};
		}
	}
	while(pname.size());
	iconapp.open(GetSysFolderLocation(CSIDL_SYSTEM) / L"svchost.exe");
}


void SaveSettings()
{
	std::filesystem::remove(inifile);
	IniFile ini(inifile);
	int idx(0);
	for(auto &monwin : monwins)
	{
		string s = to_string(idx++), modpath = monwin.second.modpath.string();
		ini.WriteString(s, "p", modpath.empty() ? monwin.second.pname : modpath);
		ini.WriteInt(s, "s", monwin.second.style);
	}
}