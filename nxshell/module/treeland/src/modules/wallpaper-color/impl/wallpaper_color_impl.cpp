// Copyright (C) 2024 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "wallpaper_color_impl.h"

#include <wayland-server-core.h>

#include <QLoggingCategory>

#include <cassert>

Q_LOGGING_CATEGORY(qlcWallpapercolor, "treeland.modules.wallpapercolor", QtWarningMsg)

using QW_NAMESPACE::QWDisplay;

static void color_manager_bind(wl_client *client, void *data, uint32_t version, uint32_t id);
static wallpaper_color_manager_v1 *output_manager_from_resource(wl_resource *resource);

// For treeland_output_manager_v1_interface
static void wallpaper_color_manager_interface_handle_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void wallpaper_color_manager_handle_watch([[maybe_unused]] wl_client *client,
                                                 wl_resource *resource,
                                                 const char *output)
{
    auto *manager = output_manager_from_resource(resource);

    auto op = QString::fromUtf8(output);

    if (!manager->color_map.contains(op)) {
        qCWarning(qlcWallpapercolor)
            << QString("Wallpaper info for output:(%1) never set, will ignore this requset!")
                   .arg(op);
        return;
    }
    auto isDark = manager->color_map[op];
    qCDebug(qlcWallpapercolor) << QString("New watch requset for output:(%1), it's wallpaper is %2")
                                      .arg(op, isDark ? "dark" : "light");
    wallpaper_color_manager_v1_send_output_color(resource, output, isDark);
    manager->watch_lists[resource].append(output);
}

static void wallpaper_color_manager_handle_unwatch([[maybe_unused]] wl_client *client,
                                                   wl_resource *resource,
                                                   const char *output)
{
    auto *manager = output_manager_from_resource(resource);
    manager->watch_lists[resource].removeOne(QString::fromLocal8Bit(output));
}

static const struct wallpaper_color_manager_v1_interface color_manager_impl
{
    .watch = wallpaper_color_manager_handle_watch,
    .unwatch = wallpaper_color_manager_handle_unwatch,
    .destroy = wallpaper_color_manager_interface_handle_destroy,
};

// treeland output manager impl
wallpaper_color_manager_v1::~wallpaper_color_manager_v1()
{
    Q_EMIT beforeDestroy();
    if (global)
        wl_global_destroy(global);
}

wallpaper_color_manager_v1 *wallpaper_color_manager_v1::create(QWDisplay *display)
{
    auto *manager = new wallpaper_color_manager_v1;
    if (!manager) {
        return nullptr;
    }
    manager->global = wl_global_create(display->handle(),
                                       &wallpaper_color_manager_v1_interface,
                                       TREELAND_COLOR_MANAGER_V1_VERSION,
                                       manager,
                                       color_manager_bind);

    QObject::connect(display, &QWDisplay::beforeDestroy, manager, [manager] {
        delete manager;
    });

    return manager;
}

void wallpaper_color_manager_v1::updateWallpaperColor(const QString &output, bool isDarkType)
{
    if (color_map.contains(output)) {
        if (color_map[output] == isDarkType)
            return;
    }
    qCDebug(qlcWallpapercolor)
        << QString("Wallpaper info for output:(%1) changed, it's wallpaper is %2")
               .arg(output, isDarkType ? "dark" : "light");

    color_map[output] = isDarkType;
    for (auto i = watch_lists.cbegin(), end = watch_lists.cend(); i != end; ++i)
        if (i.value().contains(output))
            wallpaper_color_manager_v1_send_output_color(i.key(), output.toLocal8Bit(), isDarkType);
}

static void treeland_color_manager_resource_destroy(struct wl_resource *resource)
{
    qCDebug(qlcWallpapercolor) << "Client destroyed it's resource(" << resource
                               << "), stop watching.";
    auto *manager = output_manager_from_resource(resource);
    manager->watch_lists.remove(resource);
}

static void color_manager_bind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<wallpaper_color_manager_v1 *>(data);

    struct wl_resource *resource =
        wl_resource_create(client, &wallpaper_color_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource,
                                   &color_manager_impl,
                                   manager,
                                   treeland_color_manager_resource_destroy);
}

static wallpaper_color_manager_v1 *output_manager_from_resource(wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &wallpaper_color_manager_v1_interface,
                                   &color_manager_impl));
    auto *manager = static_cast<wallpaper_color_manager_v1 *>(wl_resource_get_user_data(resource));
    assert(manager != nullptr);
    return manager;
}
