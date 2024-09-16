// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "x11utils.h"
#include "dsglobal.h"

#include <cstddef>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/res.h>
#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

#include <QList>
#include <QImage>
#include <QBuffer>
#include <QGuiApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(x11UtilsLog, "dde.shell.dock.taskmanager.x11utils")

namespace dock {

X11Utils* X11Utils::instance()
{
    static X11Utils* utils = nullptr;
    if (utils == nullptr) {
        utils = new X11Utils();
    }
    return utils;
}

X11Utils::X11Utils()
{
    auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    m_connection = x11Application->connection();

    // init rootWidnow
    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    m_rootWindow = screen->root;

    xcb_ewmh_init_atoms_replies(&m_ewmh, xcb_ewmh_init_atoms(m_connection, &m_ewmh), nullptr);
}

X11Utils::~X11Utils()
{
    xcb_disconnect(m_connection);
}

xcb_connection_t* X11Utils::getXcbConnection()
{
    return m_connection;
}

xcb_window_t X11Utils::getRootWindow()
{
    return m_rootWindow;
}

xcb_window_t X11Utils::getActiveWindow()
{
    xcb_window_t ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window(&m_ewmh, 0);
    if (!xcb_ewmh_get_active_window_reply(&m_ewmh, cookie, &ret, nullptr)) {
        qCWarning(x11UtilsLog()) << "getActiveWindow failed";
    }

    return ret;
}

xcb_atom_t X11Utils::getAtomByName(const QString &name)
{
    xcb_atom_t ret = m_atoms.value(name, 0);
    if (!ret) {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(getXcbConnection(), false, name.size(), name.toStdString().c_str());
        QSharedPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(getXcbConnection(), cookie, nullptr), [=](xcb_intern_atom_reply_t* reply){
            free(reply);}
        );

        if (reply) {
            m_atoms.insert(name, xcb_atom_t(reply->atom));
            ret = reply->atom;
        }
    }
    return ret;
}

