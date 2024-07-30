/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2007 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kx11extras.h"

// clang-format off
#include <kxerrorhandler_p.h>
#include <fixx11h.h>
#include <kxutils_p.h>
// clang-format on

#include "cptr_p.h"
#include "kwindowsystem.h"
#include "kwindowsystem_debug.h"
#include "netwm.h"
#include "kxcbevent_p.h"

#include <QAbstractNativeEventFilter>
#include <QGuiApplication>
#include <QMetaMethod>
#include <QRect>
#include <QScreen>
#include <private/qtx11extras_p.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <xcb/xcb.h>
#include <xcb/xfixes.h>

// QPoint and QSize all have handy / operators which are useful for scaling, positions and sizes for high DPI support
// QRect does not, so we create one for internal purposes within this class
inline QRect operator/(const QRect &rectangle, qreal factor)
{
    return QRect(rectangle.topLeft() / factor, rectangle.size() / factor);
}

class MainThreadInstantiator : public QObject
{
    Q_OBJECT

public:
    MainThreadInstantiator(KX11Extras::FilterInfo _what);
    Q_INVOKABLE NETEventFilter *createNETEventFilter();

private:
    KX11Extras::FilterInfo m_what;
};

class NETEventFilter : public NETRootInfo, public QAbstractNativeEventFilter
{
public:
    NETEventFilter(KX11Extras::FilterInfo _what);
    ~NETEventFilter() override;
    void activate();
    QList<WId> windows;
    QList<WId> stackingOrder;

    struct StrutData {
        StrutData(WId window_, const NETStrut &strut_, int desktop_)
            : window(window_)
            , strut(strut_)
            , desktop(desktop_)
        {
        }
        WId window;
        NETStrut strut;
        int desktop;
    };
    QList<StrutData> strutWindows;
    QList<WId> possibleStrutWindows;
    bool strutSignalConnected;
    bool compositingEnabled;
    bool haveXfixes;
    KX11Extras::FilterInfo what;
    int xfixesEventBase;
    bool mapViewport();

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

    void updateStackingOrder();
    bool removeStrutWindow(WId);

protected:
    void addClient(xcb_window_t) override;
    void removeClient(xcb_window_t) override;

private:
    bool nativeEventFilter(xcb_generic_event_t *event);
    xcb_window_t winId;
    xcb_window_t m_appRootWindow;
};

static Atom net_wm_cm;
static void create_atoms();

static inline const QRect &displayGeometry()
{
    static QRect displayGeometry;
    static bool isDirty = true;

    if (isDirty) {
        static QList<QMetaObject::Connection> connections;
        auto dirtify = [&] {
            isDirty = true;
            for (const QMetaObject::Connection &con : std::as_const(connections)) {
                QObject::disconnect(con);
            }
            connections.clear();
        };

        QObject::connect(qApp, &QGuiApplication::screenAdded, dirtify);
        QObject::connect(qApp, &QGuiApplication::screenRemoved, dirtify);
        const QList<QScreen *> screenList = QGuiApplication::screens();
        QRegion region;
        for (int i = 0; i < screenList.count(); ++i) {
            const QScreen *screen = screenList.at(i);
            connections << QObject::connect(screen, &QScreen::geometryChanged, dirtify);
            const QRect geometry = screen->geometry();
            const qreal dpr = screen->devicePixelRatio();
            region += QRect(geometry.topLeft(), geometry.size() * dpr);
        }
        displayGeometry = region.boundingRect();
        isDirty = false;
    }

    return displayGeometry;
}

static inline int displayWidth()
{
    return displayGeometry().width();
}

static inline int displayHeight()
{
    return displayGeometry().height();
}

// clang-format off
static const NET::Properties windowsProperties = NET::ClientList | NET::ClientListStacking |
                                                 NET::Supported |
                                                 NET::NumberOfDesktops |
                                                 NET::DesktopGeometry |
                                                 NET::DesktopViewport |
                                                 NET::CurrentDesktop |
                                                 NET::DesktopNames |
                                                 NET::ActiveWindow |
                                                 NET::WorkArea;
static const NET::Properties2 windowsProperties2 = NET::WM2ShowingDesktop;

// ClientList and ClientListStacking is not per-window information, but a desktop information,
// so track it even with only INFO_BASIC
static const NET::Properties desktopProperties = NET::ClientList | NET::ClientListStacking |
                                                 NET::Supported |
                                                 NET::NumberOfDesktops |
                                                 NET::DesktopGeometry |
                                                 NET::DesktopViewport |
                                                 NET::CurrentDesktop |
                                                 NET::DesktopNames |
                                                 NET::ActiveWindow |
                                                 NET::WorkArea;
static const NET::Properties2 desktopProperties2 = NET::WM2ShowingDesktop;
// clang-format on

MainThreadInstantiator::MainThreadInstantiator(KX11Extras::FilterInfo _what)
    : QObject()
    , m_what(_what)
{
}

NETEventFilter *MainThreadInstantiator::createNETEventFilter()
{
    return new NETEventFilter(m_what);
}

