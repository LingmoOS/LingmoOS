// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "x11dockhelper.h"
#include "constants.h"
#include "dockhelper.h"
#include "dockpanel.h"

#include <algorithm>
#include <xcb/res.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <QAbstractNativeEventFilter>
#include <QGuiApplication>
#include <QPointer>

namespace dock {
Q_LOGGING_CATEGORY(dockX11Log, "ocean.shell.dock.x11")

const uint16_t monitorSize = 15;
const uint32_t allWorkspace = 0xffffffff;

// TODO: use taskmanager window data
struct WindowData
{
    QRect rect;
    bool isMinimized;
    bool overlap;
    uint32_t workspace;

    WindowData()
        : isMinimized(false)
        , overlap(false)
        , workspace(0)
    {
    }
};

XcbEventFilter::XcbEventFilter(X11DockHelper *helper)
    : m_helper(helper)
    , m_currentWorkspace(0)
{
    auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    m_connection = x11Application->connection();
    xcb_ewmh_init_atoms_replies(&m_ewmh, xcb_ewmh_init_atoms(m_connection, &m_ewmh), nullptr);

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    m_rootWindow = screen->root;

    uint32_t value_list[] = { XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_FOCUS_CHANGE };

    xcb_change_window_attributes(m_connection, m_rootWindow, XCB_CW_EVENT_MASK, value_list);
    xcb_flush(m_connection);
}

bool XcbEventFilter::inTriggerArea(xcb_window_t win) const
{
    return m_helper->m_areas.contains(win);
}

void XcbEventFilter::processEnterLeave(xcb_window_t win, bool enter)
{
    if (inTriggerArea(win)) {
        if (enter) {
            m_helper->enterScreen(m_helper->m_areas.value(win)->screen());
        } else {
            m_helper->leaveScreen();
        }
    }
}

bool XcbEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    if (eventType != "xcb_generic_event_t" || m_helper.isNull())
        return false;

    auto xcb_event = reinterpret_cast<xcb_generic_event_t *>(message);
    switch (xcb_event->response_type & ~0x80) {
    case XCB_ENTER_NOTIFY: {
        auto eN = reinterpret_cast<xcb_enter_notify_event_t *>(xcb_event);
        processEnterLeave(eN->event, true);
        break;
    }
    case XCB_LEAVE_NOTIFY: {
        auto lN = reinterpret_cast<xcb_leave_notify_event_t *>(xcb_event);
        processEnterLeave(lN->event, false);
        break;
    }
    case XCB_PROPERTY_NOTIFY: {
        auto pE = reinterpret_cast<xcb_property_notify_event_t *>(xcb_event);
        if (pE->window == m_rootWindow) {
            if (pE->atom == getAtomByName("_NET_CLIENT_LIST")) {
                Q_EMIT windowClientListChanged();
            } else if (pE->atom == getAtomByName("_NET_CURRENT_DESKTOP")) {
                checkCurrentWorkspace();
            }
        } else {
            Q_EMIT windowPropertyChanged(pE->window, pE->atom);
        }
        break;
    }
    case XCB_CONFIGURE_NOTIFY: {
        auto cE = reinterpret_cast<xcb_configure_notify_event_t *>(xcb_event);
        Q_EMIT windowGeometryChanged(cE->window);
        break;
    }
    }
    return false;
}

xcb_atom_t XcbEventFilter::getAtomByName(const QString &name)
{
    xcb_atom_t ret = m_atoms.value(name, 0);
    if (!ret) {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_connection, false, name.size(), name.toStdString().c_str());
        QSharedPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(m_connection, cookie, nullptr), [](xcb_intern_atom_reply_t *reply) {
            free(reply);
        });

        if (reply) {
            m_atoms.insert(name, xcb_atom_t(reply->atom));
            ret = reply->atom;
        }
    }
    return ret;
}

QString XcbEventFilter::getNameByAtom(const xcb_atom_t &atom)
{
    auto name = m_atoms.key(atom);
    if (name.isEmpty()) {
        xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(m_connection, atom);
        QSharedPointer<xcb_get_atom_name_reply_t> reply(xcb_get_atom_name_reply(m_connection, cookie, nullptr), [](xcb_get_atom_name_reply_t *reply) {
            free(reply);
        });
        if (!reply) {
            qCWarning(dockX11Log) << "failed to get atom value for " + name;
            return name;
        }
        std::string tmp;
        tmp.assign(xcb_get_atom_name_name(reply.get()), xcb_get_atom_name_name_length(reply.get()));
        name = tmp.c_str();
        if (!name.isEmpty()) {
            m_atoms.insert(name, atom);
        }
    }
    return name;
}

