#include "FileDialog.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif

bool FileDialog::saveAs(std::string& outPath) {
#ifdef _WIN32
	WCHAR szFile[MAX_PATH] = L"";
	OPENFILENAMEW ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetSaveFileNameW(&ofn)) {
		outPath = std::filesystem::path(szFile).string();
		return true;
	}
	return false;
#else
	(void)outPath;
	return false;
#endif
}

bool FileDialog::open(std::string& outPath) {
#ifdef _WIN32
	WCHAR szFile[MAX_PATH] = L"";
	OPENFILENAMEW ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameW(&ofn)) {
		outPath = std::filesystem::path(szFile).string();
		return true;
	}
	return false;
#else
	(void)outPath;
	return false;
#endif
}