NETEventFilter::NETEventFilter(KX11Extras::FilterInfo _what)
    : NETRootInfo(QX11Info::connection(),
                  _what >= KX11Extras::INFO_WINDOWS ? windowsProperties : desktopProperties,
                  _what >= KX11Extras::INFO_WINDOWS ? windowsProperties2 : desktopProperties2,
                  QX11Info::appScreen(),
                  false)
    , QAbstractNativeEventFilter()
    , strutSignalConnected(false)
    , compositingEnabled(false)
    , haveXfixes(false)
    , what(_what)
    , winId(XCB_WINDOW_NONE)
    , m_appRootWindow(QX11Info::appRootWindow())
{
    QCoreApplication::instance()->installNativeEventFilter(this);

    int errorBase;
    if ((haveXfixes = XFixesQueryExtension(QX11Info::display(), &xfixesEventBase, &errorBase))) {
        create_atoms();
        winId = xcb_generate_id(QX11Info::connection());
        uint32_t values[] = {true, XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY};
        xcb_create_window(QX11Info::connection(),
                          XCB_COPY_FROM_PARENT,
                          winId,
                          m_appRootWindow,
                          0,
                          0,
                          1,
                          1,
                          0,
                          XCB_WINDOW_CLASS_INPUT_ONLY,
                          XCB_COPY_FROM_PARENT,
                          XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK,
                          values);
        XFixesSelectSelectionInput(QX11Info::display(),
                                   winId,
                                   net_wm_cm,
                                   XFixesSetSelectionOwnerNotifyMask | XFixesSelectionWindowDestroyNotifyMask | XFixesSelectionClientCloseNotifyMask);
        compositingEnabled = XGetSelectionOwner(QX11Info::display(), net_wm_cm) != None;
    }
}

NETEventFilter::~NETEventFilter()
{
    if (QX11Info::connection() && winId != XCB_WINDOW_NONE) {
        xcb_destroy_window(QX11Info::connection(), winId);
        winId = XCB_WINDOW_NONE;
    }
}

// not virtual, but it's called directly only from init()
void NETEventFilter::activate()
{
    NETRootInfo::activate();
    updateStackingOrder();
}

bool NETEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    if (eventType != "xcb_generic_event_t") {
        // only interested in XCB events of course
        return false;
    }
    return nativeEventFilter(reinterpret_cast<xcb_generic_event_t *>(message));
}

bool NETEventFilter::nativeEventFilter(xcb_generic_event_t *ev)
{
    KWindowSystem *s_q = KWindowSystem::self();
    const uint8_t eventType = ev->response_type & ~0x80;

    if (eventType == xfixesEventBase + XCB_XFIXES_SELECTION_NOTIFY) {
        xcb_xfixes_selection_notify_event_t *event = reinterpret_cast<xcb_xfixes_selection_notify_event_t *>(ev);
        if (event->window == winId) {
            bool haveOwner = event->owner != XCB_WINDOW_NONE;
            if (compositingEnabled != haveOwner) {
                compositingEnabled = haveOwner;
                Q_EMIT KX11Extras::self()->compositingChanged(compositingEnabled);
            }
            return true;
        }
        // Qt compresses XFixesSelectionNotifyEvents without caring about the actual window
        // gui/kernel/qapplication_x11.cpp
        // until that can be assumed fixed, we also react on events on the root (caused by Qts own compositing tracker)
        if (event->window == m_appRootWindow) {
            if (event->selection == net_wm_cm) {
                bool haveOwner = event->owner != XCB_WINDOW_NONE;
                if (compositingEnabled != haveOwner) {
                    compositingEnabled = haveOwner;
                    Q_EMIT KX11Extras::self()->compositingChanged(compositingEnabled);
                }
                // NOTICE this is not our event, we just randomly captured it from Qt -> pass on
                return false;
            }
        }
        return false;
    }

    xcb_window_t eventWindow = XCB_WINDOW_NONE;
    switch (eventType) {
    case XCB_CLIENT_MESSAGE:
        eventWindow = reinterpret_cast<xcb_client_message_event_t *>(ev)->window;
        break;
    case XCB_PROPERTY_NOTIFY:
        eventWindow = reinterpret_cast<xcb_property_notify_event_t *>(ev)->window;
        break;
    case XCB_CONFIGURE_NOTIFY:
        eventWindow = reinterpret_cast<xcb_configure_notify_event_t *>(ev)->window;
        break;
    }

    if (eventWindow == m_appRootWindow) {
        int old_current_desktop = currentDesktop();
        xcb_window_t old_active_window = activeWindow();
        int old_number_of_desktops = numberOfDesktops();
        bool old_showing_desktop = showingDesktop();
        NET::Properties props;
        NET::Properties2 props2;
        NETRootInfo::event(ev, &props, &props2);

        if ((props & CurrentDesktop) && currentDesktop() != old_current_desktop) {
            Q_EMIT KX11Extras::self()->currentDesktopChanged(currentDesktop());
        }
        if ((props & DesktopViewport) && mapViewport() && currentDesktop() != old_current_desktop) {
            Q_EMIT KX11Extras::self()->currentDesktopChanged(currentDesktop());
        }
        if ((props & ActiveWindow) && activeWindow() != old_active_window) {
            Q_EMIT KX11Extras::self()->activeWindowChanged(activeWindow());
        }
        if (props & DesktopNames) {
            Q_EMIT KX11Extras::self()->desktopNamesChanged();
        }
        if ((props & NumberOfDesktops) && numberOfDesktops() != old_number_of_desktops) {
            Q_EMIT KX11Extras::self()->numberOfDesktopsChanged(numberOfDesktops());
        }
        if ((props & DesktopGeometry) && mapViewport() && numberOfDesktops() != old_number_of_desktops) {
            Q_EMIT KX11Extras::self()->numberOfDesktopsChanged(numberOfDesktops());
        }
        if (props & WorkArea) {
            Q_EMIT KX11Extras::self()->workAreaChanged();
        }
        if (props & ClientListStacking) {
            updateStackingOrder();
            Q_EMIT KX11Extras::self()->stackingOrderChanged();
        }
        if ((props2 & WM2ShowingDesktop) && showingDesktop() != old_showing_desktop) {
            Q_EMIT s_q->showingDesktopChanged(showingDesktop());
        }
    } else if (windows.contains(eventWindow)) {
        NETWinInfo ni(QX11Info::connection(), eventWindow, m_appRootWindow, NET::Properties(), NET::Properties2());
        NET::Properties dirtyProperties;
        NET::Properties2 dirtyProperties2;
        ni.event(ev, &dirtyProperties, &dirtyProperties2);
        if (eventType == XCB_PROPERTY_NOTIFY) {
            xcb_property_notify_event_t *event = reinterpret_cast<xcb_property_notify_event_t *>(ev);
            if (event->atom == XCB_ATOM_WM_HINTS) {
                dirtyProperties |= NET::WMIcon; // support for old icons
            } else if (event->atom == XCB_ATOM_WM_NAME) {
                dirtyProperties |= NET::WMName; // support for old name
            } else if (event->atom == XCB_ATOM_WM_ICON_NAME) {
                dirtyProperties |= NET::WMIconName; // support for old iconic name
            }
        }
        if (mapViewport() && (dirtyProperties & (NET::WMState | NET::WMGeometry))) {
            /* geometry change -> possible viewport change
             * state change -> possible NET::Sticky change
             */
            dirtyProperties |= NET::WMDesktop;
        }
        if ((dirtyProperties & NET::WMStrut) != 0) {
            removeStrutWindow(eventWindow);
            if (!possibleStrutWindows.contains(eventWindow)) {
                possibleStrutWindows.append(eventWindow);
            }
        }
        if (dirtyProperties || dirtyProperties2) {
            Q_EMIT KX11Extras::self()->windowChanged(eventWindow, dirtyProperties, dirtyProperties2);

            if ((dirtyProperties & NET::WMStrut) != 0) {
                Q_EMIT KX11Extras::self()->strutChanged();
            }
        }
    }

    return false;
}