QList<xcb_window_t> XcbEventFilter::getWindowClientList()
{
    QList<xcb_window_t> ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_client_list(&m_ewmh, 0);
    xcb_ewmh_get_windows_reply_t reply;
    if (xcb_ewmh_get_client_list_reply(&m_ewmh, cookie, &reply, nullptr)) {
        for (uint32_t i = 0; i < reply.windows_len; i++) {
            ret.push_back(reply.windows[i]);
        }

        xcb_ewmh_get_windows_reply_wipe(&reply);
    }

    return ret;
}

QList<xcb_atom_t> XcbEventFilter::getWindowState(const xcb_window_t &window)
{
    QList<xcb_atom_t> ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state(&m_ewmh, window);
    xcb_ewmh_get_atoms_reply_t reply;
    if (xcb_ewmh_get_wm_state_reply(&m_ewmh, cookie, &reply, nullptr)) {
        for (uint32_t i = 0; i < reply.atoms_len; i++) {
            ret.push_back(reply.atoms[i]);
        }
        xcb_ewmh_get_atoms_reply_wipe(&reply);
    }

    return ret;
}

QList<xcb_atom_t> XcbEventFilter::getWindowTypes(const xcb_window_t &window)
{
    QList<xcb_atom_t> ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type(&m_ewmh, window);
    xcb_ewmh_get_atoms_reply_t reply; // a list of Atom
    if (xcb_ewmh_get_wm_window_type_reply(&m_ewmh, cookie, &reply, nullptr)) {
        for (uint32_t i = 0; i < reply.atoms_len; i++) {
            ret.push_back(reply.atoms[i]);
        }
        xcb_ewmh_get_atoms_reply_wipe(&reply);
    }
    return ret;
}

QRect XcbEventFilter::getWindowGeometry(const xcb_window_t &window)
{
    QRect geometry;
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(m_connection, window);
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(m_connection, cookie, nullptr);
    if (geom) {
        int x = geom->x, y = geom->y;
        xcb_translate_coordinates_reply_t *trans = xcb_translate_coordinates_reply(m_connection, xcb_translate_coordinates(m_connection, window, m_rootWindow, 0, 0), nullptr);
        if (trans) {
            x = trans->dst_x;
            y = trans->dst_y;
            free(trans);
        }
        geometry.setRect(x, y, geom->width, geom->height);

        xcb_window_t dwin = getDecorativeWindow(window);
        if (dwin == 0) {
            return QRect();
        }
        xcb_get_geometry_reply_t *dgeom = xcb_get_geometry_reply(m_connection, xcb_get_geometry(m_connection, dwin), nullptr);
        if (dgeom) {
            if (geometry.x() == dgeom->x && geometry.y() == dgeom->y) {
                // 无标题栏窗口,比如 lingmo-editor, dconf-editor
                xcb_get_property_reply_t *pro = xcb_get_property_reply(m_connection, xcb_get_property(m_connection, false, window, getAtomByName("_NET_FRAME_EXTENTS"), 6, 0, 4), nullptr);
                if (pro) {
                    if (pro->format == 0) {
                        free(pro);
                        pro = xcb_get_property_reply(m_connection, xcb_get_property(m_connection, false, window, getAtomByName("_GTK_FRAME_EXTENTS"), 6, 0, 4), nullptr);
                    }
                    if (pro && pro->format == 32) {
                        uint32_t values[4];
                        memcpy(values, xcb_get_property_value(pro), sizeof(values));
                        geometry.setRect(geometry.x() + values[0], geometry.y() + values[2], geometry.width() - values[0] - values[1], geometry.height() - values[2] - values[3]);
                    }
                    free(pro);
                }
            } else {
                geometry.setRect(dgeom->x, dgeom->y, dgeom->width, dgeom->height);
            }
            free(dgeom);
        }
        free(geom);
    }
    return geometry;
}

