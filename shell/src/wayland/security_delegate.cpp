// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/security_delegate.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <wayland-server-core.h>

namespace Lingmo::Wayland {

namespace {

// Empty when `path` does not resolve.
std::string canonicalPath(const std::string &path)
{
    char resolved[PATH_MAX];
    return realpath(path.c_str(), resolved) ? std::string(resolved) : std::string();
}

std::vector<std::string> canonicalPaths(const std::vector<std::string> &paths)
{
    std::vector<std::string> result;
    for (const auto &path : paths) {
        std::string canonical = canonicalPath(path);
        if (!canonical.empty())
            result.push_back(std::move(canonical));
    }
    return result;
}

// /proc/<pid> itself belongs to the process owner; its exe link would resolve
// to a root-owned file under /usr.
bool runsOneOf(pid_t pid, const std::vector<std::string> &executables)
{
    if (pid <= 0)
        return false;
    const std::string process = "/proc/" + std::to_string(pid);
    struct stat info {};
    if (stat(process.c_str(), &info) != 0 || info.st_uid != getuid())
        return false;
    const std::string executable = canonicalPath(process + "/exe");
    return std::find(executables.begin(), executables.end(), executable) != executables.end();
}

bool runsOneOf(const wl_client *client, const std::vector<std::string> &executables)
{
    pid_t pid = 0;
    uid_t uid = 0;
    gid_t gid = 0;
    wl_client_get_credentials(const_cast<wl_client *>(client), &pid, &uid, &gid);
    return uid == getuid() && runsOneOf(pid, executables);
}

} // namespace

ExecutableSecurityDelegate::ExecutableSecurityDelegate(
        const std::vector<std::string> &clipboardClients,
        const std::vector<std::string> &captureClients,
        const std::vector<std::string> &systemDialogClients, bool captureEnabled,
        const std::vector<std::string> &computerControlClients,
        const std::vector<std::string> &windowPlacementClients)
    : m_clipboardClients(canonicalPaths(clipboardClients))
    , m_captureClients(canonicalPaths(captureClients))
    , m_systemDialogClients(canonicalPaths(systemDialogClients))
    , m_computerControlClients(canonicalPaths(computerControlClients))
    , m_windowPlacementClients(canonicalPaths(windowPlacementClients))
    , m_captureEnabled(captureEnabled)
{
}

bool ExecutableSecurityDelegate::canManageClipboard(const wl_client *client) const
{
    return runsOneOf(client, m_clipboardClients);
}

bool ExecutableSecurityDelegate::canCaptureOutputs(const wl_client *client) const
{
    return m_captureEnabled && runsOneOf(client, m_captureClients);
}

bool ExecutableSecurityDelegate::showsSystemDialogs(int64_t pid) const
{
    return runsOneOf(static_cast<pid_t>(pid), m_systemDialogClients);
}

bool ExecutableSecurityDelegate::canControlComputer(const wl_client *client) const
{
    return m_captureEnabled && runsOneOf(client, m_computerControlClients);
}

bool ExecutableSecurityDelegate::canPlaceWindows(const wl_client *client) const
{
    return runsOneOf(client, m_windowPlacementClients);
}

} // namespace Lingmo::Wayland