QString X11Utils::getNameByAtom(const xcb_atom_t& atom)
{
    auto name = m_atoms.key(atom);
    if (name.isEmpty()) {
        xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(getXcbConnection(), atom);
        QSharedPointer<xcb_get_atom_name_reply_t> reply(
            xcb_get_atom_name_reply(getXcbConnection(), cookie, nullptr),
            [=](xcb_get_atom_name_reply_t* reply) {free(reply);});
        if (!reply) {
            qCWarning(x11UtilsLog()) << "failed to get atom value for " + name;
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
QList<xcb_window_t> X11Utils::getWindowClientList(const xcb_window_t& window)
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

pid_t X11Utils::getWindowPid(const xcb_window_t& window)
{
    xcb_res_client_id_spec_t spec = { window, XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID };
    xcb_res_query_client_ids_cookie_t cookie = xcb_res_query_client_ids_unchecked(getXcbConnection(), 1, &spec);
    QSharedPointer<xcb_res_query_client_ids_reply_t> reply(xcb_res_query_client_ids_reply(getXcbConnection(), cookie, NULL),[](xcb_res_query_client_ids_reply_t* reply){
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
    qCWarning(x11UtilsLog()) << "failed to get pid for window: " << window;
    return 0;
}

QString X11Utils::getWindowName(const xcb_window_t& window)
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

QString X11Utils::getWindowIconName(const xcb_window_t& window)
{
    std::string ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_name(&m_ewmh, window);
    xcb_ewmh_get_utf8_strings_reply_t reply;
    if (xcb_ewmh_get_wm_icon_name_reply(&m_ewmh, cookie, &reply, nullptr)) {
        ret.assign(reply.strings, reply.strings_len);
        xcb_ewmh_get_utf8_strings_reply_wipe(&reply);
    }
    return ret.c_str();
}

QString X11Utils::getWindowIcon(const xcb_window_t& window)
{
    QString iconContent;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon(&m_ewmh, window);
    xcb_ewmh_get_wm_icon_reply_t reply;
    do {
        if(!xcb_ewmh_get_wm_icon_reply(&m_ewmh, cookie, &reply, nullptr)) break;

        QSharedPointer<xcb_ewmh_get_wm_icon_reply_t> replyPtr(&reply,[](xcb_ewmh_get_wm_icon_reply_t *ptr){
            xcb_ewmh_get_wm_icon_reply_wipe(ptr);
        });
    
        xcb_ewmh_wm_icon_iterator_t iter = xcb_ewmh_get_wm_icon_iterator(replyPtr.get());
        xcb_ewmh_wm_icon_iterator_t wmIconIt{0, 0, nullptr};
        for (; iter.rem; xcb_ewmh_get_wm_icon_next(&iter)) {
            const uint32_t size = iter.width * iter.height;
            if (size > 0 && size > wmIconIt.width * wmIconIt.height) {
                wmIconIt = iter;
            }
        }

        if (!wmIconIt.data) break;

        QImage img = QImage((uchar *)wmIconIt.data, wmIconIt.width, wmIconIt.height, QImage::Format_ARGB32).copy();

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");
        QString encode = buffer.data().toBase64();
        iconContent = QString("%1,%2").arg("data:image/png;base64").arg(encode);
        buffer.close();

    } while(0);

    return iconContent;
}

QList<xcb_atom_t> X11Utils::getWindowState(const xcb_window_t& window)
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

QList<xcb_atom_t> X11Utils::getWindowAllowedActions(const xcb_window_t &window)
{
    QList<xcb_atom_t> ret;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_allowed_actions(&m_ewmh, window);
    xcb_ewmh_get_atoms_reply_t reply;
    if (xcb_ewmh_get_wm_allowed_actions_reply(&m_ewmh, cookie, &reply, nullptr)) {
        for (uint32_t i = 0; i < reply.atoms_len; i++) {
            ret.push_back(reply.atoms[i]);
        }
        xcb_ewmh_get_atoms_reply_wipe(&reply);
    }

    return ret;
}

MotifWMHints X11Utils::getWindowMotifWMHints(const xcb_window_t& window)
{
    xcb_atom_t atomWmHints = getAtomByName("_MOTIF_WM_HINTS");
    xcb_get_property_cookie_t cookie = xcb_get_property(m_connection, false, window, atomWmHints, atomWmHints, 0, 5);
    std::unique_ptr<xcb_get_property_reply_t> reply(xcb_get_property_reply(m_connection, cookie, nullptr));
    if (!reply || reply->format != 32 || reply->value_len != 5)
        return MotifWMHints{0, 0, 0, 0, 0};

    uint32_t *data = static_cast<uint32_t *>(xcb_get_property_value(reply.get()));
    MotifWMHints ret;
    ret.flags = data[0];
    ret.functions = data[1];
    ret.decorations = data[2];
    ret.inputMode = data[3];
    ret.status = data[4];
    return ret;
}

QList<xcb_atom_t> X11Utils::getWindowTypes(const xcb_window_t &window)
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

void X11Utils::minimizeWindow(const xcb_window_t& window)
{
    uint32_t data[2];
    data[0] = XCB_ICCCM_WM_STATE_ICONIC;
    data[1] = XCB_NONE;
    xcb_ewmh_send_client_message(m_connection, window, m_rootWindow,getAtomByName("WM_CHANGE_STATE"), 2, data);
    xcb_flush(m_connection);
}

void X11Utils::maxmizeWindow(const xcb_window_t &window)
{
    xcb_ewmh_request_change_wm_state(&m_ewmh,
                                    0,
                                    window,
                                    XCB_EWMH_WM_STATE_ADD,
                                    getAtomByName("_NET_WM_STATE_MAXIMIZED_VERT"),
                                    getAtomByName("_NET_WM_STATE_MAXIMIZED_HORZ"),
                                    XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER);
    xcb_flush(m_connection);
}

void X11Utils::closeWindow(const xcb_window_t &window)
{
    xcb_ewmh_request_close_window(&m_ewmh, 0, window, 0, XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER);
}

void X11Utils::killClient(const xcb_window_t& winid)
{
    xcb_kill_client_checked(getXcbConnection(), winid);
}

void X11Utils::setActiveWindow(const xcb_window_t& window)
{
    xcb_ewmh_request_change_active_window(&m_ewmh, 0, window, XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER, XCB_CURRENT_TIME, XCB_WINDOW_NONE);
    restackWindow(window);
    xcb_flush(m_connection);
}

void X11Utils::restackWindow(const xcb_window_t& window)
{
    xcb_ewmh_request_restack_window(&m_ewmh, 0, window, 0, XCB_STACK_MODE_ABOVE);
}

void X11Utils::setWindowIconGemeotry(const xcb_window_t& window, const QRect& geometry)
{
    const auto ratio = qApp->devicePixelRatio();
    xcb_ewmh_set_wm_icon_geometry(&m_ewmh, window, geometry.x() * ratio, geometry.y() * ratio, geometry.width() * ratio, geometry.height() * ratio);
}

}
