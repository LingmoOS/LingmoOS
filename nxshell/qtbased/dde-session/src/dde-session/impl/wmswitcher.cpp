// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmswitcher.h"

#include <QDBusConnection>
#include <QDBusInterface>

#include <DDBusSender>

#define WM_NAME_3D "deepin wm"
#define WM_NAME_2D "deepin metacity"
#define OSD_Switch_WM_2D "SwitchWM2D"
#define OSD_Switch_WM_3D "SwitchWM3D"
#define OSD_Switch_WM_ERROR "SwitchWMError"

WMSwitcher::WMSwitcher(QObject *parent)
    : QObject(parent)
    , m_wmInter(new com::deepin::wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
{
    connect(m_wmInter, &com::deepin::wm::compositingEnabledChanged, this, &WMSwitcher::onCompositingEnabledChanged);
}

bool WMSwitcher::AllowSwitch() const
{
    return m_wmInter->compositingPossible();
}

QString WMSwitcher::CurrentWM() const
{
    return m_wmInter->compositingEnabled() ? WM_NAME_3D : WM_NAME_2D;
}

void WMSwitcher::RequestSwitchWM() const
{
    if (m_wmInter->compositingEnabled()) {
        m_wmInter->setCompositingEnabled(false);
        return;
    }

    // 开启特效前需要先判断是否支持开启
    if (m_wmInter->compositingPossible()) {
        m_wmInter->setCompositingEnabled(true);
        return;
    }

    // 无法开启特效，发送osd通知
    showOSD(OSD_Switch_WM_ERROR);
}

void WMSwitcher::onCompositingEnabledChanged(bool enable)
{
    Q_EMIT WMChanged(enable ? WM_NAME_3D : WM_NAME_2D);

    showOSD(enable ? OSD_Switch_WM_3D : OSD_Switch_WM_2D);
}

void WMSwitcher::showOSD(const QString &osd) const
{
    DDBusSender()
            .service("org.deepin.dde.Osd1")
            .path("/")
            .interface("org.deepin.dde.Osd1")
            .method("ShowOSD")
            .arg(osd)
            .call();
}
