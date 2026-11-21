// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "auth/pam_backend.h"
#include "base/i18n.h"
#include "core/main_loop.h"
#include <pwd.h>
#include <unistd.h>

using Lingmo::i18n::tr;

namespace Lingmo::Auth {
PamBackend::PamBackend()
    : m_timeout(core::mainLoop())
{
    m_timeout.setSingleShot(true);
    m_timeout.onTimeout = [this] {
        reset();
        failed(tr("Authentication timed out"));
    };
}

PamBackend::~PamBackend()
{
    reset();
}

std::string PamBackend::defaultUserName() const
{
    const passwd *user = getpwuid(getuid());
    return user && user->pw_name ? std::string(user->pw_name) : std::string();
}

void PamBackend::authenticate(const std::string &userName, Secret secret)
{
    if (m_pending || (m_process && m_process->isRunning()))
        return;
    if (userName != defaultUserName() || secret.size() > 4096 || secret.isEmpty()) {
        failed(tr("Authentication failed"));
        return;
    }
    m_pending = true;
    m_process = std::make_unique<base::Process>(core::mainLoop());
    // A crash is the service failing, not a verdict.
    m_process->onFinished = [this](int code, bool crashed) {
        if (!m_pending)
            return;
        m_pending = false;
        m_timeout.stop();
        if (crashed)
            failed(tr("Authentication service unavailable"));
        else if (code == 0)
            succeeded();
        else
            failed(tr("Authentication failed"));
    };
    // The helper reads only this inherited pipe, never argv or a public socket.
    m_process->setStandardInput(std::string(secret.constData(), secret.size()));
    secret.clear();
    if (!m_process->start(LINGMO_AUTH_HELPER)) {
        m_pending = false;
        m_process.reset();
        failed(tr("Authentication service unavailable"));
        return;
    }
    m_timeout.start(std::chrono::milliseconds(30000));
}

void PamBackend::reset()
{
    m_pending = false;
    m_timeout.stop();
    // Kills a helper still running and waits for it.
    m_process.reset();
}
} // namespace Lingmo::Auth
