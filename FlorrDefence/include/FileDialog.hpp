#pragma once
#include <string>
#include <filesystem>

struct FileDialog {
	// Show save-as dialog. Returns true and sets outPath on success.
	static bool saveAs(std::string& outPath);

	// Show open dialog. Returns true and sets outPath on success.
	static bool open(std::string& outPath);
};
