// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WM_INFO_H
#define WM_INFO_H

#include <QList>
#include <QRect>
#include <QPixmap>
#include <QString>
#include <QByteArray>
#include <QSharedPointer>
#include <QByteArrayList>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_icccm.h>

#include <memory>
#include <map>
#include <list>

namespace core {
// x11/xcb stuff
namespace wm {

struct wm_window_t;
struct wm_window_ext_t;
struct wm_request_t;
struct wm_frame_extents_t;
struct wm_tree_t;
struct atom_meta;

using WMWindow = std::unique_ptr<struct wm_window_t>;
using WMWindowExt = std::unique_ptr<struct core::wm::wm_window_ext_t>;
using AtomMeta      = std::unique_ptr<struct atom_meta>;
using WMWindowArea  = std::unique_ptr<struct wm_window_area_t>;
using WMWId         = xcb_window_t;
using WMTree = std::unique_ptr<struct core::wm::wm_tree_t>;

struct wm_window_t {
    WMWId winId;
    pid_t   pid;      // _NET_WM_PID
    QString title {}; // _NET_WM_NAME && UTF8_STRING || WM_NAME
};

struct atom_meta {
    QByteArray name;
    xcb_atom_t atom;
};

struct wm_window_area_t {
    WMWId wid;
    pid_t pid;
    QRect rect;
};

class WMInfo
{
    enum intern_atom_type {
        kOtherAtom = -1,
        kNetNameAtom = 0,
        kUTF8StringAtom,
        kNetDesktopAtom,
        kNetWindowTypeAtom,
        kNetStateAtom,
        kNetPIDAtom,
        kNetFrameExtentsAtom,
        kNetVirtualRootsAtom,
        kStateAtom,
        kIconAtom
    };

public:
    WMInfo();
    ~WMInfo();

    // top level window (including wm frame) in top to bottom order that contains cursor
    std::list<WMWindowArea> selectWindow(const QPoint &pos) const;
    WMWId getRootWindow() const;
    std::list<WMWindowArea> getHoveredByWindowList(WMWId wid, QRect &area) const;
    bool isCursorHoveringDocks(const QPoint &pos) const;

private:
    void buildWindowTreeSchema();
    void findDockWindows();

    void initAtomCache(xcb_connection_t *conn);
    inline xcb_atom_t getAtom(xcb_connection_t *conn, xcb_intern_atom_cookie_t &cookie);
    QByteArray getAtomName(xcb_connection_t *conn, xcb_atom_t atom);

    WMWindowExt requestWindowExtInfo(xcb_connection_t *conn, xcb_window_t window);

private:
    WMTree m_tree;

    std::map<intern_atom_type, xcb_atom_t>  m_internAtomCache;
    std::map<xcb_atom_t, AtomMeta>          m_atomCache;
    std::list<WMWindowArea> m_dockWindowList;
};

} // !wm
} // namespace core
#endif // WM_INFO_H
