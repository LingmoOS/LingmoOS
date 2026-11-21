// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_SECURITY_DELEGATE_H_
#define LINGMO_WAYLAND_SECURITY_DELEGATE_H_

#include <cstdint>
#include <string>
#include <vector>

struct wl_client;

namespace Lingmo::Wayland {

// What a client may do beyond the ordinary protocol surface.
//
// Upstream: chromium/src/components/exo/security_delegate.h, narrowed to the
// privileges Lingmo actually grants. Protocol code asks the delegate instead
// of carrying a policy of its own.
class SecurityDelegate
{
public:
    virtual ~SecurityDelegate() = default;

    virtual bool canManageClipboard(const wl_client *client) const = 0;
    virtual bool canCaptureOutputs(const wl_client *client) const = 0;
    // Whether the process's toplevels are the session's own dialogs, stacked
    // above every application window.
    virtual bool showsSystemDialogs(int64_t pid) const = 0;
    // The computer-use service, which may ask the user to let an agent drive
    // the seat, and reads the screen while it does.
    virtual bool canControlComputer(const wl_client *) const { return false; }
    // Clients that place their own windows and read where they are, as
    // programs written for a desktop where windows do.
    virtual bool canPlaceWindows(const wl_client *) const { return false; }
};

// Grants a privilege when the connecting process runs one of a fixed set of
// executables. The pid comes from the kernel through the socket, so a client
// cannot claim an identity it does not have.
class ExecutableSecurityDelegate final : public SecurityDelegate
{
public:
    // Paths are canonicalized once; entries that do not resolve are dropped, so
    // a client can never match an empty path.
    ExecutableSecurityDelegate(const std::vector<std::string> &clipboardClients,
                               const std::vector<std::string> &captureClients,
                               const std::vector<std::string> &systemDialogClients,
                               bool captureEnabled,
                               const std::vector<std::string> &computerControlClients = {},
                               const std::vector<std::string> &windowPlacementClients = {});

    bool canManageClipboard(const wl_client *client) const override;
    bool canCaptureOutputs(const wl_client *client) const override;
    bool showsSystemDialogs(int64_t pid) const override;
    bool canControlComputer(const wl_client *client) const override;
    bool canPlaceWindows(const wl_client *client) const override;

private:
    const std::vector<std::string> m_clipboardClients;
    const std::vector<std::string> m_captureClients;
    const std::vector<std::string> m_systemDialogClients;
    const std::vector<std::string> m_computerControlClients;
    const std::vector<std::string> m_windowPlacementClients;
    // Capture has nothing to read without a display, and computer control
    // nothing to show.
    const bool m_captureEnabled;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_SECURITY_DELEGATE_H_
