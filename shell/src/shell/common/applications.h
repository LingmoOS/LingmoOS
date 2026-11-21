// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>
#include <vector>

#include "base/desktop_entry.h"
#include "base/signal.h"

namespace Lingmo::base {
class ApplicationRegistry;
} // namespace Lingmo::Base

namespace Lingmo::Shell {

using DesktopEntryPtr = std::shared_ptr<const base::DesktopEntry>;

// The installed applications as the shell's models read them. The session
// has one, over the toolkit's ApplicationRegistry; tests hand in entries of
// their own.
class Applications
{
public:
    virtual ~Applications() = default;

    // Until the first scan has arrived, and while another runs.
    virtual bool isLoading() const = 0;
    // An exact id first, then one that differs only in case.
    virtual DesktopEntryPtr byId(const std::string &id) const = 0;
    virtual DesktopEntryPtr byPath(const std::string &path) const = 0;
    // The entry a window or process belongs to; null when none fits.
    virtual DesktopEntryPtr resolveWindow(const std::string &appId, const std::string &windowClass,
                                          const std::string &executablePath,
                                          const std::string &executableName) const = 0;
    // What menus show, sorted by name.
    virtual const std::vector<DesktopEntryPtr> &applications() const = 0;

    // After each scan.
    base::Signal<> changed;
};

// The toolkit's registry of the calling thread's loop.
class RegistryApplications : public Applications
{
public:
    RegistryApplications();

    bool isLoading() const override;
    DesktopEntryPtr byId(const std::string &id) const override;
    DesktopEntryPtr byPath(const std::string &path) const override;
    DesktopEntryPtr resolveWindow(const std::string &appId, const std::string &windowClass,
                                  const std::string &executablePath,
                                  const std::string &executableName) const override;
    const std::vector<DesktopEntryPtr> &applications() const override;

private:
    base::ApplicationRegistry &m_registry;
    base::Subscription m_changed;
};

} // namespace Lingmo::Shell

