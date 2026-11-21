// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "auth/auth_backend.h"
#include "base/process.h"
#include "base/timer.h"
#include <memory>

namespace Lingmo::Auth {
class PamBackend : public AuthBackend
{
public:
    PamBackend();
    ~PamBackend() override;
    std::string defaultUserName() const override;

    void start() override { }

    void authenticate(const std::string &userName, Secret secret) override;
    void reset() override;

    void accept() override { }

private:
    std::unique_ptr<base::Process> m_process;
    base::Timer m_timeout;
    bool m_pending = false;
};
} // namespace Lingmo::Auth

