#include "OS.hpp"
#include <portable-file-dialogs.h>

#ifdef _WIN32
#include <windows.h>
#endif

bool OS::saveAs(std::string& outPath) {
	auto result = pfd::save_file(
		"Save As",
		"FlorrDefence.json",
		{ "JSON Files", "*.json", "All Files", "*" }
	).result();

	if (result.empty())
		return false;

	outPath = std::move(result);
	return true;
}

bool OS::open(std::string& outPath) {
	auto result = pfd::open_file(
		"Open",
		"",
		{ "JSON Files", "*.json", "All Files", "*" }
	).result();

	if (result.empty())
		return false;

	outPath = std::move(result[0]);
	return true;
}

void OS::showConsole(bool show) {
#ifdef _WIN32
	if (show) {
		HWND console = GetConsoleWindow();
		if (!console) {
			if (AllocConsole()) {
				FILE* fDummy;
				freopen_s(&fDummy, "CONOUT$", "w", stdout);
				freopen_s(&fDummy, "CONIN$", "r", stdin);
				freopen_s(&fDummy, "CONOUT$", "w", stderr);
			}
			console = GetConsoleWindow();
		}
		if (console) ShowWindow(console, SW_SHOW);
	}
	else {
		HWND console = GetConsoleWindow();
		if (console) ShowWindow(console, SW_HIDE);
		FreeConsole();
	}
#else
	// Windows only
	(void)show;
#endif
}
