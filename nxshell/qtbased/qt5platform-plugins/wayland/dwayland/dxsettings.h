// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXSETTINGS
#define DXSETTINGS

#include <xcb/xcb.h>
#include "dxcbxsettings.h"
#include "dnativesettings.h"

#include <QThread>

DPP_BEGIN_NAMESPACE

class DXSettings {
    DXSettings() {}
    ~DXSettings() {}
public:
    static DXSettings *instance() {
        static DXSettings *dxsettings = new DXSettings;
        return dxsettings;
    }
    void initXcbConnection();
    bool buildNativeSettings(QObject *object, quint32 settingWindow);
    void clearNativeSettings(quint32 settingWindow);
    DXcbXSettings *globalSettings();
    xcb_window_t getOwner(xcb_connection_t *conn = nullptr, int screenNumber = 0);

private:
    static xcb_connection_t *xcb_connection;
    static DXcbXSettings *m_xsettings;
};

#define dXSettings DXSettings::instance()

DPP_END_NAMESPACE

#endif//DXSETTINGS
