// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wm_info.h"
#include "ddlog.h"
#include <QDebug>

#include <memory>

using namespace core::wm;
using namespace std;
using namespace DDLog;

enum wm_window_type_t {
    kUnknownWindowType = -1,
    kNormalWindowType = 0,
    kDesktopWindowType,
    kDockWindowType,
    kToolbarWindowType,
    kMenuWindowType,
    kUtilityWindowType,
    kSplashWindowType,
    kDialogWindowType,
    kDropdownMenuWindowType,
    kPopupMenuWindowType,
    kTooltipWindowType,
    kNotificationWindowType,
    kComboWindowType,
    kDNDWindowType
};

enum wm_state_t {
    kUnknownState = -1,
    kModalState = 0,
    kStickyState,
    kMaximizedVertState,
    kMaximizedHorzState,
    kShadedState,
    kSkipTaskbarState,
    kSkipPagerState,
    kHiddenState,
    kFullScreenState,
    kAboveState,
    kBelowState,
    kDemandsAttentionState,
    kFocusedState
};

enum wm_window_class_t {
    kUnknownClass = -1,
    kInputOutputClass = XCB_WINDOW_CLASS_INPUT_OUTPUT,
    kInputOnlyClass = XCB_WINDOW_CLASS_INPUT_ONLY
};

enum wm_window_map_state_t {
    kUnknownMapState = -1,
    kUnMappedState = XCB_MAP_STATE_UNMAPPED,
    kUnViewableState = XCB_MAP_STATE_UNVIEWABLE,
    kViewableState = XCB_MAP_STATE_VIEWABLE
};

struct core::wm::wm_request_t
{
    xcb_get_property_cookie_t netNameCookie;
    xcb_get_property_cookie_t nameCookie;
    xcb_get_geometry_cookie_t geomCookie;
    xcb_translate_coordinates_cookie_t transCoordsCookie;
    xcb_get_window_attributes_cookie_t attrCookie;
    xcb_get_property_cookie_t desktopCookie;
    xcb_get_property_cookie_t windowTypeCookie;
    xcb_get_property_cookie_t stateCookie;
    xcb_get_property_cookie_t pidCookie;
    xcb_get_property_cookie_t frameExtentsCookie;
    xcb_query_tree_cookie_t treeCookie;
};

struct core::wm::wm_frame_extents_t
{
    uint left;
    uint right;
    uint top;
    uint bottom;
};

struct core::wm::wm_window_ext_t
{
    WMWId windowId;   // window id
    WMWId parent {};   // window tree schema
    QList<WMWId> children {};

    pid_t pid;   // _NET_WM_PID
    int desktop {};   // desktop
    enum wm_window_class_t wclass {};   // class: InputOutput || InputOnly && win attrs: XCB_WINDOW_CLASS_INPUT_OUTPUT || XCB_WINDOW_CLASS_INPUT_ONLY
    enum wm_window_map_state_t map_state {};   // map state: unmapped || unviewable || viewable && win attrs: XCB_MAP_STATE_UNMAPPED || XCB_MAP_STATE_UNVIEWABLE || XCB_MAP_STATE_VIEWABLE
    QList<enum wm_state_t> states {};   // window states && // _NET_WM_STATE
    QList<enum wm_window_type_t> types {};   // window types (in order of preference) && _NET_WM_WINDOW_TYPE
    struct wm_frame_extents_t extents
    {
    };   // window extents && //_NET_FRAME_EXTENTS
    QRect rect {};   // geometry

    std::unique_ptr<wm_request_t> request;
};

// tree schema in bottom to top stacking order
struct core::wm::wm_tree_t
{
    struct wm_window_ext_t *root;   // root window
    std::map<WMWId, WMWindowExt> cache;   // [windowId : window extended info] mapping
};

struct XReplyDeleter
{
    void operator()(void *p)
    {
        free(p);
    }
};
using XGetPropertyReply = std::unique_ptr<xcb_get_property_reply_t, XReplyDeleter>;
using XGetGeometryReply = std::unique_ptr<xcb_get_geometry_reply_t, XReplyDeleter>;
using XGetWindowAttributeReply = std::unique_ptr<xcb_get_window_attributes_reply_t, XReplyDeleter>;
using XQueryTreeReply = std::unique_ptr<xcb_query_tree_reply_t, XReplyDeleter>;
using XTransCoordsReply = std::unique_ptr<xcb_translate_coordinates_reply_t, XReplyDeleter>;
using XGetAtomNameReply = std::unique_ptr<xcb_get_atom_name_reply_t, XReplyDeleter>;
using XInternAtomReply = std::unique_ptr<xcb_intern_atom_reply_t, XReplyDeleter>;

