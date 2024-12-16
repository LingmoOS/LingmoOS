// Copyright (C) 2023 WenHao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wayland-server-core.h>

#include <qwdisplay.h>
#include <QObject>
#include <QStringList>

struct personalization_window_context_v1;
struct personalization_wallpaper_context_v1;
struct personalization_cursor_context_v1;
struct wlr_surface;

struct treeland_personalization_manager_v1 : public QObject
{
    Q_OBJECT
public:
    explicit treeland_personalization_manager_v1();
    ~treeland_personalization_manager_v1();

    wl_event_loop *event_loop;
    wl_global *global;
    wl_list resources; // wl_resource_get_link()

    static treeland_personalization_manager_v1* create(QW_NAMESPACE::QWDisplay *display);

Q_SIGNALS:
    void beforeDestroy();
    void windowContextCreated(personalization_window_context_v1 *context);
    void wallpaperContextCreated(personalization_wallpaper_context_v1 *context);
    void cursorContextCreated(personalization_cursor_context_v1 *context);
};

struct personalization_window_context_v1 : public QObject
{
    Q_OBJECT
public:
    ~personalization_window_context_v1();
    treeland_personalization_manager_v1 *manager;
    wlr_surface *surface;
    uint32_t background_type;

Q_SIGNALS:
    void beforeDestroy();
    void backgroundTypeChanged(personalization_window_context_v1 *handle);
};

struct personalization_wallpaper_context_v1 : public QObject
{
    Q_OBJECT
public:
    ~personalization_wallpaper_context_v1();
    treeland_personalization_manager_v1 *manager;
    wl_resource *resource;
    int32_t fd;
    uint32_t uid;
    uint32_t options;
    bool isdark;

    QString meta_data;
    QString identifier ;
    QString output_name;

    void set_meta_data(const QString &data);

Q_SIGNALS:
    void beforeDestroy();
    void commit(personalization_wallpaper_context_v1 *handle);
    void getWallpapers(personalization_wallpaper_context_v1 *handle);
};

struct personalization_cursor_context_v1 : public QObject
{
    Q_OBJECT
public:
    ~personalization_cursor_context_v1();
    treeland_personalization_manager_v1 *manager;
    wl_resource *resource;

    uint32_t size;
    QString theme;

    void set_theme(const QString &theme);
    void set_size(uint32_t size);
    void verfity(bool verfityed);

Q_SIGNALS:
    void beforeDestroy();
    void commit(personalization_cursor_context_v1 *handle);
    void get_size(personalization_cursor_context_v1 *handle);
    void get_theme(personalization_cursor_context_v1 *handle);
};