bool NETEventFilter::removeStrutWindow(WId w)
{
    for (QList<StrutData>::Iterator it = strutWindows.begin(); it != strutWindows.end(); ++it) {
        if ((*it).window == w) {
            strutWindows.erase(it);
            return true;
        }
    }
    return false;
}

void NETEventFilter::updateStackingOrder()
{
    stackingOrder.clear();
    for (int i = 0; i < clientListStackingCount(); i++) {
        stackingOrder.append(clientListStacking()[i]);
    }
}

void NETEventFilter::addClient(xcb_window_t w)
{
    if ((what >= KX11Extras::INFO_WINDOWS)) {
        xcb_connection_t *c = QX11Info::connection();
        UniqueCPointer<xcb_get_window_attributes_reply_t> attr(xcb_get_window_attributes_reply(c, xcb_get_window_attributes_unchecked(c, w), nullptr));

        uint32_t events = XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
        if (attr) {
            events = events | attr->your_event_mask;
        }
        xcb_change_window_attributes(c, w, XCB_CW_EVENT_MASK, &events);
    }

    bool emit_strutChanged = false;

    if (strutSignalConnected) {
        NETWinInfo info(QX11Info::connection(), w, QX11Info::appRootWindow(), NET::WMStrut | NET::WMDesktop, NET::Properties2());
        NETStrut strut = info.strut();
        if (strut.left || strut.top || strut.right || strut.bottom) {
            strutWindows.append(StrutData(w, strut, info.desktop()));
            emit_strutChanged = true;
        }
    } else {
        possibleStrutWindows.append(w);
    }

    windows.append(w);
    Q_EMIT KX11Extras::self()->windowAdded(w);
    if (emit_strutChanged) {
        Q_EMIT KX11Extras::self()->strutChanged();
    }
}

void NETEventFilter::removeClient(xcb_window_t w)
{
    bool emit_strutChanged = removeStrutWindow(w);
    if (strutSignalConnected && possibleStrutWindows.contains(w)) {
        NETWinInfo info(QX11Info::connection(), w, QX11Info::appRootWindow(), NET::WMStrut, NET::Properties2());
        NETStrut strut = info.strut();
        if (strut.left || strut.top || strut.right || strut.bottom) {
            emit_strutChanged = true;
        }
    }

    possibleStrutWindows.removeAll(w);
    windows.removeAll(w);
    Q_EMIT KX11Extras::self()->windowRemoved(w);
    if (emit_strutChanged) {
        Q_EMIT KX11Extras::self()->strutChanged();
    }
}

bool NETEventFilter::mapViewport()
{
    // compiz claims support even though it doesn't use virtual desktops :(
    //    if( isSupported( NET::DesktopViewport ) && !isSupported( NET::NumberOfDesktops ))

    // this test is duplicated in KWindowSystem::mapViewport()
    if (isSupported(NET::DesktopViewport) && numberOfDesktops(true) <= 1
        && (desktopGeometry().width > displayWidth() || desktopGeometry().height > displayHeight())) {
        return true;
    }
    return false;
}

static bool atoms_created = false;

static Atom _wm_protocols;
static Atom _wm_change_state;
static Atom kwm_utf8_string;

