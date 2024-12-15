// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEW_H
#define VIEW_H

#include "Listener.h"

extern "C" {
#include <wlr/backend.h>
#define static
#include <wlr/types/wlr_scene.h>
#undef static
#include <wlr/types/wlr_xdg_shell.h>
}

class Server;

class View
{
public:
    View(Server *server, wlr_xdg_surface *xdg_surface, wl_list *list);
    ~View();

    pid_t getPid() const { return pid_; }

    wlr_surface *surface() { return xdg_surface_->surface; }

    void focusView();

private:
    void xdgSurfaceMapNotify(void *data);
    void xdgSurfaceUnmapNotify(void *data);
    void xdgSurfaceDestroyNotify(void *data);

    void xdgToplevelRequestMoveNotify(void *data);
    void xdgToplevelRequestResizeNotify(void *data);
    void xdgToplevelRequestMaximizeNotify(void *data);
    void xdgToplevelRequestFullscreenNotify(void *data);

public:
    wl_list link_;

private:
    Server *server_;
    wl_list *list_;

    wlr_xdg_surface *xdg_surface_;
    wlr_xdg_toplevel *xdg_toplevel_;
    wlr_scene_tree *scene_tree_;

    Listener<&View::xdgSurfaceMapNotify> xdg_surface_map_;
    Listener<&View::xdgSurfaceUnmapNotify> xdg_surface_unmap_;
    Listener<&View::xdgSurfaceDestroyNotify> xdg_surface_destroy_;

    Listener<&View::xdgToplevelRequestMoveNotify> xdg_toplevel_request_move_;
    Listener<&View::xdgToplevelRequestResizeNotify> xdg_toplevel_request_resize_;
    Listener<&View::xdgToplevelRequestMaximizeNotify> xdg_toplevel_request_maximize_;
    Listener<&View::xdgToplevelRequestFullscreenNotify> xdg_toplevel_request_fullscreen_;

    pid_t pid_;
};

#endif // !VIEW_H
