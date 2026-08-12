#pragma once

#include <optional>
#include <string>
#include <memory>

namespace pfd {
    class open_file;
    class save_file;
}

class OS {
public:
    enum class DialogType {
        Open,
        Save
    };

    struct DialogResult {
        DialogType type;
        std::string path;
    };

    // Start a native file dialog
    // These functions return immediately
    static bool open();
    static bool saveAs();

    // Check whether the currently open dialog has finished
    // Returns a result only when the dialog has completed successfully
    static std::optional<DialogResult> pollDialog();

    // Returns true while a file dialog is active
    static bool isDialogOpen();

    static void showConsole(bool show);

private:
    static std::unique_ptr<pfd::open_file> m_openDialog;
    static std::unique_ptr<pfd::save_file> m_saveDialog;
};