static void create_atoms()
{
    if (!atoms_created) {
        const int max = 20;
        Atom *atoms[max];
        const char *names[max];
        Atom atoms_return[max];
        int n = 0;

        atoms[n] = &_wm_protocols;
        names[n++] = "WM_PROTOCOLS";

        atoms[n] = &_wm_change_state;
        names[n++] = "WM_CHANGE_STATE";

        atoms[n] = &kwm_utf8_string;
        names[n++] = "UTF8_STRING";

        char net_wm_cm_name[100];
        sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", QX11Info::appScreen());
        atoms[n] = &net_wm_cm;
        names[n++] = net_wm_cm_name;

        // we need a const_cast for the shitty X API
        XInternAtoms(QX11Info::display(), const_cast<char **>(names), n, false, atoms_return);
        for (int i = 0; i < n; i++) {
            *atoms[i] = atoms_return[i];
        }

        atoms_created = True;
    }
}

#define CHECK_X11                                                                                                                                              \
    if (!KWindowSystem::isPlatformX11()) {                                                                                                                     \
        qCWarning(LOG_KWINDOWSYSTEM) << Q_FUNC_INFO << "may only be used on X11";                                                                              \
        return {};                                                                                                                                             \
    }

#define CHECK_X11_VOID                                                                                                                                         \
    if (!KWindowSystem::isPlatformX11()) {                                                                                                                     \
        qCWarning(LOG_KWINDOWSYSTEM) << Q_FUNC_INFO << "may only be used on X11";                                                                              \
        return;                                                                                                                                                \
    }

// WARNING
// you have to call s_d_func() again after calling this function if you want a valid pointer!
void KX11Extras::init(FilterInfo what)
{
    NETEventFilter *const s_d = s_d_func();

    if (what >= INFO_WINDOWS) {
        what = INFO_WINDOWS;
    } else {
        what = INFO_BASIC;
    }

    if (!s_d || s_d->what < what) {
        const bool wasCompositing = s_d ? s_d->compositingEnabled : false;
        MainThreadInstantiator instantiator(what);
        NETEventFilter *filter;
        if (instantiator.thread() == QCoreApplication::instance()->thread()) {
            filter = instantiator.createNETEventFilter();
        } else {
            // the instantiator is not in the main app thread, which implies
            // we are being called in a thread that is not the main app thread
            // so we move the instantiator to the main app thread and invoke
            // the method with a blocking call
            instantiator.moveToThread(QCoreApplication::instance()->thread());
            QMetaObject::invokeMethod(&instantiator, "createNETEventFilter", Qt::BlockingQueuedConnection, Q_RETURN_ARG(NETEventFilter *, filter));
        }
        d.reset(filter);
        d->activate();
        if (wasCompositing != s_d_func()->compositingEnabled) {
            Q_EMIT KX11Extras::self()->compositingChanged(s_d_func()->compositingEnabled);
        }
    }
}

KX11Extras *KX11Extras::self()
{
    static KX11Extras instance;
    return &instance;
}

QList<WId> KX11Extras::windows()
{
    CHECK_X11
    KX11Extras::self()->init(INFO_BASIC);
    return KX11Extras::self()->s_d_func()->windows;
}

bool KX11Extras::hasWId(WId w)
{
    CHECK_X11
    return windows().contains(w);
}

QList<WId> KX11Extras::stackingOrder()
{
    CHECK_X11
    KX11Extras::self()->init(INFO_BASIC);
    return KX11Extras::self()->s_d_func()->stackingOrder;
}

WId KX11Extras::activeWindow()
{
    CHECK_X11
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
    if (s_d) {
        return s_d->activeWindow();
    }
    NETRootInfo info(QX11Info::connection(), NET::ActiveWindow, NET::Properties2(), QX11Info::appScreen());
    return info.activeWindow();
}

void KX11Extras::activateWindow(WId win, long time)
{
    CHECK_X11_VOID
    NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::Properties2(), QX11Info::appScreen());
    if (time == 0) {
        time = QX11Info::appUserTime();
    }
    info.setActiveWindow(win, NET::FromApplication, time, QGuiApplication::focusWindow() ? QGuiApplication::focusWindow()->winId() : 0);
}

void KX11Extras::forceActiveWindow(WId win, long time)
{
    CHECK_X11_VOID
    NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::Properties2(), QX11Info::appScreen());
    if (time == 0) {
        time = QX11Info::appTime();
    }
    info.setActiveWindow(win, NET::FromTool, time, 0);
}

void KX11Extras::forceActiveWindow(QWindow *win, long time)
{
    CHECK_X11_VOID
    forceActiveWindow(win->winId(), time);
}

bool KX11Extras::compositingActive()
{
    CHECK_X11
    KX11Extras::self()->init(INFO_BASIC);
    if (KX11Extras::self()->s_d_func()->haveXfixes) {
        return KX11Extras::self()->s_d_func()->compositingEnabled;
    } else {
        create_atoms();
        return XGetSelectionOwner(QX11Info::display(), net_wm_cm);
    }
}

int KX11Extras::currentDesktop()
{
    CHECK_X11
    if (!QX11Info::connection()) {
        return 1;
    }

    if (mapViewport()) {
        KX11Extras::self()->init(INFO_BASIC);
        NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
        NETPoint p = s_d->desktopViewport(s_d->currentDesktop(true));
        return KX11Extras::self()->viewportToDesktop(QPoint(p.x, p.y) / qApp->devicePixelRatio());
    }

    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
    if (s_d) {
        return s_d->currentDesktop(true);
    }
    NETRootInfo info(QX11Info::connection(), NET::CurrentDesktop, NET::Properties2(), QX11Info::appScreen());
    return info.currentDesktop(true);
}

