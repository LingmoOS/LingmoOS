#pragma once

#include <string>
#include <vector>

namespace dm {

struct Config {
    // Separate VTs keep a session's end off the screen; VT1 has the boot splash.
    int greeterVt = 1;
    int sessionVt = 2;
    std::string greeterUser = "lingmo-greeter";
    std::vector<std::string> greeterCommand = {"/usr/bin/lingmo-aurora", "--login"};
    std::string greeterService = "lingmo-dm-greeter";
    std::string sessionService = "lingmo-dm";
    // A separate PAM service for passwordless autologin: skips pam_unix and
    // common-auth, so it works whether or not the account has a password set.
    std::string autologinService = "lingmo-dm-autologin";
    std::vector<std::string> sessionDirectories = {"/usr/local/share/wayland-sessions", "/usr/share/wayland-sessions"};
    std::string defaultSession = "lingmo-wayland";
    // Logged in without a password once per boot, to `autologinSession` or the
    // default; nobody when empty.
    std::string autologinUser;
    std::string autologinSession;
    std::string statePath = "/var/lib/lingmo-dm/state.conf";
    std::string backgroundDirectory = "/var/lib/lingmo-dm/backgrounds";
    std::string runtimeDirectory = "/run/lingmo-dm";

    // Defaults, overridden by whatever `path` sets.
    static Config load(const std::string &path = "/etc/lingmo-dm.conf");
};

// Splits a command line at white space; double quotes group, and three in a
// row stand for a quote character.
std::vector<std::string> splitCommand(const std::string &command);

} // namespace dm
