// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>

#include "util.h"

#include <QSize>
#include <QPixmap>
#include <QBitmap>
#include <QFileInfo>
#include <QtGlobal>

#include <X11/Xlib.h>

#include <mutex>
#include <xcb/res.h>
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>

namespace tray {
void clean_xcb_image(void *data)
{
    xcb_image_destroy(static_cast<xcb_image_t *>(data));
}

Util* Util::instance()
{
    static Util* _instance = nullptr;
    if (_instance == nullptr)
        _instance = new Util();
    return _instance;
}

Util::Util()
{
    m_x11connection = xcb_connect(nullptr, nullptr);
    m_display = XOpenDisplay("");
    if (!m_x11connection || !isXAvaliable()) {
        return;
    }

    const xcb_setup_t *setup = xcb_get_setup(m_x11connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t* screen = iter.data;
    m_rootWindow = screen->root;

    xcb_ewmh_init_atoms_replies(&m_ewmh, xcb_ewmh_init_atoms(m_x11connection, &m_ewmh), nullptr);
}

Util::~Util()
{

}

bool Util::isXAvaliable()
{
    static std::once_flag flag;
    static bool avaliable = false;

    std::call_once(flag, [this](){
        if (!(m_x11connection && m_display)) return;

        // xtest support
        const xcb_query_extension_reply_t *xtest_ext_reply;
        xtest_ext_reply = xcb_get_extension_data(m_x11connection, &xcb_test_id);

        // xshape support
        const xcb_query_extension_reply_t *xshape_ext_reply;
        xshape_ext_reply = xcb_get_extension_data(m_x11connection, &xcb_shape_id);

        // xewmh support
        xcb_ewmh_connection_t ewmh;
        xcb_intern_atom_cookie_t *ewmh_cookie = xcb_ewmh_init_atoms(m_x11connection, &ewmh);
        if (!ewmh_cookie) return;

        xcb_ewmh_init_atoms_replies(&ewmh, ewmh_cookie, NULL);
        avaliable = m_x11connection && m_display &&
            (xtest_ext_reply && xtest_ext_reply->present) &&
            (xshape_ext_reply && xshape_ext_reply->present) &&
            (ewmh._NET_WM_STATE && ewmh._NET_WM_WINDOW_TYPE);
    });

    return avaliable;
}

xcb_connection_t* Util::getX11Connection()
{
    return m_x11connection;
}

xcb_window_t Util::getRootWindow()
{
    return m_rootWindow;
}

_XDisplay* Util::getDisplay()
{
    return m_display;
}

xcb_atom_t Util::getAtomByName(const QString &name)
{
    xcb_atom_t ret = m_atoms.value(name, 0);
    if (!ret) {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_x11connection, false, name.size(), name.toStdString().c_str());
        QSharedPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(m_x11connection, cookie, nullptr), [=](xcb_intern_atom_reply_t* reply){
            free(reply);}
        );

        if (reply) {
            m_atoms.insert(name, xcb_atom_t(reply->atom));
            ret = reply->atom;
        }
    }
    return ret;
}

