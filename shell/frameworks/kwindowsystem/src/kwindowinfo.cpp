/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include "kwindowsystem_debug.h"
#include "kx11extras.h"
#include "netwm.h"

#include <config-kwindowsystem.h>

#include "private/qtx11extras_p.h"
#include <QDebug>
#include <QRect>

#include "kxerrorhandler_p.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <xcb/res.h>

#include "cptr_p.h"

static bool haveXRes()
{
    static bool s_checked = false;
    static bool s_haveXRes = false;
    if (!s_checked) {
        auto cookie = xcb_res_query_version(QX11Info::connection(), XCB_RES_MAJOR_VERSION, XCB_RES_MINOR_VERSION);
        UniqueCPointer<xcb_res_query_version_reply_t> reply(xcb_res_query_version_reply(QX11Info::connection(), cookie, nullptr));
        s_haveXRes = reply != nullptr;
        s_checked = true;
    }
    return s_haveXRes;
}

class Q_DECL_HIDDEN KWindowInfoPrivate : public QSharedData
{
public:
    WId window;
    NET::Properties properties;
    NET::Properties2 properties2;

    std::unique_ptr<NETWinInfo> m_info;
    QString m_name;
    QString m_iconic_name;
    QRect m_geometry;
    QRect m_frame_geometry;
    int m_pid = -1; // real PID from XResources. Valid if > 0
    bool m_valid = false;
};

KWindowInfo::KWindowInfo(WId window, NET::Properties properties, NET::Properties2 properties2)
    : d(new KWindowInfoPrivate)
{
    d->window = window;
    d->properties = properties;
    d->properties2 = properties2;

    if (!KWindowSystem::isPlatformX11()) {
        return;
    }

    KXErrorHandler handler;
    if (properties & NET::WMVisibleIconName) {
        properties |= NET::WMIconName | NET::WMVisibleName; // force, in case it will be used as a fallback
    }
    if (properties & NET::WMVisibleName) {
        properties |= NET::WMName; // force, in case it will be used as a fallback
    }
    if (properties2 & NET::WM2ExtendedStrut) {
        properties |= NET::WMStrut; // will be used as fallback
    }
    if (properties & NET::WMWindowType) {
        properties2 |= NET::WM2TransientFor; // will be used when type is not set
    }
    if ((properties & NET::WMDesktop) && KX11Extras::mapViewport()) {
        properties |= NET::WMGeometry; // for viewports, the desktop (workspace) is determined from the geometry
    }
    properties |= NET::XAWMState; // force to get error detection for valid()
    d->m_info.reset(new NETWinInfo(QX11Info::connection(), d->window, QX11Info::appRootWindow(), properties, properties2));
    if (properties & NET::WMName) {
        if (d->m_info->name() && d->m_info->name()[0] != '\0') {
            d->m_name = QString::fromUtf8(d->m_info->name());
        } else {
            d->m_name = KX11Extras::readNameProperty(d->window, XA_WM_NAME);
        }
    }
    if (properties & NET::WMIconName) {
        if (d->m_info->iconName() && d->m_info->iconName()[0] != '\0') {
            d->m_iconic_name = QString::fromUtf8(d->m_info->iconName());
        } else {
            d->m_iconic_name = KX11Extras::readNameProperty(d->window, XA_WM_ICON_NAME);
        }
    }
    if (properties & (NET::WMGeometry | NET::WMFrameExtents)) {
        NETRect frame;
        NETRect geom;
        d->m_info->kdeGeometry(frame, geom);
        d->m_geometry.setRect(geom.pos.x, geom.pos.y, geom.size.width, geom.size.height);
        d->m_frame_geometry.setRect(frame.pos.x, frame.pos.y, frame.size.width, frame.size.height);
    }
    d->m_valid = !handler.error(false); // no sync - NETWinInfo did roundtrips

    if (haveXRes()) {
        xcb_res_client_id_spec_t specs;
        specs.client = win();
        specs.mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID;
        auto cookie = xcb_res_query_client_ids(QX11Info::connection(), 1, &specs);

        UniqueCPointer<xcb_res_query_client_ids_reply_t> reply(xcb_res_query_client_ids_reply(QX11Info::connection(), cookie, nullptr));
        if (reply && xcb_res_query_client_ids_ids_length(reply.get()) > 0) {
            uint32_t pid = *xcb_res_client_id_value_value((xcb_res_query_client_ids_ids_iterator(reply.get()).data));
            d->m_pid = pid;
        }
    }
}

