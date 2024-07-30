/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "xrandr.h"

#include "xrandrconfig.h"
#include "xrandrscreen.h"

#include "../xcbeventlistener.h"
#include "../xcbwrapper.h"

#include "types.h"

#include <QRect>
#include <QTime>
#include <QTimer>

#include <QtGui/private/qtx11extras_p.h>

xcb_screen_t *XRandR::s_screen = nullptr;
xcb_window_t XRandR::s_rootWindow = 0;
XRandRConfig *XRandR::s_internalConfig = nullptr;
int XRandR::s_randrBase = 0;
int XRandR::s_randrError = 0;
bool XRandR::s_monitorInitialized = false;
bool XRandR::s_has_1_3 = false;
bool XRandR::s_xorgCacheInitialized = false;

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_XRANDR, "kscreen.xrandr")

XRandR::XRandR()
    : KScreen::AbstractBackend()
    , m_x11Helper(nullptr)
    , m_isValid(false)
    , m_configChangeCompressor(nullptr)
{
    qRegisterMetaType<xcb_randr_output_t>("xcb_randr_output_t");
    qRegisterMetaType<xcb_randr_crtc_t>("xcb_randr_crtc_t");
    qRegisterMetaType<xcb_randr_mode_t>("xcb_randr_mode_t");
    qRegisterMetaType<xcb_randr_connection_t>("xcb_randr_connection_t");
    qRegisterMetaType<xcb_randr_rotation_t>("xcb_randr_rotation_t");
    qRegisterMetaType<xcb_timestamp_t>("xcb_timestamp_t");

    // Use our own connection to make sure that we won't mess up Qt's connection
    // if something goes wrong on our side.
    xcb_generic_error_t *error = nullptr;
    xcb_randr_query_version_reply_t *version;
    XCB::connection();
    version = xcb_randr_query_version_reply(XCB::connection(), //
                                            xcb_randr_query_version(XCB::connection(), XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION),
                                            &error);
    if (!version || error) {
        XCB::closeConnection();
        free(error);
        return;
    }

    if ((version->major_version > 1) || ((version->major_version == 1) && (version->minor_version >= 2))) {
        m_isValid = true;
    } else {
        XCB::closeConnection();
        qCWarning(KSCREEN_XRANDR) << "XRandR extension not available or unsupported version";
        return;
    }

    if (s_screen == nullptr) {
        s_screen = XCB::screenOfDisplay(XCB::connection(), QX11Info::appScreen());
        s_rootWindow = s_screen->root;

        xcb_prefetch_extension_data(XCB::connection(), &xcb_randr_id);
        auto reply = xcb_get_extension_data(XCB::connection(), &xcb_randr_id);
        s_randrBase = reply->first_event;
        s_randrError = reply->first_error;
    }

    XRandR::s_has_1_3 = (version->major_version > 1 || (version->major_version == 1 && version->minor_version >= 3));

    if (s_internalConfig == nullptr) {
        s_internalConfig = new XRandRConfig();
    }

    if (!s_monitorInitialized) {
        m_x11Helper = new XCBEventListener();
        connect(m_x11Helper, &XCBEventListener::outputChanged, this, &XRandR::outputChanged, Qt::QueuedConnection);
        connect(m_x11Helper, &XCBEventListener::crtcChanged, this, &XRandR::crtcChanged, Qt::QueuedConnection);
        connect(m_x11Helper, &XCBEventListener::screenChanged, this, &XRandR::screenChanged, Qt::QueuedConnection);

        m_configChangeCompressor = new QTimer(this);
        m_configChangeCompressor->setSingleShot(true);
        m_configChangeCompressor->setInterval(500);
        connect(m_configChangeCompressor, &QTimer::timeout, [&]() {
            qCDebug(KSCREEN_XRANDR) << "Emitting configChanged()";
            Q_EMIT configChanged(config());
        });

        s_monitorInitialized = true;
    }
}

XRandR::~XRandR()
{
    delete m_x11Helper;
}

QString XRandR::name() const
{
    return QStringLiteral("XRandR");
}

QString XRandR::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.XRandR");
}

void XRandR::outputChanged(xcb_randr_output_t output, xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_connection_t connection)
{
    m_configChangeCompressor->start();

    XRandROutput *xOutput = s_internalConfig->output(output);
    if (!xOutput) {
        s_internalConfig->addNewOutput(output);
        return;
    }

    // check if this output disappeared
    if (crtc == XCB_NONE && mode == XCB_NONE && connection == XCB_RANDR_CONNECTION_DISCONNECTED) {
        XCB::OutputInfo info(output, XCB_TIME_CURRENT_TIME);
        if (info.isNull()) {
            s_internalConfig->removeOutput(output);
            qCDebug(KSCREEN_XRANDR) << "Output" << output << " removed";
            return;
        }
        // info is valid: the output is still there
    }

    xOutput->update(crtc, mode, connection);
    qCDebug(KSCREEN_XRANDR) << "Output" << xOutput->id() << ": connected =" << xOutput->isConnected() << ", enabled =" << xOutput->isEnabled();
}

