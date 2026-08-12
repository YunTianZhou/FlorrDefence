#pragma once
#include <string>
#include <filesystem>

struct OS {
	// Show a Save-As dialog. Returns true and sets outPath on success
	static bool saveAs(std::string& outPath);

	// Show an Open dialog. Returns true and sets outPath on success
	static bool open(std::string& outPath);

	// Show or hide the console (windows only)
	static void showConsole(bool show);
};
