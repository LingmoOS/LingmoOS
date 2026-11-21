// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <dlfcn.h>
#include "ui/view.h"
namespace Lingmo::Shell {
// Runtime integration keeps the compositor independent of file-manager build
// dependencies. An absent provider leaves notifications fully functional.
// Never unload: SDK views and outstanding jobs may still execute provider code.
struct NotificationWidgetProvider {
    using Create = ui::View* (*)();
    using Active = void (*)(ui::View*, bool);
    Create create = nullptr;
    Active active = nullptr;
    static const NotificationWidgetProvider& instance() {
        static const auto provider=[] {
            NotificationWidgetProvider result;
            if(void* module=dlopen("liblingmo-desktop-widgets.so.1",RTLD_NOW|RTLD_LOCAL)) {
                result.create=reinterpret_cast<Create>(dlsym(module,"lingmo_notification_widgets_create_v1"));
                result.active=reinterpret_cast<Active>(dlsym(module,"lingmo_notification_widgets_active_v1"));
                if(!result.active)result.create=nullptr;
            }
            return result;
        }();
        return provider;
    }
};
}
