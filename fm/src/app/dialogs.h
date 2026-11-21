// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/fileitem.h"
#include "ops/operations.h"

#include "base/signal.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class ConfirmDialog;
class Window;
}

namespace fm {

class FramelessDialog;

// The file manager's dialogs. Each belongs to the window it was opened over,
// if any, and closes with it; progress dialogs outlive their windows.
class Dialogs
{
public:
    Dialogs();
    ~Dialogs();

    // Progress and conflicts of every task, shown only if it takes a while.
    void ioTask(const OperationTaskPtr& task);
    void confirmDelete(const std::vector<std::string>& urls, ui::Window* parent);
    void confirmEmptyTrash(ui::Window* parent);
    void confirmLaunch(const std::string& path, ui::Window* parent);
    void properties(const FileItemList& items, ui::Window* parent);
    void openWith(const std::string& url, ui::Window* parent);

    // The window goes: its dialogs go with it.
    void closeOwnedBy(ui::Window* window);
    bool isEmpty() const { return m_dialogs.empty() && m_confirms.empty(); }

    // A dialog opened or closed.
    std::function<void()> onChanged;

private:
    struct Confirm
    {
        std::unique_ptr<ui::ConfirmDialog> dialog;
        ui::Window* parent;
        // Sets the title, the text and the confirming button's text.
        std::function<void(ui::ConfirmDialog&)> texts;
    };

    void retranslate();
    void add(std::unique_ptr<FramelessDialog> dialog, ui::Window* parent);
    void remove(FramelessDialog* dialog);
    // The dialog, shown; `texts` runs again when the language changes.
    ui::ConfirmDialog* confirm(ui::Window* parent, std::function<void(ui::ConfirmDialog&)> texts, bool destructive,
                               std::function<void()> accepted);
    void removeConfirm(ui::ConfirmDialog* dialog);

    std::vector<std::pair<std::unique_ptr<FramelessDialog>, ui::Window*>> m_dialogs;
    std::vector<Confirm> m_confirms;
    base::Subscription m_taskAdded;
    base::Subscription m_languageChanged;
};

} // namespace fm
