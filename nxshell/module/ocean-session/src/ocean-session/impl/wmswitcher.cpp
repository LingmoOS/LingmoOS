// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmswitcher.h"
#include "utils/utils.h"

#include <QDBusConnection>
#include <QDBusInterface>

#include <DDBusSender>

#define WM_NAME_3D "lingmo wm"
#define WM_NAME_2D "lingmo metacity"
#define OSD_Switch_WM_2D "SwitchWM2D"
#define OSD_Switch_WM_3D "SwitchWM3D"
#define OSD_Switch_WM_ERROR "SwitchWMError"

WMSwitcher::WMSwitcher(QObject *parent)
    : QObject(parent)
    , m_wmInter(new com::lingmo::wm("com.lingmo.wm", "/com/lingmo/wm", QDBusConnection::sessionBus(), this))
{
    connect(m_wmInter, &com::lingmo::wm::compositingEnabledChanged, this, &WMSwitcher::onCompositingEnabledChanged);
}

bool WMSwitcher::AllowSwitch() const
{
    if (Utils::IS_WAYLAND_DISPLAY) {
        return false;
    } else {
        return m_wmInter->compositingPossible();
    }
}

QString WMSwitcher::CurrentWM() const
{
    if (Utils::IS_WAYLAND_DISPLAY) {
        return QString();
    } else {
        return m_wmInter->compositingEnabled() ? WM_NAME_3D : WM_NAME_2D;
    }
}

void WMSwitcher::RequestSwitchWM() const
{
    if (Utils::IS_WAYLAND_DISPLAY)
        return;

    showOSD("SwitchWM");
}

void WMSwitcher::onCompositingEnabledChanged(bool enable)
{
    Q_EMIT WMChanged(enable ? WM_NAME_3D : WM_NAME_2D);

    showOSD(enable ? OSD_Switch_WM_3D : OSD_Switch_WM_2D);
}

void WMSwitcher::showOSD(const QString &osd) const
{
    DDBusSender()
            .service("org.lingmo.ocean.Osd1")
            .path("/")
            .interface("org.lingmo.ocean.Osd1")
            .method("ShowOSD")
            .arg(osd)
            .call();
}
