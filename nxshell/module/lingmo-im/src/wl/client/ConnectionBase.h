// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_CONNECTIONBASE_H
#define WL_CLIENT_CONNECTIONBASE_H

#include "wl/Type.h"

#include <wayland-client-protocol.h>

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

namespace wl {
namespace client {

struct globalInfo
{
    uint32_t version;
    std::set<uint32_t> names;
};

class ConnectionBase 
{
public:
    ConnectionBase();
    virtual ~ConnectionBase();

    virtual struct wl_display *display() const = 0;
    void roundtrip();
    void flush();

    template<typename T>
    std::vector<std::shared_ptr<T>> getGlobals()
    {
        static_assert(std::is_base_of<Type<typename T::raw_type>, T>::value);

        const std::string &interface = T::interface;
        auto iter = bindedGlobals_.find(interface);
        if (iter == bindedGlobals_.end()) {
            // bind
            auto gIter = globals_.find(interface);
            if (gIter == globals_.end()) {
                return {};
            }

            uint32_t version = gIter->second.version;
            for (auto &name : gIter->second.names) {
                auto *g = static_cast<typename T::raw_type *>(
                    wl_registry_bind(wl_display_get_registry(display()),
                                     name,
                                     T::wl_interface,
                                     version));
                std::shared_ptr<void> ptr = std::make_shared<T>(g);
                bindedGlobals_[interface].emplace(name, ptr);
            }

            iter = bindedGlobals_.find(interface);
        }

        auto &tmp = iter->second;
        std::vector<std::shared_ptr<T>> res;
        res.reserve(tmp.size());

        std::transform(tmp.cbegin(), tmp.cend(), std::back_inserter(res), [](const auto &pair) {
            return std::static_pointer_cast<T>(pair.second);
        });

        return res;
    }

    template<typename T>
    std::shared_ptr<T> getGlobal()
    {
        auto list = getGlobals<T>();
        if (list.empty()) {
            return nullptr;
        }

        return list[0];
    }

protected:
    void init();

private:
    static const wl_registry_listener registryListener_;
    std::unordered_map<std::string, globalInfo> globals_;
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::shared_ptr<void>>>
        bindedGlobals_;

    void
    onGlobal(struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    void onGlobalRemove(struct wl_registry *wl_registry, uint32_t name);
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_CONNECTIONBASE_H