xcb_window_t XcbEventFilter::getDecorativeWindow(const xcb_window_t &window)
{
    xcb_window_t win = window;
    for (int i = 0; i < 10; i++) {
        xcb_query_tree_reply_t *qTree = xcb_query_tree_reply(m_connection, xcb_query_tree(m_connection, win), nullptr);
        if (!qTree) {
            free(qTree);
            return 0;
        }
        if (qTree->root == qTree->parent) {
            free(qTree);
            return win;
        }
        win = qTree->parent;
        free(qTree);
    }
    return 0;
}

uint32_t XcbEventFilter::getWindowWorkspace(const xcb_window_t &window)
{
    uint32_t desktop = XCB_NONE;
    xcb_ewmh_get_wm_desktop_reply(&m_ewmh, xcb_ewmh_get_wm_desktop(&m_ewmh, window), &desktop, nullptr);
    return desktop;
}

uint32_t XcbEventFilter::getCurrentWorkspace()
{
    if (m_currentWorkspace <= 0) {
        checkCurrentWorkspace();
    }
    return m_currentWorkspace;
}

void XcbEventFilter::checkCurrentWorkspace()
{
    uint32_t desktop = XCB_NONE;
    int ret = xcb_ewmh_get_current_desktop_reply(&m_ewmh, xcb_ewmh_get_current_desktop(&m_ewmh, 0), &desktop, nullptr);
    if (ret > 0) {
        if (desktop != m_currentWorkspace) {
            m_currentWorkspace = desktop;
            Q_EMIT currentWorkspaceChanged();
        }
    } else {
        qCWarning(dockX11Log) << "failed to get current workspace: " << ret;
    }
}

bool XcbEventFilter::shouldSkip(const xcb_window_t &window)
{
    QList<xcb_atom_t> windowTypes = getWindowTypes(window);
    for (auto atom : windowTypes) {
        if (atom == getAtomByName("_NET_WM_WINDOW_TYPE_UTILITY") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_COMBO") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_DESKTOP")
            || atom == getAtomByName("_NET_WM_WINDOW_TYPE_DND") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_DOCK") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_DROPDOWN_MENU")
            || atom == getAtomByName("_NET_WM_WINDOW_TYPE_MENU") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_NOTIFICATION") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_POPUP_MENU")
            || atom == getAtomByName("_NET_WM_WINDOW_TYPE_SPLASH") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_TOOLBAR") || atom == getAtomByName("_NET_WM_WINDOW_TYPE_TOOLTIP"))
            return true;
    }

    return false;
}

void XcbEventFilter::monitorWindowChange(const xcb_window_t &window)
{
    uint32_t value_list[] = { XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE };
    xcb_change_window_attributes(m_connection, window, XCB_CW_EVENT_MASK, value_list);
};

void XcbEventFilter::setWindowState(const xcb_window_t& window, uint32_t list_len, xcb_atom_t *state)
{
    xcb_ewmh_set_wm_state(&m_ewmh, window, list_len, state);
}

