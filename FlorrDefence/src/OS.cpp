#include "OS.hpp"

#include <portable-file-dialogs.h>

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#endif

std::unique_ptr<pfd::open_file> OS::m_openDialog;
std::unique_ptr<pfd::save_file> OS::m_saveDialog;

bool OS::open() {
    // Don't allow multiple file dialogs at once
    if (isDialogOpen())
        return false;

    m_openDialog = std::make_unique<pfd::open_file>(
        "Open",
        "",
        std::vector<std::string>{
            "JSON Files", "*.json",
            "All Files", "*"
        }
    );

    return true;
}

bool OS::saveAs() {
    // Don't allow multiple file dialogs at once
    if (isDialogOpen())
        return false;

    m_saveDialog = std::make_unique<pfd::save_file>(
        "Save As",
        "FlorrDefence.json",
        std::vector<std::string>{
            "JSON Files", "*.json",
            "All Files", "*"
        }
    );

    return true;
}

bool OS::isDialogOpen() {
    return m_openDialog != nullptr || m_saveDialog != nullptr;
}

std::optional<OS::DialogResult> OS::pollDialog() {
    // Open dialog
    if (m_openDialog) {
        if (!m_openDialog->ready(0))
            return std::nullopt;

        std::vector<std::string> result = m_openDialog->result();

        m_openDialog.reset();

        // User cancelled
        if (result.empty())
            return std::nullopt;

        return DialogResult{
            DialogType::Open,
            std::move(result[0])
        };
    }

    // Save dialog
    if (m_saveDialog) {
        if (!m_saveDialog->ready(0))
            return std::nullopt;

        std::string result = m_saveDialog->result();

        m_saveDialog.reset();

        // User cancelled
        if (result.empty())
            return std::nullopt;

        return DialogResult{
            DialogType::Save,
            std::move(result)
        };
    }

    return std::nullopt;
}

void OS::showConsole(bool show) {
#ifdef _WIN32
    if (show) {
        HWND console = GetConsoleWindow();

        if (!console) {
            if (AllocConsole()) {
                FILE* fDummy;

                freopen_s(
                    &fDummy,
                    "CONOUT$",
                    "w",
                    stdout
                );

                freopen_s(
                    &fDummy,
                    "CONIN$",
                    "r",
                    stdin
                );

                freopen_s(
                    &fDummy,
                    "CONOUT$",
                    "w",
                    stderr
                );
            }

            console = GetConsoleWindow();
        }

        if (console)
            ShowWindow(console, SW_SHOW);
    }
    else {
        HWND console = GetConsoleWindow();

        if (console)
            ShowWindow(console, SW_HIDE);

        FreeConsole();
    }
#else
    (void)show;
#endif
}