int KX11Extras::numberOfDesktops()
{
    CHECK_X11
    if (!QX11Info::connection()) {
        return 1;
    }

    if (mapViewport()) {
        KX11Extras::self()->init(INFO_BASIC);
        NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
        NETSize s = s_d->desktopGeometry();
        return s.width / displayWidth() * s.height / displayHeight();
    }

    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
    if (s_d) {
        return s_d->numberOfDesktops(true);
    }
    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops, NET::Properties2(), QX11Info::appScreen());
    return info.numberOfDesktops(true);
}

void KX11Extras::setCurrentDesktop(int desktop)
{
    CHECK_X11_VOID
    if (mapViewport()) {
        KX11Extras::self()->init(INFO_BASIC);
        NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
        NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::Properties2(), QX11Info::appScreen());
        QPoint pos = KX11Extras::self()->desktopToViewport(desktop, true);
        NETPoint p;
        p.x = pos.x();
        p.y = pos.y();
        info.setDesktopViewport(s_d->currentDesktop(true), p);
        return;
    }
    NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::Properties2(), QX11Info::appScreen());
    info.setCurrentDesktop(desktop, true);
}

void KX11Extras::setOnAllDesktops(WId win, bool b)
{
    CHECK_X11_VOID
    if (mapViewport()) {
        if (b) {
            setState(win, NET::Sticky);
        } else {
            clearState(win, NET::Sticky);
        }
        return;
    }
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMDesktop, NET::Properties2());
    if (b) {
        info.setDesktop(NETWinInfo::OnAllDesktops, true);
    } else if (info.desktop(true) == NETWinInfo::OnAllDesktops) {
        NETRootInfo rinfo(QX11Info::connection(), NET::CurrentDesktop, NET::Properties2(), QX11Info::appScreen());
        info.setDesktop(rinfo.currentDesktop(true), true);
    }
}

void KX11Extras::setOnDesktop(WId win, int desktop)
{
    CHECK_X11_VOID
    if (mapViewport()) {
        if (desktop == NET::OnAllDesktops) {
            return setOnAllDesktops(win, true);
        } else {
            clearState(win, NET::Sticky);
        }
        KX11Extras::self()->init(INFO_BASIC);
        QPoint p = KX11Extras::self()->desktopToViewport(desktop, false);
        Window dummy;
        int x;
        int y;
        unsigned int w;
        unsigned int h;
        unsigned int b;
        unsigned int dp;
        XGetGeometry(QX11Info::display(), win, &dummy, &x, &y, &w, &h, &b, &dp);
        // get global position
        XTranslateCoordinates(QX11Info::display(), win, QX11Info::appRootWindow(), 0, 0, &x, &y, &dummy);
        x += w / 2; // center
        y += h / 2;
        // transform to coordinates on the current "desktop"
        x = x % displayWidth();
        y = y % displayHeight();
        if (x < 0) {
            x = x + displayWidth();
        }
        if (y < 0) {
            y = y + displayHeight();
        }
        x += p.x(); // move to given "desktop"
        y += p.y();
        x -= w / 2; // from center back to topleft
        y -= h / 2;
        p = KX11Extras::self()->constrainViewportRelativePosition(QPoint(x, y));
        int flags = (NET::FromTool << 12) | (0x03 << 8) | 10; // from tool(?), x/y, static gravity
        NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
        s_d->moveResizeWindowRequest(win, flags, p.x(), p.y(), w, h);
        return;
    }
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMDesktop, NET::Properties2());
    info.setDesktop(desktop, true);
}

void KX11Extras::setOnActivities(WId win, const QStringList &activities)
{
    CHECK_X11_VOID
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::Properties(), NET::WM2Activities);
    info.setActivities(activities.join(QLatin1Char(',')).toLatin1().constData());
}

QPixmap KX11Extras::icon(WId win, int width, int height, bool scale)
{
    CHECK_X11
    return icon(win, width, height, scale, NETWM | WMHints | ClassHint | XApp);
}