KWindowInfo::KWindowInfo(const KWindowInfo &other)
    : d(other.d)
{
}

KWindowInfo::~KWindowInfo()
{
}

KWindowInfo &KWindowInfo::operator=(const KWindowInfo &other)
{
    if (d != other.d) {
        d = other.d;
    }
    return *this;
}

bool KWindowInfo::valid(bool withdrawn_is_valid) const
{
    if (!KWindowSystem::isPlatformX11()) {
        return false;
    }

    if (!d->m_valid) {
        return false;
    }
    if (!withdrawn_is_valid && mappingState() == NET::Withdrawn) {
        return false;
    }
    return true;
}

WId KWindowInfo::win() const
{
    return d->window;
}

#define CHECK_X11                                                                                                                                              \
    if (!KWindowSystem::isPlatformX11()) {                                                                                                                     \
        qCWarning(LOG_KWINDOWSYSTEM) << "KWindowInfo is only functional when running on X11";                                                                  \
        return {};                                                                                                                                             \
    }

NET::States KWindowInfo::state() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMState)) {
        qWarning() << "Pass NET::WMState to KWindowInfo";
    }
#endif
    return d->m_info->state();
}

bool KWindowInfo::hasState(NET::States s) const
{
    CHECK_X11
    return (state() & s) == s;
}

bool KWindowInfo::icccmCompliantMappingState() const
{
    CHECK_X11
    static enum { noidea, yes, no } wm_is_1_2_compliant = noidea;
    if (wm_is_1_2_compliant == noidea) {
        NETRootInfo info(QX11Info::connection(), NET::Supported, NET::Properties2(), QX11Info::appScreen());
        wm_is_1_2_compliant = info.isSupported(NET::Hidden) ? yes : no;
    }
    return wm_is_1_2_compliant == yes;
}

// see NETWM spec section 7.6
bool KWindowInfo::isMinimized() const
{
    CHECK_X11
    if (mappingState() != NET::Iconic) {
        return false;
    }
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if ((state() & NET::Hidden) != 0 && (state() & NET::Shaded) == 0) { // shaded may have NET::Hidden too
        return true;
    }
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return icccmCompliantMappingState() ? false : true;
}

NET::MappingState KWindowInfo::mappingState() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::XAWMState)) {
        qWarning() << "Pass NET::XAWMState to KWindowInfo";
    }
#endif
    return d->m_info->mappingState();
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2ExtendedStrut)) {
        qWarning() << "Pass NET::WM2ExtendedStrut to KWindowInfo";
    }
#endif
    NETExtendedStrut ext = d->m_info->extendedStrut();
    NETStrut str = d->m_info->strut();
    if (ext.left_width == 0 && ext.right_width == 0 && ext.top_width == 0 && ext.bottom_width == 0
        && (str.left != 0 || str.right != 0 || str.top != 0 || str.bottom != 0)) {
        // build extended from simple
        if (str.left != 0) {
            ext.left_width = str.left;
            ext.left_start = 0;
            ext.left_end = XDisplayHeight(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
        if (str.right != 0) {
            ext.right_width = str.right;
            ext.right_start = 0;
            ext.right_end = XDisplayHeight(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
        if (str.top != 0) {
            ext.top_width = str.top;
            ext.top_start = 0;
            ext.top_end = XDisplayWidth(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
        if (str.bottom != 0) {
            ext.bottom_width = str.bottom;
            ext.bottom_start = 0;
            ext.bottom_end = XDisplayWidth(QX11Info::display(), DefaultScreen(QX11Info::display()));
        }
    }
    return ext;
}

NET::WindowType KWindowInfo::windowType(NET::WindowTypes supported_types) const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMWindowType)) {
        qWarning() << "Pass NET::WMWindowType to KWindowInfo";
    }
#endif
    if (!d->m_info->hasWindowType()) { // fallback, per spec recommendation
        if (transientFor() != XCB_WINDOW_NONE) { // dialog
            if (supported_types & NET::DialogMask) {
                return NET::Dialog;
            }
        } else {
            if (supported_types & NET::NormalMask) {
                return NET::Normal;
            }
        }
    }
    return d->m_info->windowType(supported_types);
}

QString KWindowInfo::visibleName() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMVisibleName)) {
        qWarning() << "Pass NET::WMVisibleName to KWindowInfo";
    }
