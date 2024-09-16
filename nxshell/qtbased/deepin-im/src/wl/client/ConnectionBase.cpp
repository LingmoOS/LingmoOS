// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ConnectionBase.h"

#include "common/common.h"

#include <poll.h>

using namespace wl::client;

const wl_registry_listener ConnectionBase::registryListener_ = {
    CallbackWrapper<&ConnectionBase::onGlobal>::func,
    CallbackWrapper<&ConnectionBase::onGlobalRemove>::func,
};

ConnectionBase::ConnectionBase() { }

ConnectionBase::~ConnectionBase() { }

void ConnectionBase::init()
{
    auto *registry = wl_display_get_registry(display());
    wl_registry_add_listener(registry, &registryListener_, this);
    roundtrip();
}

void ConnectionBase::roundtrip()
{
    wl_display_roundtrip(display());
}

void ConnectionBase::flush()
{
    wl_display_flush(display());
}

void ConnectionBase::onGlobal([[maybe_unused]] struct wl_registry *registry,
                              uint32_t name,
                              const char *interface,
                              uint32_t version)
{
    auto &gi = globals_[interface];
    gi.version = version;
    gi.names.emplace(name);
}

void ConnectionBase::onGlobalRemove([[maybe_unused]] struct wl_registry *wl_registry, uint32_t name)
{
    for (auto &[_, t] : bindedGlobals_) {
        if (t.erase(name)) {
            break;
        }
    }

    for (auto &gi : globals_) {
        if (gi.second.names.erase(name)) {
            break;
        }
    }
}
