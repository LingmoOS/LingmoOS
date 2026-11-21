// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/archivepasswordprompt.h"
#include "dialogs/archivepassworddialog.h"
#include "model/foldermodel.h"

#include "base/event_loop.h"

namespace fm {

ArchivePasswordPrompt::ArchivePasswordPrompt() = default;

ArchivePasswordPrompt::~ArchivePasswordPrompt() = default;

void ArchivePasswordPrompt::setModel(FolderModel* model, ui::Window* host)
{
    if (model != m_model) {
        dismissPassword();
        m_passwordRequest = 0;
    }
    m_model = model;
    m_host = host;
    m_changed.reset();
    if (model)
        m_changed = model->archiveChanged.connect([this] { update(); });
    update();
}

void ArchivePasswordPrompt::retranslate()
{
    if (m_password)
        m_password->retranslate();
}

void ArchivePasswordPrompt::update()
{
    const bool busy = m_model && m_model->archiveBusy();
    // A request stays open while the daemon tries the password given.
    const uint32_t request = busy ? m_model->archivePasswordId() : 0;
    if (request == 0)
        dismissPassword();
    // Each job numbers its requests from 1.
    if (!busy)
        m_passwordRequest = 0;
    if (request != 0)
        askPassword(request);
}

void ArchivePasswordPrompt::askPassword(uint32_t request)
{
    if (request == m_passwordRequest || m_model->archiveCancelling())
        return;
    dismissPassword();
    m_passwordRequest = request;
    m_password = std::make_unique<ArchivePasswordDialog>(m_host, *m_model, request, m_model->archiveSourceName(),
                                                         m_model->archivePasswordRetry());
    ArchivePasswordDialog* raw = m_password.get();
    raw->onDone = [this, raw] {
        if (m_password.get() != raw)
            return;
        // Not from inside the dialog's own callback.
        std::shared_ptr<ArchivePasswordDialog> gone(std::move(m_password));
        base::EventLoop::current()->post([gone] {});
    };
    raw->show();
}

void ArchivePasswordPrompt::dismissPassword()
{
    if (m_password)
        m_password->dismiss();
}

} // namespace fm
