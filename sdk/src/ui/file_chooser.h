// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Lingmo::ui {

class Window;

// The desktop's file dialog, through the org.freedesktop.portal.FileChooser
// portal: xdg-desktop-portal-lingmo shows it in a process of its own and
// one callback answers.
class FileChooser
{
public:
    enum class Mode {
        OpenFile,
        OpenFiles,
        OpenFolder,
        SaveFile,
    };

    struct Filter
    {
        std::string name;
        // Glob patterns such as "*.png", or MIME types such as "image/png".
        std::vector<std::string> patterns;
        std::vector<std::string> mimeTypes;

        // From a name filter: "Image files (*.jpg *.png)".
        static Filter fromNameFilter(std::string_view nameFilter);
    };

    // A control of the program's in the dialog: a combo box of options, ids
    // and labels, or a check box when it has none, "true" or "false".
    struct Choice
    {
        std::string id;
        std::string label;
        std::vector<std::pair<std::string, std::string>> options;
        std::string selected;
    };

    // With a parent, the dialog is attached to it, modal unless told otherwise.
    explicit FileChooser(Window* parent = nullptr);
    // Closes a dialog still open.
    ~FileChooser();

    FileChooser(const FileChooser&) = delete;
    FileChooser& operator=(const FileChooser&) = delete;

    void setMode(Mode mode);
    // "Open File", "Open Folder" or "Save File" unless set.
    void setTitle(std::string title);
    void setAcceptLabel(std::string label);
    void setFilters(std::vector<Filter> filters);
    // One of the filters' names.
    void setCurrentFilter(std::string name);
    void setChoices(std::vector<Choice> choices);
    void setCurrentFolder(std::string path);
    // Saving: the name suggested, and the file it replaces when it exists.
    void setCurrentFile(std::string path);
    void setModal(bool modal);

    void open();
    void close();
    bool isOpen() const;

    // Local paths, and the name of the filter chosen, if any.
    std::function<void(const std::vector<std::string>& paths, const std::string& filter)> onAccepted;
    // While onAccepted runs: each choice's id and the option picked.
    const std::vector<std::pair<std::string, std::string>>& chosenOptions() const { return m_chosenOptions; }
    std::function<void()> onRejected;

    struct Request;

private:
    void call(Request* request, const std::string& parentWindow);

    Window* m_parent;
    Mode m_mode = Mode::OpenFile;
    std::string m_title;
    std::string m_acceptLabel;
    std::vector<Filter> m_filters;
    std::string m_currentFilter;
    std::vector<Choice> m_choices;
    std::vector<std::pair<std::string, std::string>> m_chosenOptions;
    std::string m_currentFolder;
    std::string m_currentFile;
    bool m_modal = true;
    std::unique_ptr<Request> m_request;
    // Gone with the chooser, so a window handle arriving later finds it gone.
    std::shared_ptr<char> m_alive = std::make_shared<char>();
};

} // namespace Lingmo::ui