QString Util::getNameByAtom(const xcb_atom_t& atom)
{
    auto name = m_atoms.key(atom);
    if (name.isEmpty()) {
        xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(m_x11connection, atom);
        QSharedPointer<xcb_get_atom_name_reply_t> reply(
            xcb_get_atom_name_reply(m_x11connection, cookie, nullptr),
            [=](xcb_get_atom_name_reply_t* reply) {free(reply);});
        if (!reply) {
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

void Util::moveX11Window(const xcb_window_t& window, const uint32_t& x, const uint32_t& y)
{
    const uint32_t windowMoveConfigVals[2] = {x, y};
    xcb_configure_window(m_x11connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, windowMoveConfigVals);
    xcb_flush(m_x11connection);
}

void Util::setX11WindowSize(const xcb_window_t& window, const QSize& size)
{
    const int windowSizeConfigVals[2] = {size.width(), size.height()};
    xcb_configure_window(m_x11connection, window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, windowSizeConfigVals);
    xcb_flush(m_x11connection);
}

QSize Util::getX11WindowSize(const xcb_window_t& window)
{
    auto cookie = xcb_get_geometry(m_x11connection, window);
    QSharedPointer<xcb_get_geometry_reply_t> clientGeom(xcb_get_geometry_reply(m_x11connection, cookie, nullptr));

    return clientGeom ? QSize(clientGeom->width, clientGeom->height) : QSize(0, 0);
}

QString Util::getX11WindowName(const xcb_window_t& window)
{
    std::string ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name(&m_ewmh, window);
    xcb_ewmh_get_utf8_strings_reply_t reply;
    if (xcb_ewmh_get_wm_name_reply(&m_ewmh, cookie, &reply, nullptr)) {
        ret.assign(reply.strings, reply.strings_len);
        xcb_ewmh_get_utf8_strings_reply_wipe(&reply);
    }
    return ret.c_str();
}

void Util::setX11WindowInputShape(const xcb_window_t& window, const QSize& size)
{
    xcb_rectangle_t rectangle;
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = size.width();
    rectangle.height = size.height();
    xcb_shape_rectangles(m_x11connection, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED, window, 0, 0, 1, &rectangle);
    xcb_shape_mask(m_x11connection, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, window, 0, 0, XCB_PIXMAP_NONE);

    const uint32_t stackData[] = {size.width() > 0 && size.height() > 0 ? XCB_STACK_MODE_ABOVE : XCB_STACK_MODE_BELOW};
    xcb_configure_window(m_x11connection, window, XCB_CONFIG_WINDOW_STACK_MODE, stackData);
    xcb_flush(m_x11connection);
}

QImage Util::getX11WidnowImageNonComposite(const xcb_window_t& window)
{
    QSize size = getX11WindowSize(window);
    xcb_image_t *image = xcb_image_get(m_x11connection, window, 0, 0, size.width(), size.height(), 0xFFFFFFFF, XCB_IMAGE_FORMAT_Z_PIXMAP);

    QImage naiveConversion;
    if (image) {
        naiveConversion = QImage(image->data, image->width, image->height, QImage::Format_ARGB32);
    } else {
        return QImage();
    }

    if (isTransparentImage(naiveConversion)) {
        QImage elaborateConversion = QImage(convertFromNative(image));
        if (isTransparentImage(elaborateConversion)) {
            return QImage();
        } else
            return elaborateConversion;
    } else {
        return QImage(image->data, image->width, image->height, image->stride, QImage::Format_ARGB32, clean_xcb_image, image);
    }
}

void Util::setX11WindowOpacity(const xcb_window_t& window, const double& opacity)
{
    xcb_atom_t opacityAtom = getAtomByName("_NET_WM_WINDOW_OPACITY");
    quint32 value = qRound64(qBound(qreal(0), opacity, qreal(1)) * 0xffffffff);

    xcb_change_property(m_x11connection,
                        XCB_PROP_MODE_REPLACE,
                        window,
                        opacityAtom,
                        XCB_ATOM_CARDINAL,
                        32,
                        1,
                        (uchar *)&value);
    xcb_flush(m_x11connection);
}

pid_t Util::getWindowPid(const xcb_window_t& window)
{
    xcb_res_client_id_spec_t spec = { window, XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID };
    xcb_res_query_client_ids_cookie_t cookie = xcb_res_query_client_ids_unchecked(m_x11connection, 1, &spec);
    QSharedPointer<xcb_res_query_client_ids_reply_t> reply(xcb_res_query_client_ids_reply(m_x11connection, cookie, NULL),[](xcb_res_query_client_ids_reply_t* reply){
        free(reply);
    });

    if (reply) {
        xcb_res_client_id_value_iterator_t iter = xcb_res_query_client_ids_ids_iterator(reply.get());
        for (; iter.rem; xcb_res_client_id_value_next(&iter)) {
            if (iter.data->spec.mask == XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID && xcb_res_client_id_value_value_length(iter.data) == 1) {
                return xcb_res_client_id_value_value(iter.data)[0];
            }
        }
    }
    // qWarning() << "failed to get pid for window: " << window;
    return 0;
}

QString Util::getProcExe(const pid_t& pid)
{
    return QFileInfo(QString("/proc/").append(QString::number(pid).append("/exe"))).canonicalFilePath().split("/").last();
}

void Util::sendXembedMessage(const xcb_window_t& window, const long& message, const long& data1, const long& data2, const long& data3)
{
    xcb_client_message_event_t ev;

    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = window;
    ev.format = 32;
    ev.data.data32[0] = XCB_CURRENT_TIME;
    ev.data.data32[1] = message;
    ev.data.data32[2] = data1;
    ev.data.data32[3] = data2;
    ev.data.data32[4] = data3;
    ev.type = getAtomByName(QStringLiteral("_XEMBED"));
    xcb_send_event(m_x11connection, false, window, XCB_EVENT_MASK_NO_EVENT, (char *)&ev);
}

QString Util::generateUniqueId(const QString &id)
{
    for (int i = 0; i < 100; i++) {
        QString newId = id + "-" + QString::number(i);
        if (!m_currentIds.contains(newId)) {
            m_currentIds.insert(newId);
            return newId;
        }
    }

    qWarning() << "failed to generate unique id:" << id;
    return id;
}

void Util::removeUniqueId(const QString &id) {
    m_currentIds.remove(id);
}

bool Util::isTransparentImage(const QImage &image)
{
    int w = image.width();
    int h = image.height();

    if (!(qAlpha(image.pixel(w >> 1, h >> 1)) + qAlpha(image.pixel(w >> 2, h >> 2)) == 0))
        return false;

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            if (qAlpha(image.pixel(x, y))) {
                return false;
            }
        }
    }

    return true;
}

QImage Util::convertFromNative(xcb_image_t *xcbImage)
{
    QImage::Format format = QImage::Format_Invalid;

    switch (xcbImage->depth) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 24:
        format = QImage::Format_RGB32;
        break;
    case 30: {
        quint32 *pixels = reinterpret_cast<quint32 *>(xcbImage->data);
        for (uint i = 0; i < (xcbImage->size / 4); i++) {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >> 2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
        Q_FALLTHROUGH();
    }
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        return QImage();
    }

    QImage image(xcbImage->data, xcbImage->width, xcbImage->height, xcbImage->stride, format, clean_xcb_image, xcbImage);

    if (image.isNull()) {
        return QImage();
    }

    if (format == QImage::Format_RGB32 && xcbImage->bpp == 32) {
        QImage m = image.createHeuristicMask();
        QPixmap p = QPixmap::fromImage(std::move(image));
        p.setMask(QBitmap::fromImage(std::move(m)));
        image = p.toImage();
    }

    if (image.format() == QImage::Format_MonoLSB) {
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }

    return image;
}
}