QPixmap iconFromNetWinInfo(int width, int height, bool scale, int flags, NETWinInfo *info)
{
    QPixmap result;
    if (!info) {
        return result;
    }
    if (flags & KX11Extras::NETWM) {
        NETIcon ni = info->icon(width, height);
        if (ni.data && ni.size.width > 0 && ni.size.height > 0) {
            QImage img((uchar *)ni.data, (int)ni.size.width, (int)ni.size.height, QImage::Format_ARGB32);
            if (scale && width > 0 && height > 0 && img.size() != QSize(width, height) && !img.isNull()) {
                img = img.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
            if (!img.isNull()) {
                result = QPixmap::fromImage(img);
            }
            return result;
        }
    }

    if (flags & KX11Extras::WMHints) {
        xcb_pixmap_t p = info->icccmIconPixmap();
        xcb_pixmap_t p_mask = info->icccmIconPixmapMask();

        if (p != XCB_PIXMAP_NONE) {
            QPixmap pm = KXUtils::createPixmapFromHandle(info->xcbConnection(), p, p_mask);
            if (scale && width > 0 && height > 0 && !pm.isNull() //
                && (pm.width() != width || pm.height() != height)) {
                result = QPixmap::fromImage(pm.toImage().scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                result = pm;
            }
        }
    }

    // Since width can be any arbitrary size, but the icons cannot,
    // take the nearest value for best results (ignoring 22 pixel
    // icons as they don't exist for apps):
    int iconWidth;
    if (width < 24) {
        iconWidth = 16;
    } else if (width < 40) {
        iconWidth = 32;
    } else if (width < 56) {
        iconWidth = 48;
    } else if (width < 96) {
        iconWidth = 64;
    } else if (width < 192) {
        iconWidth = 128;
    } else {
        iconWidth = 256;
    }

    if (flags & KX11Extras::ClassHint) {
        // Try to load the icon from the classhint if the app didn't specify
        // its own:
        if (result.isNull()) {
            const QIcon icon = QIcon::fromTheme(QString::fromUtf8(info->windowClassClass()).toLower());
            const QPixmap pm = icon.isNull() ? QPixmap() : icon.pixmap(iconWidth, iconWidth);
            if (scale && !pm.isNull()) {
                result = QPixmap::fromImage(pm.toImage().scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                result = pm;
            }
        }
    }

    if (flags & KX11Extras::XApp) {
        // If the icon is still a null pixmap, load the icon for X applications
        // as a last resort:
        if (result.isNull()) {
            const QIcon icon = QIcon::fromTheme(QStringLiteral("xorg"));
            const QPixmap pm = icon.isNull() ? QPixmap() : icon.pixmap(iconWidth, iconWidth);
            if (scale && !pm.isNull()) {
                result = QPixmap::fromImage(pm.toImage().scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                result = pm;
            }
        }
    }
    return result;
}

QPixmap KX11Extras::icon(WId win, int width, int height, bool scale, int flags)
{
    CHECK_X11
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMIcon, NET::WM2WindowClass | NET::WM2IconPixmap);
    return iconFromNetWinInfo(width, height, scale, flags, &info);
}

QPixmap KX11Extras::icon(WId win, int width, int height, bool scale, int flags, NETWinInfo *info)
{
    // No CHECK_X11 here, kwin_wayland calls this to get the icon for XWayland windows
    width *= qGuiApp->devicePixelRatio();
    height *= qGuiApp->devicePixelRatio();

    if (info) {
        return iconFromNetWinInfo(width, height, scale, flags, info);
    }
    CHECK_X11

    NETWinInfo newInfo(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMIcon, NET::WM2WindowClass | NET::WM2IconPixmap);

    return iconFromNetWinInfo(width, height, scale, flags, &newInfo);
}

// enum values for ICCCM 4.1.2.4 and 4.1.4, defined to not depend on xcb-icccm
enum {
    _ICCCM_WM_STATE_WITHDRAWN = 0,
    _ICCCM_WM_STATE_NORMAL = 1,
    _ICCCM_WM_STATE_ICONIC = 3,
};

void KX11Extras::minimizeWindow(WId win)
{
    CHECK_X11_VOID
    create_atoms();
    // as described in ICCCM 4.1.4
    KXcbEvent<xcb_client_message_event_t> ev;
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = win;
    ev.type = _wm_change_state;
    ev.format = 32;
    ev.data.data32[0] = _ICCCM_WM_STATE_ICONIC;
    ev.data.data32[1] = 0;
    ev.data.data32[2] = 0;
    ev.data.data32[3] = 0;
    ev.data.data32[4] = 0;
    xcb_send_event(QX11Info::connection(),
                   false,
                   QX11Info::appRootWindow(),
                   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                   ev.buffer());
}

void KX11Extras::unminimizeWindow(WId win)
{
    CHECK_X11_VOID
    xcb_map_window(QX11Info::connection(), win);
}

QRect KX11Extras::workArea(int desktop)
{
    CHECK_X11
    KX11Extras::self()->init(INFO_BASIC);
    int desk = (desktop > 0 && desktop <= (int)KX11Extras::self()->s_d_func()->numberOfDesktops()) ? desktop : currentDesktop();
    if (desk <= 0) {
        return displayGeometry() / qApp->devicePixelRatio();
    }

    NETRect r = KX11Extras::self()->s_d_func()->workArea(desk);
    if (r.size.width <= 0 || r.size.height <= 0) { // not set
        return displayGeometry() / qApp->devicePixelRatio();
    }

    return QRect(r.pos.x, r.pos.y, r.size.width, r.size.height) / qApp->devicePixelRatio();
}

QRect KX11Extras::workArea(const QList<WId> &exclude, int desktop)
{
    CHECK_X11
    KX11Extras::self()->init(INFO_WINDOWS); // invalidates s_d_func's return value
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();

    QRect all = displayGeometry();
    QRect a = all;

    if (desktop == -1) {
        desktop = s_d->currentDesktop();
    }

    QList<WId>::ConstIterator it1;
    for (it1 = s_d->windows.constBegin(); it1 != s_d->windows.constEnd(); ++it1) {
        if (exclude.contains(*it1)) {
            continue;
        }

        // Kicker (very) extensively calls this function, causing hundreds of roundtrips just
        // to repeatedly find out struts of all windows. Therefore strut values for strut
        // windows are cached here.
        NETStrut strut;
        auto it2 = s_d->strutWindows.begin();
        for (; it2 != s_d->strutWindows.end(); ++it2) {
            if ((*it2).window == *it1) {
                break;
            }
        }

        if (it2 != s_d->strutWindows.end()) {
            if (!((*it2).desktop == desktop || (*it2).desktop == NETWinInfo::OnAllDesktops)) {
                continue;
            }

            strut = (*it2).strut;
        } else if (s_d->possibleStrutWindows.contains(*it1)) {
            NETWinInfo info(QX11Info::connection(), (*it1), QX11Info::appRootWindow(), NET::WMStrut | NET::WMDesktop, NET::Properties2());
            strut = info.strut();
            s_d->possibleStrutWindows.removeAll(*it1);
            s_d->strutWindows.append(NETEventFilter::StrutData(*it1, info.strut(), info.desktop()));

            if (!(info.desktop() == desktop || info.desktop() == NETWinInfo::OnAllDesktops)) {
                continue;
            }
        } else {
            continue; // not a strut window
        }

        QRect r = all;
        if (strut.left > 0) {
            r.setLeft(r.left() + (int)strut.left);
        }
        if (strut.top > 0) {
            r.setTop(r.top() + (int)strut.top);
        }
        if (strut.right > 0) {
            r.setRight(r.right() - (int)strut.right);
        }
        if (strut.bottom > 0) {
            r.setBottom(r.bottom() - (int)strut.bottom);
        }

        a = a.intersected(r);
    }
    return a / qApp->devicePixelRatio();
}

QString KX11Extras::desktopName(int desktop)
{
    CHECK_X11
    KX11Extras::self()->init(INFO_BASIC);
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();

    bool isDesktopSane = (desktop > 0 && desktop <= (int)s_d->numberOfDesktops());
    const char *name = s_d->desktopName(isDesktopSane ? desktop : currentDesktop());

    if (name && name[0]) {
        return QString::fromUtf8(name);
    }

    return KWindowSystem::tr("Desktop %1").arg(desktop);
}

void KX11Extras::setDesktopName(int desktop, const QString &name)
{
    CHECK_X11_VOID
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();

    if (desktop <= 0 || desktop > (int)numberOfDesktops()) {
        desktop = currentDesktop();
    }

    if (s_d) {
        s_d->setDesktopName(desktop, name.toUtf8().constData());
        return;
    }

    NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::Properties2(), QX11Info::appScreen());
    info.setDesktopName(desktop, name.toUtf8().constData());
}

QString KX11Extras::readNameProperty(WId win, unsigned long atom)
{
    CHECK_X11
    XTextProperty tp;
    char **text = nullptr;
    int count;
    QString result;
    if (XGetTextProperty(QX11Info::display(), win, &tp, atom) != 0 && tp.value != nullptr) {
        create_atoms();

        if (tp.encoding == kwm_utf8_string) {
            result = QString::fromUtf8((const char *)tp.value);
        } else if (XmbTextPropertyToTextList(QX11Info::display(), &tp, &text, &count) == Success && text != nullptr && count > 0) {
            result = QString::fromLocal8Bit(text[0]);
        } else if (tp.encoding == XA_STRING) {
            result = QString::fromLocal8Bit((const char *)tp.value);
        }
        if (text != nullptr) {
            XFreeStringList(text);
        }
        XFree(tp.value);
    }
    return result;
}

bool KX11Extras::mapViewport()
{
    CHECK_X11
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
    if (s_d) {
        return s_d->mapViewport();
    }

    // Handle case of not having a QGuiApplication
    if (!QX11Info::connection()) {
        return false;
    }

    // avoid creating KWindowSystemPrivate
    NETRootInfo infos(QX11Info::connection(), NET::Supported, NET::Properties2(), QX11Info::appScreen());
    if (!infos.isSupported(NET::DesktopViewport)) {
        return false;
    }
    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops | NET::CurrentDesktop | NET::DesktopGeometry, NET::Properties2(), QX11Info::appScreen());
    if (info.numberOfDesktops(true) <= 1 && (info.desktopGeometry().width > displayWidth() || info.desktopGeometry().height > displayHeight())) {
        return true;
    }
    return false;
}

int KX11Extras::viewportWindowToDesktop(const QRect &rect)
{
    CHECK_X11
    const QRect r = rect / qApp->devicePixelRatio();

    KX11Extras::self()->init(INFO_BASIC);
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
    QPoint p = r.center();
    // make absolute
    p = QPoint(p.x() + s_d->desktopViewport(s_d->currentDesktop(true)).x, p.y() + s_d->desktopViewport(s_d->currentDesktop(true)).y);
    NETSize s = s_d->desktopGeometry();
    QSize vs(displayWidth(), displayHeight());
    int xs = s.width / vs.width();
    int x = p.x() < 0 ? 0 : p.x() >= s.width ? xs - 1 : p.x() / vs.width();
    int ys = s.height / vs.height();
    int y = p.y() < 0 ? 0 : p.y() >= s.height ? ys - 1 : p.y() / vs.height();
    return y * xs + x + 1;
}

void KX11Extras::setExtendedStrut(WId win,
                                  qreal left_width,
                                  qreal left_start,
                                  qreal left_end,
                                  qreal right_width,
                                  qreal right_start,
                                  qreal right_end,
                                  qreal top_width,
                                  qreal top_start,
                                  qreal top_end,
                                  qreal bottom_width,
                                  qreal bottom_start,
                                  qreal bottom_end)
{
    CHECK_X11_VOID
    const qreal dpr = qApp->devicePixelRatio();

    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    NETExtendedStrut strut;
    strut.left_width = std::lround(left_width * dpr);
    strut.right_width = std::lround(right_width * dpr);
    strut.top_width = std::lround(top_width * dpr);
    strut.bottom_width = std::lround(bottom_width * dpr);
    strut.left_start = std::lround(left_start * dpr);
    strut.left_end = std::lround(left_end * dpr);
    strut.right_start = std::lround(right_start * dpr);
    strut.right_end = std::lround(right_end * dpr);
    strut.top_start = std::lround(top_start * dpr);
    strut.top_end = std::lround(top_end * dpr);
    strut.bottom_start = std::lround(bottom_start * dpr);
    strut.bottom_end = std::lround(bottom_end * dpr);
    info.setExtendedStrut(strut);
    NETStrut oldstrut;
    oldstrut.left = std::lround(left_width * dpr);
    oldstrut.right = std::lround(right_width * dpr);
    oldstrut.top = std::lround(top_width * dpr);
    oldstrut.bottom = std::lround(bottom_width * dpr);
    info.setStrut(oldstrut);
}

void KX11Extras::setStrut(WId win, qreal left, qreal right, qreal top, qreal bottom)
{
    CHECK_X11_VOID
    const qreal dpr = qApp->devicePixelRatio();

    int w = displayWidth();
    int h = displayHeight();
    setExtendedStrut(win,
                     std::lround(left * dpr),
                     0,
                     std::lround(left * dpr) != 0 ? w : 0,
                     std::lround(right * dpr),
                     0,
                     std::lround(right * dpr) != 0 ? w : 0,
                     std::lround(top * dpr),
                     0,
                     std::lround(top * dpr) != 0 ? h : 0,
                     std::lround(bottom * dpr),
                     0,
                     std::lround(bottom * dpr) != 0 ? h : 0);
}

// optimalization - create private only when needed and only for what is needed
void KX11Extras::connectNotify(const QMetaMethod &signal)
{
    CHECK_X11_VOID
    FilterInfo what = INFO_BASIC;
    if (signal == QMetaMethod::fromSignal(&KX11Extras::workAreaChanged)) {
        what = INFO_WINDOWS;
    } else if (signal == QMetaMethod::fromSignal(&KX11Extras::strutChanged)) {
        what = INFO_WINDOWS;
    } else if (signal == QMetaMethod::fromSignal(&KX11Extras::windowChanged)) {
        what = INFO_WINDOWS;
    }

    init(what);
    NETEventFilter *const s_d = s_d_func();
    if (!s_d->strutSignalConnected && signal == QMetaMethod::fromSignal(&KX11Extras::strutChanged)) {
        s_d->strutSignalConnected = true;
    }
    QObject::connectNotify(signal);
}

void KX11Extras::setType(WId win, NET::WindowType windowType)
{
    CHECK_X11_VOID
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    info.setWindowType(windowType);
}

void KX11Extras::setState(WId win, NET::States state)
{
    CHECK_X11_VOID
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMState, NET::Properties2());
    info.setState(state, state);
}