void XRandR::crtcChanged(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_rotation_t rotation, const QRect &geom, xcb_timestamp_t timestamp)
{
    XRandRCrtc *xCrtc = s_internalConfig->crtc(crtc);
    if (!xCrtc) {
        s_internalConfig->addNewCrtc(crtc);
        xCrtc = s_internalConfig->crtc(crtc);
    }

    xCrtc->update(mode, rotation, geom); // BUG 472280 Still need to call update(...) because the timestamp is newer
    xCrtc->updateConfigTimestamp(timestamp);
    m_configChangeCompressor->start();
}

void XRandR::screenChanged(xcb_randr_rotation_t rotation, const QSize &sizePx, const QSize &sizeMm)
{
    Q_UNUSED(sizeMm);

    QSize newSizePx = sizePx;
    if (rotation == XCB_RANDR_ROTATION_ROTATE_90 || rotation == XCB_RANDR_ROTATION_ROTATE_270) {
        newSizePx.transpose();
    }

    XRandRScreen *xScreen = s_internalConfig->screen();
    Q_ASSERT(xScreen);
    xScreen->update(newSizePx);

    m_configChangeCompressor->start();
}

ConfigPtr XRandR::config() const
{
    return s_internalConfig->toKScreenConfig();
}

void XRandR::setConfig(const ConfigPtr &config)
{
    if (!config) {
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "XRandR::setConfig";
    s_internalConfig->applyKScreenConfig(config);
    qCDebug(KSCREEN_XRANDR) << "XRandR::setConfig done!";
}

QByteArray XRandR::edid(int outputId) const
{
    const XRandROutput *output = s_internalConfig->output(outputId);
    if (!output) {
        return QByteArray();
    }

    return output->edid();
}

bool XRandR::isValid() const
{
    return m_isValid;
}

quint8 *XRandR::getXProperty(xcb_randr_output_t output, xcb_atom_t atom, size_t &len)
{
    quint8 *result = nullptr;

    auto cookie = xcb_randr_get_output_property(XCB::connection(), output, atom, XCB_ATOM_ANY, 0, 100, false, false);
    auto reply = xcb_randr_get_output_property_reply(XCB::connection(), cookie, nullptr);
    if (!reply) {
        return result;
    }

    if (reply->type == XCB_ATOM_INTEGER && reply->format == 8) {
        result = new quint8[reply->num_items];
        memcpy(result, xcb_randr_get_output_property_data(reply), reply->num_items);
        len = reply->num_items;
    }

    free(reply);
    return result;
}

QByteArray XRandR::outputEdid(xcb_randr_output_t outputId)
{
    size_t len = 0;
    quint8 *result;

    auto edid_atom = XCB::InternAtom(false, 4, "EDID")->atom;
    result = XRandR::getXProperty(outputId, edid_atom, len);
    if (result == nullptr) {
        auto edid_atom = XCB::InternAtom(false, 9, "EDID_DATA")->atom;
        result = XRandR::getXProperty(outputId, edid_atom, len);
    }
    if (result == nullptr) {
        auto edid_atom = XCB::InternAtom(false, 25, "XFree86_DDC_EDID1_RAWDATA")->atom;
        result = XRandR::getXProperty(outputId, edid_atom, len);
    }

    QByteArray edid;
    if (result != nullptr) {
        if (len % 128 == 0) {
            edid = QByteArray(reinterpret_cast<const char *>(result), len);
        }
        delete[] result;
    }
    return edid;
}

bool XRandR::hasProperty(xcb_randr_output_t output, const QByteArray &name)
{
    xcb_generic_error_t *error = nullptr;
    auto atom = XCB::InternAtom(false, name.length(), name.constData())->atom;

    auto cookie = xcb_randr_get_output_property(XCB::connection(), output, atom, XCB_ATOM_ANY, 0, 1, false, false);
    auto prop_reply = xcb_randr_get_output_property_reply(XCB::connection(), cookie, &error);

    const bool ret = prop_reply->num_items == 1;
    free(prop_reply);
    return ret;
}

xcb_randr_get_screen_resources_reply_t *XRandR::screenResources()
{
    if (XRandR::s_has_1_3) {
        if (XRandR::s_xorgCacheInitialized) {
            // HACK: This abuses the fact that xcb_randr_get_screen_resources_reply_t
            // and xcb_randr_get_screen_resources_current_reply_t are the same
            return reinterpret_cast<xcb_randr_get_screen_resources_reply_t *>(
                xcb_randr_get_screen_resources_current_reply(XCB::connection(),
                                                             xcb_randr_get_screen_resources_current(XCB::connection(), XRandR::rootWindow()),
                                                             nullptr));
        } else {
            /* XRRGetScreenResourcesCurrent is faster then XRRGetScreenResources
             * because it returns cached values. However the cached values are not
             * available until someone calls XRRGetScreenResources first. In case
             * we happen to be the first ones, we need to fill the cache first. */
            XRandR::s_xorgCacheInitialized = true;
        }
    }

    return xcb_randr_get_screen_resources_reply(XCB::connection(), xcb_randr_get_screen_resources(XCB::connection(), XRandR::rootWindow()), nullptr);
}

xcb_window_t XRandR::rootWindow()
{
    return s_rootWindow;
}

xcb_screen_t *XRandR::screen()
{
    return s_screen;
}

#include "moc_xrandr.cpp"
