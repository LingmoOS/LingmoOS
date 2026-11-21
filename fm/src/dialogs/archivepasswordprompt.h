// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/signal.h"

#include <cstdint>
#include <memory>

namespace Lingmo::ui {
class Window;
}

namespace fm {

class ArchivePasswordDialog;
class FolderModel;

// Asks for the password an extraction of the model's waits for, over its
// window. The task's own dialog shows the progress.
class ArchivePasswordPrompt
{
public:
    ArchivePasswordPrompt();
    ~ArchivePasswordPrompt();

    void setModel(FolderModel* model, ui::Window* host);
    void retranslate();

private:
    void update();
    void askPassword(uint32_t request);
    void dismissPassword();

    FolderModel* m_model = nullptr;
    ui::Window* m_host = nullptr;
    base::Subscription m_changed;
    std::unique_ptr<ArchivePasswordDialog> m_password;
    // The request the dialog asks, or asked last, in the running job.
    uint32_t m_passwordRequest = 0;
};

} // namespace fm