struct XDisconnector
{
    void operator()(xcb_connection_t *c)
    {
        xcb_disconnect(c);
    }
};
using XConnection = std::unique_ptr<xcb_connection_t, XDisconnector>;

WMInfo::WMInfo()
{
    buildWindowTreeSchema();
    findDockWindows();
}

WMInfo::~WMInfo()
{
}

std::list<WMWindowArea> WMInfo::selectWindow(const QPoint &pos) const
{

    std::list<WMWindowArea> walist;

    std::function<void(const struct wm_window_ext_t *)> scan_tree;
    scan_tree = [&](const struct wm_window_ext_t *parent) {
        for (auto &childWindowId : parent->children) {
            auto &child = m_tree->cache[childWindowId];

            // check child first (top => bottom)
            if (child->children.length() > 0)
                scan_tree(child.get());

            // map state
            if (child->map_state != kViewableState)
                continue;

            // window class
            if (child->wclass != kInputOutputClass)
                continue;

            if (child->pid == -1)
                continue;

            // window type
            if (child->types.startsWith(kDockWindowType)
                || child->types.startsWith(kDesktopWindowType))
                continue;

            // window state
            if (child->states.contains(kHiddenState))
                continue;

            WMWindowArea warea(new struct wm_window_area_t());

            // rect & extents
            QRect rect { child->rect };
            rect = rect.marginsAdded({ int(child->extents.left),
                                       int(child->extents.top),
                                       int(child->extents.right),
                                       int(child->extents.bottom) });

            if (!rect.contains(pos))
                continue;

            // window adjusted rect (including bounding frame)
            warea->rect = rect;
            // pid
            warea->pid = child->pid;

            warea->wid = child->windowId;

            walist.push_back(std::move(warea));
        }
    };

    scan_tree(m_tree->root);

    return walist;
}

WMWId WMInfo::getRootWindow() const
{
    if (m_tree && m_tree->root) {
        return m_tree->root->windowId;
    }
    return 0;
}

bool WMInfo::isCursorHoveringDocks(const QPoint &pos) const
{
    for (auto &dock : m_dockWindowList) {
        if (dock->rect.contains(pos)) {
            return true;
        }
    }

    return false;
}

std::list<WMWindowArea> WMInfo::getHoveredByWindowList(WMWId wid, QRect &area) const
{
    std::list<WMWindowArea> list {};
    bool done { false };

    std::function<void(const struct wm_window_ext_t *, const QRect &)> scan_tree;
    scan_tree = [&](const struct wm_window_ext_t *parent, const QRect &area) {
        for (auto &childWindowId : parent->children) {
            if (childWindowId == wid) {
                done = true;
            }

            if (done) {
                return;
            }

            auto &child = m_tree->cache[childWindowId];

            // check child first (top => bottom)
            if (child->children.length() > 0)
                scan_tree(child.get(), area);

            // map state
            if (child->map_state != kViewableState)
                continue;

            // window class
            if (child->wclass != kInputOutputClass)
                continue;

            if (child->pid == -1)
                continue;

            // window type (dock type should exclude from this)
            if (child->types.startsWith(kDesktopWindowType))
                continue;

            // window state
            if (child->states.contains(kHiddenState))
                continue;

            // adjust child rect with frame extents
            auto adjusted = child->rect;
            adjusted = adjusted.marginsAdded(QMargins {
                    int(child->extents.left),
                    int(child->extents.top),
                    int(child->extents.right),
                    int(child->extents.bottom) });

            // translate child geom
            if (!adjusted.intersects(area))
                continue;

            WMWindowArea warea(new struct wm_window_area_t());
            warea->rect = adjusted;
            warea->pid = child->pid;
            warea->wid = child->windowId;

            list.push_back(std::move(warea));
        }
    };

    Q_ASSERT(m_tree->root != nullptr);
    scan_tree(m_tree->root, area);

    return list;
}