#endif
    return d->m_info->visibleName() && d->m_info->visibleName()[0] != '\0' ? QString::fromUtf8(d->m_info->visibleName()) : name();
}

QString KWindowInfo::visibleNameWithState() const
{
    CHECK_X11
    QString s = visibleName();
    if (isMinimized()) {
        s.prepend(QLatin1Char('('));
        s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfo::name() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMName)) {
        qWarning() << "Pass NET::WMName to KWindowInfo";
    }
#endif
    return d->m_name;
}

QString KWindowInfo::visibleIconName() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMVisibleIconName)) {
        qWarning() << "Pass NET::WMVisibleIconName to KWindowInfo";
    }
#endif
    if (d->m_info->visibleIconName() && d->m_info->visibleIconName()[0] != '\0') {
        return QString::fromUtf8(d->m_info->visibleIconName());
    }
    if (d->m_info->iconName() && d->m_info->iconName()[0] != '\0') {
        return QString::fromUtf8(d->m_info->iconName());
    }
    if (!d->m_iconic_name.isEmpty()) {
        return d->m_iconic_name;
    }
    return visibleName();
}

QString KWindowInfo::visibleIconNameWithState() const
{
    CHECK_X11
    QString s = visibleIconName();
    if (isMinimized()) {
        s.prepend(QLatin1Char('('));
        s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfo::iconName() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMIconName)) {
        qWarning() << "Pass NET::WMIconName to KWindowInfo";
    }
#endif
    if (d->m_info->iconName() && d->m_info->iconName()[0] != '\0') {
        return QString::fromUtf8(d->m_info->iconName());
    }
    if (!d->m_iconic_name.isEmpty()) {
        return d->m_iconic_name;
    }
    return name();
}

bool KWindowInfo::isOnCurrentDesktop() const
{
    CHECK_X11
    return isOnDesktop(KX11Extras::currentDesktop());
}

bool KWindowInfo::isOnDesktop(int desktop) const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMDesktop)) {
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
    }
#endif
    if (KX11Extras::mapViewport()) {
        if (onAllDesktops()) {
            return true;
        }
        return KX11Extras::viewportWindowToDesktop(d->m_geometry) == desktop;
    }
    return d->m_info->desktop() == desktop || d->m_info->desktop() == NET::OnAllDesktops;
}

bool KWindowInfo::onAllDesktops() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMDesktop)) {
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
    }
#endif
    if (KX11Extras::mapViewport()) {
        if (d->m_info->passedProperties() & NET::WMState) {
            return d->m_info->state() & NET::Sticky;
        }
        NETWinInfo info(QX11Info::connection(), win(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());
        return info.state() & NET::Sticky;
    }
    return d->m_info->desktop() == NET::OnAllDesktops;
}

int KWindowInfo::desktop() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMDesktop)) {
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
    }
#endif
    if (KX11Extras::mapViewport()) {
        if (onAllDesktops()) {
            return NET::OnAllDesktops;
        }
        return KX11Extras::viewportWindowToDesktop(d->m_geometry);
    }
    return d->m_info->desktop();
}

QStringList KWindowInfo::activities() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2Activities)) {
        qWarning() << "Pass NET::WM2Activities to KWindowInfo";
    }
#endif

    const QStringList result = QString::fromLatin1(d->m_info->activities()).split(QLatin1Char(','), Qt::SkipEmptyParts);

    return result.contains(QStringLiteral(KDE_ALL_ACTIVITIES_UUID)) ? QStringList() : result;
}