void KX11Extras::clearState(WId win, NET::States state)
{
    CHECK_X11_VOID
    NETWinInfo info(QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMState, NET::Properties2());
    info.setState(NET::States(), state);
}

int KX11Extras::viewportToDesktop(const QPoint &p)
{
    CHECK_X11
    KX11Extras::self()->init(INFO_BASIC);
    NETEventFilter *const s_d = KX11Extras::self()->s_d_func();
    NETSize s = s_d->desktopGeometry();
    QSize vs(displayWidth(), displayHeight());
    int xs = s.width / vs.width();
    int x = p.x() < 0 ? 0 : p.x() >= s.width ? xs - 1 : p.x() / vs.width();
    int ys = s.height / vs.height();
    int y = p.y() < 0 ? 0 : p.y() >= s.height ? ys - 1 : p.y() / vs.height();
    return y * xs + x + 1;
}

QPoint KX11Extras::constrainViewportRelativePosition(const QPoint &pos)
{
    CHECK_X11
    init(INFO_BASIC);
    NETEventFilter *const s_d = s_d_func();
    NETSize s = s_d->desktopGeometry();
    NETPoint c = s_d->desktopViewport(s_d->currentDesktop(true));
    int x = (pos.x() + c.x) % s.width;
    int y = (pos.y() + c.y) % s.height;
    if (x < 0) {
        x += s.width;
    }
    if (y < 0) {
        y += s.height;
    }
    return QPoint(x - c.x, y - c.y);
}