void WMInfo::buildWindowTreeSchema()
{
    int screenNumber {};
    int err {};

    XConnection connection(xcb_connect(nullptr, &screenNumber));
    auto *conn = connection.get();
    err = xcb_connection_has_error(conn);
    if (err) {
        qCDebug(app) << "Unable to connect to X server";
        return;
    }

    initAtomCache(conn);

    xcb_screen_t *screen {};
    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    int screenCount = xcb_setup_roots_length(setup);
    if (screenCount < screenNumber) {
        qCDebug(app) << QString("Unable to access to screen %1, max is %2").arg(screenNumber).arg(screenCount - 1);
        return;
    }
    // get the screen we want from a list of screens in linked list structure
    for (int i = 0; i < screenNumber; i++)
        xcb_screen_next(&iter);
    screen = iter.data;

    xcb_window_t root = screen->root;
    auto rootWinExtInfo = requestWindowExtInfo(conn, root);

    WMTree tree(new struct wm_tree_t());
    m_tree = std::move(tree);
    m_tree->root = rootWinExtInfo.get();
    m_tree->cache[rootWinExtInfo->windowId] = std::move(rootWinExtInfo);

    std::function<void(xcb_window_t)> build_tree;
    build_tree = [&](xcb_window_t parent) {
        if (m_tree->cache.find(parent) == m_tree->cache.end())
            return;

        auto &clist = m_tree->cache[parent]->children;
        for (auto &child : clist) {
            auto winfo = requestWindowExtInfo(conn, child);
            m_tree->cache[child] = std::move(winfo);
            build_tree(child);
        }
    };

    build_tree(root);
}

void WMInfo::findDockWindows()
{
    m_dockWindowList.clear();

    std::function<void(const struct wm_window_ext_t *)> scan_tree;
    scan_tree = [&](const struct wm_window_ext_t *parent) {
        for (auto &childWindowId : parent->children) {
            auto &child = m_tree->cache[childWindowId];

            // check child first (top => bottom)
            if (child->children.length() > 0)
                scan_tree(child.get());

            // map state
            if (child->map_state != kViewableState)
                continue;

            // window class
            if (child->wclass != kInputOutputClass)
                continue;

            if (child->pid == -1)
                continue;

            // window state
            if (child->states.contains(kHiddenState))
                continue;

            // window type (dock type should exclude from this)
            if (!child->types.startsWith(kDockWindowType))
                continue;

            WMWindowArea warea(new struct wm_window_area_t());
            warea->rect = child->rect;
            warea->pid = child->pid;
            warea->wid = child->windowId;

            m_dockWindowList.push_back(std::move(warea));
        }
    };

    Q_ASSERT(m_tree->root != nullptr);
    scan_tree(m_tree->root);
}

void WMInfo::initAtomCache(xcb_connection_t *conn)
{
    QByteArray buffer {};

    // _NET_WM_NAME
    AtomMeta nameAtomMeta(new atom_meta {});
    nameAtomMeta->name = "_NET_WM_NAME";
    buffer = nameAtomMeta->name;
    auto nameAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    // UTF8_STRING
    AtomMeta utf8StringAtomMeta(new atom_meta {});
    utf8StringAtomMeta->name = "UTF8_STRING";
    buffer = utf8StringAtomMeta->name;
    auto utf8StringAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    // _NET_WM_DESKTOP
    AtomMeta desktopAtomMeta(new atom_meta {});
    desktopAtomMeta->name = "_NET_WM_DESKTOP";
    buffer = desktopAtomMeta->name;
    auto desktopAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    // _NET_WM_WINDOW_TYPE
    AtomMeta windowTypeAtomMeta(new atom_meta {});
    windowTypeAtomMeta->name = "_NET_WM_WINDOW_TYPE";
    buffer = windowTypeAtomMeta->name;
    auto windowTypeAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    // _NET_WM_STATE
    AtomMeta stateAtomMeta(new atom_meta {});
    stateAtomMeta->name = "_NET_WM_STATE";
    buffer = stateAtomMeta->name;
    auto stateAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    // _NET_WM_PID
    AtomMeta pidAtomMeta(new atom_meta {});
    pidAtomMeta->name = "_NET_WM_PID";
    buffer = pidAtomMeta->name;
    auto pidAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    // _NET_FRAME_EXTENTS
    AtomMeta frameExtentsAtomMeta(new atom_meta {});
    frameExtentsAtomMeta->name = "_NET_FRAME_EXTENTS";
    buffer = frameExtentsAtomMeta->name;
    auto frameExtentsAtomCookie = xcb_intern_atom(conn, false, uint16_t(buffer.length()), buffer.data());

    auto nameAtom = getAtom(conn, nameAtomCookie);
    nameAtomMeta->atom = nameAtom;
    auto utf8StringAtom = getAtom(conn, utf8StringAtomCookie);
    utf8StringAtomMeta->atom = utf8StringAtom;
    auto desktopAtom = getAtom(conn, desktopAtomCookie);
    desktopAtomMeta->atom = desktopAtom;
    auto windowTypeAtom = getAtom(conn, windowTypeAtomCookie);
    windowTypeAtomMeta->atom = windowTypeAtom;
    auto stateAtom = getAtom(conn, stateAtomCookie);
    stateAtomMeta->atom = stateAtom;
    auto pidAtom = getAtom(conn, pidAtomCookie);
    pidAtomMeta->atom = pidAtom;
    auto frameExtentsAtom = getAtom(conn, frameExtentsAtomCookie);
    frameExtentsAtomMeta->atom = frameExtentsAtom;

    m_internAtomCache[kNetNameAtom] = nameAtom;
    m_internAtomCache[kUTF8StringAtom] = utf8StringAtom;
    m_internAtomCache[kNetDesktopAtom] = desktopAtom;
    m_internAtomCache[kNetWindowTypeAtom] = windowTypeAtom;
    m_internAtomCache[kNetStateAtom] = stateAtom;
    m_internAtomCache[kNetPIDAtom] = pidAtom;
    m_internAtomCache[kNetFrameExtentsAtom] = frameExtentsAtom;

    m_atomCache[nameAtom] = std::move(nameAtomMeta);
    m_atomCache[utf8StringAtom] = std::move(utf8StringAtomMeta);
    m_atomCache[desktopAtom] = std::move(desktopAtomMeta);
    m_atomCache[windowTypeAtom] = std::move(windowTypeAtomMeta);
    m_atomCache[stateAtom] = std::move(stateAtomMeta);
    m_atomCache[pidAtom] = std::move(pidAtomMeta);
    m_atomCache[frameExtentsAtom] = std::move(frameExtentsAtomMeta);
}

