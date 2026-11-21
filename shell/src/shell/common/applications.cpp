// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/applications.h"

#include "base/application_registry.h"

namespace Lingmo::Shell {

RegistryApplications::RegistryApplications()
    : m_registry(base::ApplicationRegistry::instance())
{
    m_changed = m_registry.changed.connect([this] { changed(); });
}

bool RegistryApplications::isLoading() const
{
    return m_registry.isLoading();
}

DesktopEntryPtr RegistryApplications::byId(const std::string &id) const
{
    return id.empty() ? nullptr : m_registry.byId(id);
}

DesktopEntryPtr RegistryApplications::byPath(const std::string &path) const
{
    return path.empty() ? nullptr : m_registry.byPath(path);
}

DesktopEntryPtr RegistryApplications::resolveWindow(const std::string &appId, const std::string &windowClass,
                                                    const std::string &executablePath,
                                                    const std::string &executableName) const
{
    return m_registry.resolveWindow(appId, windowClass, executablePath, executableName);
}

const std::vector<DesktopEntryPtr> &RegistryApplications::applications() const
{
    return m_registry.applications();
}

} // namespace Lingmo::Shell