QPoint KX11Extras::desktopToViewport(int desktop, bool absolute)
{
    CHECK_X11
    init(INFO_BASIC);
    NETEventFilter *const s_d = s_d_func();
    NETSize s = s_d->desktopGeometry();
    QSize vs(displayWidth(), displayHeight());
    int xs = s.width / vs.width();
    int ys = s.height / vs.height();
    if (desktop <= 0 || desktop > xs * ys) {
        return QPoint(0, 0);
    }
    --desktop;
    QPoint ret(vs.width() * (desktop % xs), vs.height() * (desktop / xs));
    if (!absolute) {
        ret = QPoint(ret.x() - s_d->desktopViewport(s_d->currentDesktop(true)).x, ret.y() - s_d->desktopViewport(s_d->currentDesktop(true)).y);
        if (ret.x() >= s.width) {
            ret.setX(ret.x() - s.width);
        }
        if (ret.x() < 0) {
            ret.setX(ret.x() + s.width);
        }
        if (ret.y() >= s.height) {
            ret.setY(ret.y() - s.height);
        }
        if (ret.y() < 0) {
            ret.setY(ret.y() + s.height);
        }
    }
    return ret;
}

bool KX11Extras::showingDesktop()
{
    KX11Extras::self()->init(INFO_BASIC);
    return KX11Extras::self()->s_d_func()->showingDesktop();
}

void KX11Extras::setShowingDesktop(bool showing)
{
    NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::WM2ShowingDesktop, QX11Info::appScreen());
    info.setShowingDesktop(showing);
}

#include "kx11extras.moc"
#include "moc_kx11extras.cpp"
