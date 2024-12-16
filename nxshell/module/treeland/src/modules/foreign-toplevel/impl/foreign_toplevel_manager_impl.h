// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wayland-server-core.h>

#include <qwdisplay.h>
#include <qwoutput.h>

#include <QList>
#include <QObject>
#include <QString>

#include <vector>

struct treeland_dock_preview_context_v1;
struct treeland_foreign_toplevel_handle_v1;

struct treeland_foreign_toplevel_manager_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_foreign_toplevel_manager_v1();
    wl_event_loop *event_loop{ nullptr };
    wl_global *global{ nullptr };
    wl_list resources; // wl_resource_get_link()
    QList<treeland_dock_preview_context_v1 *> dock_preview;
    QList<treeland_foreign_toplevel_handle_v1 *> toplevels;

    static treeland_foreign_toplevel_manager_v1 *create(QW_NAMESPACE::QWDisplay *display);

Q_SIGNALS:
    void beforeDestroy();
    void dockPreviewContextCreated(treeland_dock_preview_context_v1 *context);
};

enum treeland_foreign_toplevel_handle_v1_state {
    TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED = (1 << 0),
    TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED = (1 << 1),
    TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED = (1 << 2),
    TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN = (1 << 3),
};

struct treeland_foreign_toplevel_handle_v1;

struct treeland_foreign_toplevel_handle_v1_output
{
    QW_NAMESPACE::QWOutput *output{ nullptr };
    treeland_foreign_toplevel_handle_v1 *toplevel{ nullptr };
};

struct treeland_dock_preview_context_v1_preview_event;

struct treeland_dock_preview_context_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_dock_preview_context_v1();
    treeland_foreign_toplevel_manager_v1 *manager{ nullptr };
    wl_resource *resource{ nullptr };
    wl_resource *relative_surface{ nullptr };

    void enter();
    void leave();
Q_SIGNALS:
    void beforeDestroy();
    void requestShow(treeland_dock_preview_context_v1_preview_event *event);
    void requestClose();
};

static void toplevel_handle_output_bind(struct wl_listener *listener, void *data);
struct treeland_foreign_toplevel_handle_v1_maximized_event;
struct treeland_foreign_toplevel_handle_v1_minimized_event;
struct treeland_foreign_toplevel_handle_v1_activated_event;
struct treeland_foreign_toplevel_handle_v1_fullscreen_event;
struct treeland_foreign_toplevel_handle_v1_set_rectangle_event;

struct treeland_foreign_toplevel_handle_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_foreign_toplevel_handle_v1();
    treeland_foreign_toplevel_manager_v1 *manager{ nullptr };
    wl_list resources;
    wl_event_source *idle_source{ nullptr };

    QString title;
    QString app_id;
    uint32_t identifier;
    pid_t pid;

    treeland_foreign_toplevel_handle_v1 *parent{ nullptr };
    QList<treeland_foreign_toplevel_handle_v1_output>
        outputs;    // treeland_foreign_toplevel_v1_output.link
    uint32_t state; // enum treeland_foreign_toplevel_v1_state

    void set_title(const QString &title);
    void set_app_id(const QString &app_id);
    void set_pid(const pid_t pid);
    void set_identifier(uint32_t identifier);
    void output_enter(QW_NAMESPACE::QWOutput *output);
    void output_leave(QW_NAMESPACE::QWOutput *output);

    void set_maximized(bool maximized);
    void set_minimized(bool minimized);
    void set_activated(bool activated);
    void set_fullscreen(bool fullscreen);
    void set_parent(treeland_foreign_toplevel_handle_v1 *parent);

    static treeland_foreign_toplevel_handle_v1 *create(
        treeland_foreign_toplevel_manager_v1 *manager);

Q_SIGNALS:
    void beforeDestroy();
    void requestMaximize(treeland_foreign_toplevel_handle_v1_maximized_event *event);
    void requestMinimize(treeland_foreign_toplevel_handle_v1_minimized_event *event);
    void requestActivate(treeland_foreign_toplevel_handle_v1_activated_event *event);
    void requestFullscreen(treeland_foreign_toplevel_handle_v1_fullscreen_event *event);
    void requestClose();
    void rectangleChanged(treeland_foreign_toplevel_handle_v1_set_rectangle_event *event);

private:
    void update_idle_source();
    void send_state();
    void send_output(QW_NAMESPACE::QWOutput *output, bool enter);

    friend void toplevel_handle_output_bind(struct wl_listener *listener, void *data);
};

struct wlr_seat;
struct wlr_surface;

struct treeland_foreign_toplevel_handle_v1_maximized_event
{
    treeland_foreign_toplevel_handle_v1 *toplevel;
    bool maximized;
};

struct treeland_foreign_toplevel_handle_v1_minimized_event
{
    treeland_foreign_toplevel_handle_v1 *toplevel;
    bool minimized;
};

struct treeland_foreign_toplevel_handle_v1_activated_event
{
    treeland_foreign_toplevel_handle_v1 *toplevel;
    wlr_seat *seat;
};

struct treeland_foreign_toplevel_handle_v1_fullscreen_event
{
    treeland_foreign_toplevel_handle_v1 *toplevel;
    bool fullscreen;
    wlr_output *output;
};

struct treeland_foreign_toplevel_handle_v1_set_rectangle_event
{
    treeland_foreign_toplevel_handle_v1 *toplevel;
    wlr_surface *surface;
    int32_t x, y, width, height;
};

struct treeland_dock_preview_context_v1_preview_event
{
    treeland_dock_preview_context_v1 *toplevel;
    std::vector<uint32_t> toplevels;
    int32_t x, y;
    int32_t direction;
};
