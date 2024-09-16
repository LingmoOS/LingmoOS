// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QWindow>
#include <QX11Info>
#include <QMouseEvent>

#include <qpa/qplatformwindow.h>
#include <xcb/xcb.h>

void setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format)
{
    xcb_connection_t* conn = QX11Info::connection();
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, WId, propAtom, typeAtom, format, len, data);
    xcb_flush(conn);
}

void clearWindowProperty(quint32 WId, xcb_atom_t propAtom)
{
    xcb_delete_property_checked(QX11Info::connection(), WId, propAtom);
}

xcb_atom_t internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, only_if_exists, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

xcb_atom_t internAtom(const char *name, bool only_if_exists)
{
    return internAtom(QX11Info::connection(), name, only_if_exists);
}

bool isSupported(xcb_atom_t targetAtom)
{
    // get _NET_SUPPORTED from root window
    QVector<xcb_atom_t> net_wm_atoms;

    xcb_window_t root = QX11Info::appRootWindow();
    int offset = 0;
    int remaining = 0;
    xcb_connection_t *xcb_connection = QX11Info::connection();
    auto _net_supported = internAtom(QT_STRINGIFY(_NET_SUPPORTED), false);

    do {
        xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection, false, root, _net_supported,
                                                            XCB_ATOM_ATOM, offset, 1024);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection, cookie, NULL);
        if (!reply)
            break;

        remaining = 0;

        if (reply->type == XCB_ATOM_ATOM && reply->format == 32) {
            int len = xcb_get_property_value_length(reply)/sizeof(xcb_atom_t);
            xcb_atom_t *atoms = (xcb_atom_t *)xcb_get_property_value(reply);
            int s = net_wm_atoms.size();
            net_wm_atoms.resize(s + len);
            memcpy(net_wm_atoms.data() + s, atoms, len*sizeof(xcb_atom_t));

            remaining = reply->bytes_after;
            offset += len;
        }

        free(reply);
    } while (remaining > 0);

    return net_wm_atoms.contains(targetAtom);
}

bool setNoTitlebar(quint32 WId, bool on)
{
    xcb_atom_t _deepin_no_titlebar = internAtom(QT_STRINGIFY(_DEEPIN_NO_TITLEBAR), false);
    if (!isSupported(_deepin_no_titlebar))
        return false;

    quint8 value = on;
    setWindowProperty(WId, _deepin_no_titlebar, XCB_ATOM_CARDINAL, &value, 1, 8);

    // force enable window decorations, becouse we need the window shadow and border, its contains in decorations.
    xcb_atom_t _deepin_force_decorate = internAtom(QT_STRINGIFY(_DEEPIN_FORCE_DECORATE), false);
    if (on) {
        quint8 value = on;
        setWindowProperty(WId, _deepin_force_decorate, XCB_ATOM_CARDINAL, &value, 1, 8);
    } else {
        clearWindowProperty(WId, _deepin_force_decorate);
    }

    return true;
}

QObject *createNativeSettingsFor(QWindow *window)
{
    // The platform function in dxcb plugin:
    // https://github.com/linuxdeepin/qt5platform-plugins/blob/master/src/dnativesettings.h
    static QFunctionPointer build_function = qApp->platformFunction("_d_buildNativeSettings");

    if (!build_function) {
        return nullptr;
    }

    QObject *obj = new QObject(window);
    bool ok = reinterpret_cast<bool(*)(QObject*, quint32)>(build_function)(obj, window->winId());
    if (!ok) {
        delete obj;
        return nullptr;
    }

    return obj;
}

class Window : public QWindow
{
public:
    bool handleMoveing = false;

private:
    void mouseMoveEvent(QMouseEvent *e) override {
        if (handleMoveing && e->buttons() == Qt::LeftButton) {
            handle()->startSystemMove();
        } else {
            QWindow::mouseMoveEvent(e);
        }
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    Window window;
    window.resize(200, 200);

    if (setNoTitlebar(window.winId(), true)) {
        // If enabled no titlebar mode, the window is no min/max/close buttons, you need to implement it by Qt widgets.
        // Like as https://github.com/linuxdeepin/dtkwidget/blob/master/src/widgets/dtitlebar.cpp
        window.handleMoveing = true;

        auto nativeSettings = createNativeSettingsFor(&window);
        if (nativeSettings) {
            nativeSettings->setProperty("DTK/WindowRadius", "100,100");
            nativeSettings->setProperty("borderWidth", 2);
            nativeSettings->setProperty("borderColor", QColor(Qt::red));
            nativeSettings->setProperty("shadowOffset", "30,30");
            nativeSettings->setProperty("shadowRadius", 30);
            nativeSettings->setProperty("shadowColor", QColor(Qt::blue));
            // See more properties in DPlatformTheme
        }
    }

    window.show();

    return app.exec();
}
