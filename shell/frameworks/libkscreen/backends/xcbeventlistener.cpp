/*
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "xcbeventlistener.h"

#include "../xcbwrapper.h"

#include <QGuiApplication>
#include <QtGui/private/qtx11extras_p.h>

Q_LOGGING_CATEGORY(KSCREEN_XCB_HELPER, "kscreen.xcb.helper")

XCBEventListener::XCBEventListener()
    : m_isRandrPresent(false)
    , m_randrBase(0)
    , m_randrErrorBase(0)
    , m_majorOpcode(0)
    , m_versionMajor(0)
    , m_versionMinor(0)
    , m_window(0)
{
    xcb_connection_t *c = QX11Info::connection();
    xcb_prefetch_extension_data(c, &xcb_randr_id);
    auto cookie = xcb_randr_query_version(c, XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);
    const auto *queryExtension = xcb_get_extension_data(c, &xcb_randr_id);
    if (!queryExtension) {
        qCDebug(KSCREEN_XCB_HELPER) << "Fail to query for xrandr extension";
        return;
    }
    if (!queryExtension->present) {
        qCDebug(KSCREEN_XCB_HELPER) << "XRandR extension is not present at all";
        return;
    }

    m_isRandrPresent = queryExtension->present;
    m_randrBase = queryExtension->first_event;
    m_randrErrorBase = queryExtension->first_error;
    m_majorOpcode = queryExtension->major_opcode;

    xcb_generic_error_t *error = nullptr;
    auto *versionReply = xcb_randr_query_version_reply(c, cookie, &error);
    Q_ASSERT_X(versionReply, "xrandrxcbhelper", "Query to fetch xrandr version failed");
    if (error) {
        qFatal("Error while querying for xrandr version: %d", error->error_code);
    }
    m_versionMajor = versionReply->major_version;
    m_versionMinor = versionReply->minor_version;
    free(versionReply);

    qCDebug(KSCREEN_XCB_HELPER).nospace() << "Detected XRandR " << m_versionMajor << "." << m_versionMinor;
    qCDebug(KSCREEN_XCB_HELPER) << "Event Base: " << m_randrBase;
    qCDebug(KSCREEN_XCB_HELPER) << "Event Error: " << m_randrErrorBase;

    uint32_t rWindow = QX11Info::appRootWindow();
    m_window = xcb_generate_id(c);
    xcb_create_window(c, XCB_COPY_FROM_PARENT, m_window, rWindow, 0, 0, 1, 1, 0, XCB_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT, 0, nullptr);

    xcb_randr_select_input(c,
                           m_window,
                           XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE | XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE | XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE
                               | XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY);

    qApp->installNativeEventFilter(this);
}

XCBEventListener::~XCBEventListener()
{
    if (m_window && QX11Info::connection()) {
        xcb_destroy_window(QX11Info::connection(), m_window);
    }
}

QString XCBEventListener::rotationToString(xcb_randr_rotation_t rotation)
{
    switch (rotation) {
    case XCB_RANDR_ROTATION_ROTATE_0:
        return QStringLiteral("Rotate_0");
    case XCB_RANDR_ROTATION_ROTATE_90:
        return QStringLiteral("Rotate_90");
    case XCB_RANDR_ROTATION_ROTATE_180:
        return QStringLiteral("Rotate_180");
    case XCB_RANDR_ROTATION_ROTATE_270:
        return QStringLiteral("Rotate_270");
    case XCB_RANDR_ROTATION_REFLECT_X:
        return QStringLiteral("Reflect_X");
    case XCB_RANDR_ROTATION_REFLECT_Y:
        return QStringLiteral("Reflect_Y");
    }

    return QStringLiteral("invalid value (%1)").arg(rotation);
}

QString XCBEventListener::connectionToString(xcb_randr_connection_t connection)
{
    switch (connection) {
    case XCB_RANDR_CONNECTION_CONNECTED:
        return QStringLiteral("Connected");
    case XCB_RANDR_CONNECTION_DISCONNECTED:
        return QStringLiteral("Disconnected");
    case XCB_RANDR_CONNECTION_UNKNOWN:
        return QStringLiteral("UnknownConnection");
    }

    return QStringLiteral("invalid value (%1)").arg(connection);
}

bool XCBEventListener::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);

    if (eventType != "xcb_generic_event_t") {
        return false;
    }

    auto *e = static_cast<xcb_generic_event_t *>(message);
    const uint8_t xEventType = e->response_type & ~0x80;

    // If this event is not xcb_randr_notify, we don't want it
    if (xEventType == m_randrBase + XCB_RANDR_SCREEN_CHANGE_NOTIFY) {
        handleScreenChange(e);
    }
    if (xEventType == m_randrBase + XCB_RANDR_NOTIFY) {
        handleXRandRNotify(e);
    }

    return false;
}

void XCBEventListener::handleScreenChange(xcb_generic_event_t *e)
{
    auto *e2 = reinterpret_cast<xcb_randr_screen_change_notify_event_t *>(e);

    // Only accept notifications for our window
    if (e2->request_window != m_window) {
        return;
    }

    qCDebug(KSCREEN_XCB_HELPER) << "RRScreenChangeNotify";
    qCDebug(KSCREEN_XCB_HELPER) << "\tTimestamp: " << e2->timestamp;
    qCDebug(KSCREEN_XCB_HELPER) << "\tConfig_timestamp: " << e2->config_timestamp;
    qCDebug(KSCREEN_XCB_HELPER) << "\tWindow:" << e2->request_window;
    qCDebug(KSCREEN_XCB_HELPER) << "\tRoot:" << e2->root;
    qCDebug(KSCREEN_XCB_HELPER) << "\tRotation: " << rotationToString((xcb_randr_rotation_t)e2->rotation);
    qCDebug(KSCREEN_XCB_HELPER) << "\tSize ID:" << e2->sizeID;
    qCDebug(KSCREEN_XCB_HELPER) << "\tSize: " << e2->width << e2->height;
    qCDebug(KSCREEN_XCB_HELPER) << "\tSizeMM: " << e2->mwidth << e2->mheight;

    Q_EMIT screenChanged((xcb_randr_rotation_t)e2->rotation, QSize(e2->width, e2->height), QSize(e2->mwidth, e2->mheight));
    Q_EMIT outputsChanged();
}

void XCBEventListener::handleXRandRNotify(xcb_generic_event_t *e)
{
    auto *randrEvent = reinterpret_cast<xcb_randr_notify_event_t *>(e);

    if (randrEvent->subCode == XCB_RANDR_NOTIFY_CRTC_CHANGE) {
        xcb_randr_crtc_change_t crtc = randrEvent->u.cc;
        qCDebug(KSCREEN_XCB_HELPER) << "RRNotify_CrtcChange";
        qCDebug(KSCREEN_XCB_HELPER) << "\tTimestamp: " << crtc.timestamp;
        qCDebug(KSCREEN_XCB_HELPER) << "\tCRTC: " << crtc.crtc;
        qCDebug(KSCREEN_XCB_HELPER) << "\tMode: " << crtc.mode;
        qCDebug(KSCREEN_XCB_HELPER) << "\tRotation: " << rotationToString((xcb_randr_rotation_t)crtc.rotation);
        qCDebug(KSCREEN_XCB_HELPER) << "\tGeometry: " << crtc.x << crtc.y << crtc.width << crtc.height;
        Q_EMIT crtcChanged(crtc.crtc, crtc.mode, (xcb_randr_rotation_t)crtc.rotation, QRect(crtc.x, crtc.y, crtc.width, crtc.height), crtc.timestamp);

    } else if (randrEvent->subCode == XCB_RANDR_NOTIFY_OUTPUT_CHANGE) {
        xcb_randr_output_change_t output = randrEvent->u.oc;
        qCDebug(KSCREEN_XCB_HELPER) << "RRNotify_OutputChange";
        qCDebug(KSCREEN_XCB_HELPER) << "\tTimestamp: " << output.timestamp;
        qCDebug(KSCREEN_XCB_HELPER) << "\tOutput: " << output.output;
        qCDebug(KSCREEN_XCB_HELPER) << "\tCRTC: " << output.crtc;
        qCDebug(KSCREEN_XCB_HELPER) << "\tMode: " << output.mode;
        qCDebug(KSCREEN_XCB_HELPER) << "\tRotation: " << rotationToString((xcb_randr_rotation_t)output.rotation);
        qCDebug(KSCREEN_XCB_HELPER) << "\tConnection: " << connectionToString((xcb_randr_connection_t)output.connection);
        qCDebug(KSCREEN_XCB_HELPER) << "\tSubpixel Order: " << output.subpixel_order;
        Q_EMIT outputChanged(output.output, output.crtc, output.mode, (xcb_randr_connection_t)output.connection);

    } else if (randrEvent->subCode == XCB_RANDR_NOTIFY_OUTPUT_PROPERTY) {
        xcb_randr_output_property_t property = randrEvent->u.op;

        XCB::ScopedPointer<xcb_get_atom_name_reply_t> reply(
            xcb_get_atom_name_reply(QX11Info::connection(), xcb_get_atom_name(QX11Info::connection(), property.atom), nullptr));

        qCDebug(KSCREEN_XCB_HELPER) << "RRNotify_OutputProperty (ignored)";
        qCDebug(KSCREEN_XCB_HELPER) << "\tTimestamp: " << property.timestamp;
        qCDebug(KSCREEN_XCB_HELPER) << "\tOutput: " << property.output;
        qCDebug(KSCREEN_XCB_HELPER) << "\tProperty: " << QByteArray::fromRawData(xcb_get_atom_name_name(reply.data()), xcb_get_atom_name_name_length(reply.data()));
        qCDebug(KSCREEN_XCB_HELPER) << "\tState (newValue, Deleted): " << property.status;
    }
}

#include "moc_xcbeventlistener.cpp"
