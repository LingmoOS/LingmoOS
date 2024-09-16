// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_GLOBAL_H
#define WL_ADDONS_BASE_GLOBAL_H

#include <wayland-server.h>

#include <filesystem>
#include <string>
#include <string_view>

#define WL_ADDONS_BASE_BEGIN_NAMESPACE namespace wl { namespace addons { namespace base{
#define WL_ADDONS_BASE_END_NAMESPACE }}}
#define WL_ADDONS_BASE_USE_NAMESPACE using namespace wl::addons::base;
#define WL_ADDONS_BASE_NAMESPACE wl::addons::base

inline pid_t getPid(wl_resource *resource) {
    pid_t pid = 0;

    auto *client = wl_resource_get_client(resource);
    wl_client_get_credentials(client, &pid, nullptr, nullptr);

    return pid;
}

inline std::string getExePathByPid(pid_t pid) {
    namespace fs = std::filesystem;

    std::string file = std::string("/proc/") + std::to_string(pid) + "/exe";
    if (fs::exists(file) && fs::is_symlink(file)) {
        return fs::read_symlink(file).string();
    }

    return "";
}

inline bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

inline bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

#endif //!WL_ADDONS_BASE_GLOBAL_H