QRect KWindowInfo::geometry() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMGeometry)) {
        qWarning() << "Pass NET::WMGeometry to KWindowInfo";
    }
#endif
    return d->m_geometry;
}

QRect KWindowInfo::frameGeometry() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMFrameExtents)) {
        qWarning() << "Pass NET::WMFrameExtents to KWindowInfo";
    }
#endif
    return d->m_frame_geometry;
}

WId KWindowInfo::transientFor() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2TransientFor)) {
        qWarning() << "Pass NET::WM2TransientFor to KWindowInfo";
    }
#endif
    return d->m_info->transientFor();
}

WId KWindowInfo::groupLeader() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2GroupLeader)) {
        qWarning() << "Pass NET::WM2GroupLeader to KWindowInfo";
    }
#endif
    return d->m_info->groupLeader();
}

QByteArray KWindowInfo::windowClassClass() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2WindowClass)) {
        qWarning() << "Pass NET::WM2WindowClass to KWindowInfo";
    }
#endif
    return d->m_info->windowClassClass();
}

QByteArray KWindowInfo::windowClassName() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2WindowClass)) {
        qWarning() << "Pass NET::WM2WindowClass to KWindowInfo";
    }
#endif
    return d->m_info->windowClassName();
}

QByteArray KWindowInfo::windowRole() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2WindowRole)) {
        qWarning() << "Pass NET::WM2WindowRole to KWindowInfo";
    }
#endif
    return d->m_info->windowRole();
}

QByteArray KWindowInfo::clientMachine() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2ClientMachine)) {
        qWarning() << "Pass NET::WM2ClientMachine to KWindowInfo";
    }
#endif
    return d->m_info->clientMachine();
}

bool KWindowInfo::allowedActionsSupported() const
{
    CHECK_X11
    static enum { noidea, yes, no } wm_supports_allowed_actions = noidea;
    if (wm_supports_allowed_actions == noidea) {
        NETRootInfo info(QX11Info::connection(), NET::Supported, NET::Properties2(), QX11Info::appScreen());
        wm_supports_allowed_actions = info.isSupported(NET::WM2AllowedActions) ? yes : no;
    }
    return wm_supports_allowed_actions == yes;
}

bool KWindowInfo::actionSupported(NET::Action action) const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2AllowedActions)) {
        qWarning() << "Pass NET::WM2AllowedActions to KWindowInfo";
    }
#endif
    if (allowedActionsSupported()) {
        return d->m_info->allowedActions() & action;
    } else {
        return true; // no idea if it's supported or not -> pretend it is
    }
}

QByteArray KWindowInfo::desktopFileName() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2DesktopFileName)) {
        qWarning() << "Pass NET::WM2DesktopFileName to KWindowInfo";
    }
#endif
    return QByteArray(d->m_info->desktopFileName());
}

QByteArray KWindowInfo::gtkApplicationId() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2DesktopFileName)) {
        qWarning() << "Pass NET::WM2DesktopFileName to KWindowInfo";
    }
#endif
    return QByteArray(d->m_info->gtkApplicationId());
}

QByteArray KWindowInfo::applicationMenuServiceName() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2AppMenuServiceName)) {
        qWarning() << "Pass NET::WM2AppMenuServiceName to KWindowInfo";
    }
#endif
    return QByteArray(d->m_info->appMenuServiceName());
}

QByteArray KWindowInfo::applicationMenuObjectPath() const
{
    CHECK_X11
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties2() & NET::WM2AppMenuObjectPath)) {
        qWarning() << "Pass NET::WM2AppMenuObjectPath to KWindowInfo";
    }
#endif
    return QByteArray(d->m_info->appMenuObjectPath());
}

int KWindowInfo::pid() const
{
    CHECK_X11
    // If pid is found using the XRes extension use that instead.
    // It is more reliable than the app reporting it's own PID as apps
    // within an app namespace are unable to do so correctly
    if (d->m_pid > 0) {
        return d->m_pid;
    }

#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->m_info->passedProperties() & NET::WMPid)) {
        qWarning() << "Pass NET::WMPid to KWindowInfo";
    }
#endif

    return d->m_info->pid();
}