X11DockHelper::X11DockHelper(DockPanel *panel)
    : DockHelper(panel)
    , m_xcbHelper(new XcbEventFilter(this))
    , m_updateDockAreaTimer(new QTimer(this))
{
    m_updateDockAreaTimer->setSingleShot(true);
    m_updateDockAreaTimer->setInterval(100);

    connect(m_updateDockAreaTimer, &QTimer::timeout, this, &X11DockHelper::updateDockArea);
    connect(panel, &DockPanel::hideModeChanged, this, &X11DockHelper::onHideModeChanged);
    connect(panel, &DockPanel::rootObjectChanged, m_updateDockAreaTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(panel, &DockPanel::positionChanged, m_updateDockAreaTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(panel, &DockPanel::dockSizeChanged, m_updateDockAreaTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(panel, &DockPanel::geometryChanged, m_updateDockAreaTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(panel, &DockPanel::showInPrimaryChanged, m_updateDockAreaTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(panel, &DockPanel::dockScreenChanged, m_updateDockAreaTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    qGuiApp->installNativeEventFilter(m_xcbHelper);
    onHideModeChanged(panel->hideMode());
}

[[nodiscard]] DockWakeUpArea *X11DockHelper::createArea(QScreen *screen)
{
    auto area = new X11DockWakeUpArea(screen, this);
    m_areas.insert(area->m_triggerWindow, area);
    return area;
}

void X11DockHelper::destroyArea(DockWakeUpArea *area)
{
    auto x11Area = static_cast<X11DockWakeUpArea *>(area);
    if (!x11Area)
        return;

    m_areas.remove(x11Area->m_triggerWindow);
    x11Area->deleteLater();
}

void X11DockHelper::onHideModeChanged(HideMode mode)
{
    // 会收到重复信号，因此每次都清理下数据
    disconnect(m_xcbHelper, nullptr, this, nullptr);
    for (auto &&data : m_windows) {
        delete data;
    }
    m_windows.clear();

    switch (mode) {
    case SmartHide: {
        onWindowClientListChanged();
        connect(m_xcbHelper, &XcbEventFilter::windowClientListChanged, this, &X11DockHelper::onWindowClientListChanged);
        connect(m_xcbHelper, &XcbEventFilter::windowPropertyChanged, this, &X11DockHelper::onWindowPropertyChanged);
        connect(m_xcbHelper, &XcbEventFilter::windowGeometryChanged, this, &X11DockHelper::onWindowGeometryChanged);
        connect(m_xcbHelper, &XcbEventFilter::currentWorkspaceChanged, this, [this]() {
            static bool updating = false;
            if (updating)
                return;

            updating = true;
            int currentWorkspace = m_xcbHelper->getCurrentWorkspace();
            for (auto &&data : m_windows) {
                if (data->overlap && (data->workspace == currentWorkspace || data->workspace == allWorkspace)) {
                    Q_EMIT isWindowOverlapChanged(isWindowOverlap());
                    updating = false;
                    return;
                }
            }
            updating = false;
        });
    } break;
    case KeepShowing:
    case KeepHidden:
        break;
    default: {

    } break;
    }
}

void X11DockHelper::onWindowClientListChanged()
{
    QList<xcb_window_t> windows = m_xcbHelper->getWindowClientList();
    for (auto &&window : windows) {
        if (!m_windows.contains(window) && !m_xcbHelper->shouldSkip(window)) {
            m_windows.insert(window, new WindowData());
            onWindowAoceand(window);
        }
    }
    for (auto it = m_windows.cbegin(); it != m_windows.cend();) {
        if (!windows.contains(it.key())) {
            delete it.value();
            it = m_windows.erase(it);
        } else {
            it++;
        }
    }
}

void X11DockHelper::onWindowAoceand(xcb_window_t window)
{
    m_xcbHelper->monitorWindowChange(window);
    onWindowPropertyChanged(window, m_xcbHelper->getAtomByName("WM_STATE"));
    onWindowGeometryChanged(window);
    onWindowWorkspaceChanged(window);
}

void X11DockHelper::onWindowPropertyChanged(xcb_window_t window, xcb_atom_t atom)
{
    if (m_windows.contains(window)) {
        if (atom == m_xcbHelper->getAtomByName("WM_STATE")) {
            m_windows[window]->isMinimized = m_xcbHelper->getWindowState(window).contains(m_xcbHelper->getAtomByName("_NET_WM_STATE_HIDDEN"));
            updateWindowHideState(window);
        } else if (atom == m_xcbHelper->getAtomByName("_NET_WM_DESKTOP")) {
            onWindowWorkspaceChanged(window);
        }
    }
}

void X11DockHelper::onWindowGeometryChanged(xcb_window_t window)
{
    if (m_windows.contains(window)) {
        m_windows[window]->rect = m_xcbHelper->getWindowGeometry(window);
        updateWindowHideState(window);
    }
}

void X11DockHelper::onWindowWorkspaceChanged(xcb_window_t window)
{
    if (m_windows.contains(window)) {
        m_windows[window]->workspace = m_xcbHelper->getWindowWorkspace(window);
    }
}

void X11DockHelper::updateWindowHideState(xcb_window_t window)
{
    if (!m_windows.contains(window)) {
        return;
    }
    WindowData *data = m_windows.value(window);
    bool oldOverlap = data->overlap;
    if (!data->isMinimized) {
        data->overlap = data->rect.intersects(m_dockArea);
    }

    if (oldOverlap != data->overlap) {
        Q_EMIT isWindowOverlapChanged(isWindowOverlap());
    }
}

void X11DockHelper::updateDockArea()
{
    QRect rect = parent()->geometry();
    uint size = parent()->dockSize();
    switch (parent()->position()) {
    case Top:
        rect.setHeight(size);
        break;
    case Bottom:
        if (rect.height() != size) {
            rect.setTop(rect.top() + rect.height() - size);
            rect.setHeight(size);
        }
        break;
    case Right:
        if (rect.width() != size) {
            rect.setLeft(rect.left() + rect.width() - size);
            rect.setWidth(size);
        }
        break;
    case Left:
        rect.setWidth(size);
        break;
    default:
        break;
    }

    // Since the position of other windows are obtained through the xcb interface without scaling
    // the rect of the dock needs to be changed to the original size of the original xcb.
    QScreen *screen = parent()->dockScreen();
    if (screen != nullptr) {
        auto screenRect = screen->geometry();
        rect.setSize(rect.size() * parent()->devicePixelRatio());
        auto x = (rect.x() - screenRect.x()) * parent()->devicePixelRatio() + screenRect.x();
        auto y = (rect.y() - screenRect.y()) * parent()->devicePixelRatio() + screenRect.y();
        rect.moveTo(x, y);
    }

    if (m_dockArea != rect) {
        m_dockArea = rect;
        for (auto it = m_windows.cbegin(); it != m_windows.cend(); ++it) {
            updateWindowHideState(it.key());
        }
    }
}

bool X11DockHelper::currentActiveWindowFullscreened()
{
    return false;
}

bool X11DockHelper::isWindowOverlap()
{
    // any widnow overlap
    bool overlap = false;
    std::for_each(m_windows.begin(), m_windows.end(), [&overlap](const auto &window) {
        return overlap |= window->overlap;
    });
    return overlap;
}

X11DockWakeUpArea::X11DockWakeUpArea(QScreen *screen, X11DockHelper *helper)
    : QObject(helper)
    , DockWakeUpArea(screen, helper)
{
    m_connection = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->connection();
    xcb_screen_t *xcbScreen = xcb_setup_roots_iterator(xcb_get_setup(m_connection)).data;
    m_rootWindow = xcbScreen->root;
    m_triggerWindow = xcb_generate_id(m_connection);
    xcb_flush(m_connection);
}

X11DockWakeUpArea::~X11DockWakeUpArea()
{
}

void X11DockWakeUpArea::open()
{
    uint32_t values_list[] = {1};
    xcb_create_window(m_connection,
                      XCB_COPY_FROM_PARENT,
                      m_triggerWindow,
                      m_rootWindow,
                      1,
                      1,
                      1,
                      1,
                      0,
                      XCB_WINDOW_CLASS_INPUT_ONLY,
                      XCB_COPY_FROM_PARENT,
                      XCB_CW_OVERRIDE_REDIRECT,
                      values_list);
    uint32_t values[] = {XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW};
    xcb_change_window_attributes(m_connection, m_triggerWindow, XCB_CW_EVENT_MASK, values);
    xcb_map_window(m_connection, m_triggerWindow);
}

void X11DockWakeUpArea::close()
{
    xcb_destroy_window(m_connection, m_triggerWindow);
}

void X11DockWakeUpArea::updateDockWakeArea(Position pos)
{
    QRect triggerArea;
    auto rect = screen()->geometry();

    // map geometry to Native
    const auto factor = m_screen->devicePixelRatio();
    switch (pos) {
    case Top: {
        triggerArea.setX(rect.left());
        triggerArea.setY(rect.top());
        triggerArea.setWidth(rect.width());
        triggerArea.setHeight(monitorSize);
        break;
    }
    case Bottom: {
        triggerArea.setX(rect.left());
        triggerArea.setY(rect.top() + (rect.height() - monitorSize + 1) * factor);
        triggerArea.setWidth(rect.width());
        triggerArea.setHeight(monitorSize);
        break;
    }
    case Left: {
        triggerArea.setX(rect.left());
        triggerArea.setY(rect.top());
        triggerArea.setWidth(monitorSize);
        triggerArea.setHeight(rect.height());
        break;
    }
    case Right: {
        triggerArea.setX(rect.left() + (rect.width() - monitorSize + 1) * factor);
        triggerArea.setY(rect.top());
        triggerArea.setWidth(monitorSize);
        triggerArea.setHeight(rect.height());
        break;
    }
    }

    triggerArea = QRect(triggerArea.topLeft(), triggerArea.size() * factor);
    int values[4] = {triggerArea.x(), triggerArea.y(), triggerArea.width(), triggerArea.height()};
    xcb_configure_window(m_connection, m_triggerWindow, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(m_connection);
}
} // namespace dock