xcb_atom_t WMInfo::getAtom(xcb_connection_t *conn, xcb_intern_atom_cookie_t &cookie)
{
    xcb_atom_t atom = XCB_ATOM_NONE;
    XInternAtomReply reply(xcb_intern_atom_reply(conn, cookie, nullptr));
    if (reply) {
        atom = reply->atom;
    }
    return atom;
}

QByteArray WMInfo::getAtomName(xcb_connection_t *conn, xcb_atom_t atom)
{
    if (m_atomCache.find(atom) == m_atomCache.end()) {
        AtomMeta meta(new struct atom_meta());
        meta->atom = atom;

        auto cookie = xcb_get_atom_name(conn, atom);
        XGetAtomNameReply reply(xcb_get_atom_name_reply(conn, cookie, nullptr));
        if (reply) {
            auto len = xcb_get_atom_name_name_length(reply.get());
            auto name = xcb_get_atom_name_name(reply.get());
            meta->name = QByteArray { name, len };
        }
        m_atomCache[atom] = std::move(meta);
    }

    return m_atomCache[atom]->name;
}

WMWindowExt WMInfo::requestWindowExtInfo(xcb_connection_t *conn, xcb_window_t window)
{
    WMWindowExt winfo(new wm_window_ext_t {});
    std::unique_ptr<struct wm_request_t> req(new wm_request_t {});
    winfo->request = std::move(req);

    auto nameAtom = m_internAtomCache[kNetNameAtom];
    auto utf8StringAtom = m_internAtomCache[kUTF8StringAtom];
    auto desktopAtom = m_internAtomCache[kNetDesktopAtom];
    auto windowTypeAtom = m_internAtomCache[kNetWindowTypeAtom];
    auto stateAtom = m_internAtomCache[kNetStateAtom];
    auto pidAtom = m_internAtomCache[kNetPIDAtom];
    auto frameExtentsAtom = m_internAtomCache[kNetFrameExtentsAtom];

    auto discard_reply = [&conn, &winfo]() {
        unsigned int seq {};

        seq = winfo->request->netNameCookie.sequence;
        if (seq)
            xcb_discard_reply(conn, seq);

        seq = winfo->request->nameCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->attrCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->desktopCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->windowTypeCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->stateCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->pidCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->frameExtentsCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }

        seq = winfo->request->treeCookie.sequence;
        if (seq) {
            xcb_discard_reply(conn, seq);
        }
    };

    // net name
    auto netNameCookie = xcb_get_property(conn, 0, window, nameAtom, utf8StringAtom, 0, BUFSIZ);
    winfo->request->netNameCookie = netNameCookie;
    // name
    auto nameCookie = xcb_icccm_get_wm_name(conn, window);
    winfo->request->nameCookie = nameCookie;
    // geometry
    auto geomCookie = xcb_get_geometry(conn, window);
    winfo->request->geomCookie = geomCookie;
    // window attributes
    auto attrCookie = xcb_get_window_attributes(conn, window);
    winfo->request->attrCookie = attrCookie;
    // desktop
    auto desktopCookie = xcb_get_property(conn, 0, window, desktopAtom, XCB_ATOM_CARDINAL, 0, 4);
    winfo->request->desktopCookie = desktopCookie;
    // window type
    auto windowTypeCookie = xcb_get_property(conn, 0, window, windowTypeAtom, XCB_ATOM_ATOM, 0, BUFSIZ);
    winfo->request->windowTypeCookie = windowTypeCookie;
    // states
    auto stateCookie = xcb_get_property(conn, 0, window, stateAtom, XCB_ATOM_ATOM, 0, BUFSIZ);
    winfo->request->stateCookie = stateCookie;
    // pid
    auto pidCookie = xcb_get_property(conn, 0, window, pidAtom, XCB_ATOM_CARDINAL, 0, BUFSIZ);
    winfo->request->pidCookie = pidCookie;
    // frame extents
    auto frameExtentsCookie = xcb_get_property(conn, 0, window, frameExtentsAtom, XCB_ATOM_CARDINAL, 0, 4 * 4);
    winfo->request->frameExtentsCookie = frameExtentsCookie;
    // window tree info
    auto treeCookie = xcb_query_tree(conn, window);
    winfo->request->treeCookie = treeCookie;

    // flush requests to the x server
    xcb_flush(conn);

    XGetGeometryReply geomReply(xcb_get_geometry_reply(conn, geomCookie, nullptr));
    if (geomReply) {
        // translate coordinates cookie
        auto tcc = xcb_translate_coordinates(conn, window, geomReply->root, 0, 0);
        winfo->request->transCoordsCookie = tcc;
    } else {
        discard_reply();
        return {};
    }

    // window id
    winfo->windowId = window;
    // desktop
    XGetPropertyReply desktopReply(xcb_get_property_reply(conn, desktopCookie, nullptr));
    if (desktopReply && desktopReply->type != XCB_NONE) {
        auto *desktop = reinterpret_cast<uint *>(xcb_get_property_value(desktopReply.get()));
        Q_ASSERT(desktop != nullptr);
        winfo->desktop = int(*desktop);
    } else {
        winfo->desktop = -1;
    }
    // window attributes
    XGetWindowAttributeReply attrReply(xcb_get_window_attributes_reply(conn, attrCookie, nullptr));
    if (attrReply) {
        // map state
        switch (attrReply->map_state) {
        case XCB_MAP_STATE_UNMAPPED:
            winfo->map_state = kUnMappedState;
            break;
        case XCB_MAP_STATE_UNVIEWABLE:
            winfo->map_state = kUnViewableState;
            break;
        case XCB_MAP_STATE_VIEWABLE:
            winfo->map_state = kViewableState;
            break;
        default:
            winfo->map_state = kUnknownMapState;
        }

        // class
        switch (attrReply->_class) {
        case XCB_WINDOW_CLASS_INPUT_OUTPUT:
            winfo->wclass = kInputOutputClass;
            break;
        case XCB_WINDOW_CLASS_INPUT_ONLY:
            winfo->wclass = kInputOnlyClass;
            break;
        default:
            winfo->wclass = kUnknownClass;
        }
    } else {
        winfo->map_state = kUnknownMapState;
        winfo->wclass = kUnknownClass;
    }
    // rect
    XTransCoordsReply transCoordsReply(xcb_translate_coordinates_reply(conn, winfo->request->transCoordsCookie, nullptr));
    if (transCoordsReply) {
        winfo->rect = {
            transCoordsReply->dst_x - geomReply->border_width,
            transCoordsReply->dst_y - geomReply->border_width,
            geomReply->width,
            geomReply->height
        };
    } else {
        winfo->rect = {};
    }
    // window type
    XGetPropertyReply windowTypeReply(xcb_get_property_reply(conn, windowTypeCookie, nullptr));
    if (windowTypeReply && windowTypeReply->type != XCB_NONE && windowTypeReply->value_len > 0) {
        auto *atoms = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(windowTypeReply.get()));
        Q_ASSERT(atoms != nullptr);
        for (uint32_t i = 0; i < windowTypeReply->value_len; ++i) {
            auto atomName = getAtomName(conn, atoms[i]);
            if (atomName == "_NET_WM_WINDOW_TYPE_NORMAL") {
                winfo->types << kNormalWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_DESKTOP") {
                winfo->types << kDesktopWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_DOCK") {
                winfo->types << kDockWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_TOOLBAR") {
                winfo->types << kToolbarWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_MENU") {
                winfo->types << kMenuWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_UTILITY") {
                winfo->types << kUtilityWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_SPLASH") {
                winfo->types << kSplashWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_DIALOG") {
                winfo->types << kDialogWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU") {
                winfo->types << kDropdownMenuWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_POPUP_MENU") {
                winfo->types << kPopupMenuWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_TOOLTIP") {
                winfo->types << kTooltipWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_NOTIFICATION") {
                winfo->types << kNotificationWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_COMBO") {
                winfo->types << kComboWindowType;
            } else if (atomName == "_NET_WM_WINDOW_TYPE_DND") {
                winfo->types << kDNDWindowType;
            } else {
                winfo->types << kUnknownWindowType;
            }   // !if
        }   // !for
    }
    // state
    XGetPropertyReply stateReply(xcb_get_property_reply(conn, stateCookie, nullptr));
    if (stateReply && stateReply->type != XCB_NONE && stateReply->value_len > 0) {
        auto *atoms = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(stateReply.get()));
        Q_ASSERT(atoms != nullptr);
        for (uint32_t i = 0; i < stateReply->value_len; ++i) {
            auto atomName = getAtomName(conn, atoms[i]);
            if (atomName == "_NET_WM_STATE_MODAL") {
                winfo->states << kModalState;
            } else if (atomName == "_NET_WM_STATE_STICKY") {
                winfo->states << kStickyState;
            } else if (atomName == "_NET_WM_STATE_MAXIMIZED_VERT") {
                winfo->states << kMaximizedVertState;
            } else if (atomName == "_NET_WM_STATE_MAXIMIZED_HORZ") {
                winfo->states << kMaximizedHorzState;
            } else if (atomName == "_NET_WM_STATE_SHADED") {
                winfo->states << kShadedState;
            } else if (atomName == "_NET_WM_STATE_SKIP_TASKBAR") {
                winfo->states << kSkipTaskbarState;
            } else if (atomName == "_NET_WM_STATE_SKIP_PAGER") {
                winfo->states << kSkipPagerState;
            } else if (atomName == "_NET_WM_STATE_HIDDEN") {
                winfo->states << kHiddenState;
            } else if (atomName == "_NET_WM_STATE_FULLSCREEN") {
                winfo->states << kFullScreenState;
            } else if (atomName == "_NET_WM_STATE_ABOVE") {
                winfo->states << kAboveState;
            } else if (atomName == "_NET_WM_STATE_BELOW") {
                winfo->states << kBelowState;
            } else if (atomName == "_NET_WM_STATE_DEMANDS_ATTENTION") {
                winfo->states << kDemandsAttentionState;
            } else {
                winfo->states << kUnknownState;
            }   // !if
        }   // !for
    }
    // PID
    XGetPropertyReply pidReply(xcb_get_property_reply(conn, pidCookie, nullptr));
    if (pidReply && pidReply->type == XCB_ATOM_CARDINAL) {
        auto *pid = reinterpret_cast<pid_t *>(xcb_get_property_value(pidReply.get()));
        winfo->pid = *pid;
    } else {
        winfo->pid = -1;
    }
    // frame extends
    XGetPropertyReply frameExtentsReply(xcb_get_property_reply(conn, frameExtentsCookie, nullptr));
    if (frameExtentsReply && frameExtentsReply->type == XCB_ATOM_CARDINAL && frameExtentsReply->value_len == 4) {
        auto *frameExtents = reinterpret_cast<uint *>(xcb_get_property_value(frameExtentsReply.get()));
        winfo->extents.left = frameExtents[0];
        winfo->extents.right = frameExtents[1];
        winfo->extents.top = frameExtents[2];
        winfo->extents.bottom = frameExtents[3];
    } else {
        winfo->extents = {};
    }
    // child windows in top to bottom order
    XQueryTreeReply treeReply(xcb_query_tree_reply(conn, winfo->request->treeCookie, nullptr));
    if (treeReply) {
        winfo->parent = treeReply->parent;
        auto clen = xcb_query_tree_children_length(treeReply.get());
        auto *clist = xcb_query_tree_children(treeReply.get());
        for (auto i = clen - 1; i >= 0; i--)
            winfo->children << clist[i];
    } else {
        winfo->parent = 0;
    }

    return winfo;
}
