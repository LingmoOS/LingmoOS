#pragma once

#include <optional>
#include <string>
#include <vector>

namespace dm {

// One entry of wayland-sessions, as sddm and GDM offer it.
struct SessionEntry {
    // The file name without ".desktop", e.g. "lingmo-wayland".
    std::string id;
    std::string name;
    std::vector<std::string> exec;
    std::vector<std::string> desktopNames;
};

// Hidden, NoDisplay, a missing TryExec or no Exec: not a session.
std::optional<SessionEntry> readSessionFile(const std::string &path);

// An id in an earlier directory hides the same id in later ones.
std::vector<SessionEntry> findSessions(const std::vector<std::string> &directories);

std::optional<SessionEntry> findSession(const std::vector<std::string> &directories, const std::string &id);

// For pam_systemd, set before pam_open_session (sddm's Display::startAuth).
std::vector<std::string> sessionEnvironment(const SessionEntry &session, int vt);
std::vector<std::string> greeterEnvironment(int vt, const std::string &socket);

} // namespace dm
