// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace Lingmo::ui {
class DialogWindow;
class Window;
}

namespace fm {

class FolderModel;

// The password of an archive being extracted. Closing it without an answer
// cancels the extraction.
class ArchivePasswordDialog
{
public:
    ArchivePasswordDialog(ui::Window* parent, FolderModel& model, uint32_t request, std::string name, bool retry);
    ~ArchivePasswordDialog();

    void show();
    // Closes it without cancelling: the request is gone.
    void dismiss();
    // The password typed stays.
    void retranslate();
    std::function<void()> onDone;

private:
    class Content;

    std::unique_ptr<ui::DialogWindow> m_dialog;
    Content* m_content = nullptr;
    FolderModel& m_model;
    uint32_t m_request;
    std::string m_name;
    bool m_retry;
    bool m_answered = false;
};

} // namespace fm
