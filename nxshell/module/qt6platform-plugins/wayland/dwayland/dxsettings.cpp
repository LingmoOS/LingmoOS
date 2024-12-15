// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dxsettings.h"
#include <QCoreApplication>

DPP_BEGIN_NAMESPACE

class DXcbEventFilter : public QThread
{
public:
    DXcbEventFilter(xcb_connection_t *connection)
        : m_connection(connection)
        , m_threadProxy(new RunInThreadProxy(qApp))
    {
        QThread::start();
    }

    void run() override {
        xcb_generic_event_t *event;
        while (m_connection && (event = xcb_wait_for_event(m_connection))) {
            uint response_type = event->response_type & ~0x80;
            switch (response_type) {
                case XCB_PROPERTY_NOTIFY: {
                    m_threadProxy->proxyCall([event]() {
                        xcb_property_notify_event_t *pn = (xcb_property_notify_event_t *)event;
                        DXcbXSettings::handlePropertyNotifyEvent(pn);
                    });
                    break;
                }

                case XCB_CLIENT_MESSAGE: {
                    m_threadProxy->proxyCall([event]() {
                        xcb_client_message_event_t *ev = reinterpret_cast<xcb_client_message_event_t*>(event);
                        DXcbXSettings::handleClientMessageEvent(ev);
                    });
                    break;
                }
            }
        }
    }

private:
    xcb_connection_t *m_connection;
    RunInThreadProxy *m_threadProxy = nullptr;
};

xcb_connection_t *DXSettings::xcb_connection = nullptr;
DXcbXSettings *DXSettings::m_xsettings = nullptr;

void DXSettings::initXcbConnection()
{
    static bool isInit = false;

    if (isInit && xcb_connection) {
        return;
    }

    isInit = true;
    int primary_screen_number = 0;
    xcb_connection = xcb_connect(qgetenv("DISPLAY"), &primary_screen_number);

    new DXcbEventFilter(xcb_connection);
}

bool DXSettings::buildNativeSettings(QObject *object, quint32 settingWindow)
{
    QByteArray settings_property = DNativeSettings::getSettingsProperty(object);
    DXcbXSettings *settings = nullptr;
    bool global_settings = false;
    if (settingWindow || !settings_property.isEmpty()) {
        settings = new DXcbXSettings(xcb_connection, settingWindow, settings_property);
    } else {
        global_settings = true;
        settings = globalSettings();
    }

    // 跟随object销毁
    auto native_settings = new DNativeSettings(object, settings, global_settings);

    if (!native_settings->isValid()) {
        delete native_settings;
        return false;
    }

    return true;
}

void DXSettings::clearNativeSettings(quint32 settingWindow)
{
#ifdef Q_OS_LINUX
    DXcbXSettings::clearSettings(settingWindow);
#endif
}

DXcbXSettings *DXSettings::globalSettings()
{
    if (Q_LIKELY(m_xsettings)) {
        return m_xsettings;
    }

    if (!xcb_connection) {
        initXcbConnection();
    }
    m_xsettings = new DXcbXSettings(xcb_connection);

    return m_xsettings;
}

xcb_window_t DXSettings::getOwner(xcb_connection_t *conn, int screenNumber) {
    return DXcbXSettings::getOwner(conn, screenNumber);
}

DPP_END_NAMESPACE
