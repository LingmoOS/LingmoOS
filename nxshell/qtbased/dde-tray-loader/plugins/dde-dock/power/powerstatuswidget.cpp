// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "powerstatuswidget.h"
#include "powerplugin.h"
#include "dbus/dbuspower.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QIcon>
#include <QMouseEvent>

DGUI_USE_NAMESPACE

PowerStatusWidget::PowerStatusWidget(QWidget *parent)
    : QWidget(parent)
    , m_powerInter(new DBusPower(this))
    , m_iconWidget(new CommonIconButton(this))
    , m_applet(new PowerApplet(this))
{
    m_iconWidget->setFixedSize(Dock::DOCK_PLUGIN_ITEM_FIXED_SIZE);

    m_applet->setVisible(false);
    m_applet->setDccPage("power", "general");
    m_applet->setDescription(tr("Power settings"));
    m_applet->setIcon(QIcon::fromTheme("open-arrow"));

    connect(m_applet, &PowerApplet::requestHideApplet, this, &PowerStatusWidget::requestHideApplet);
    connect(m_powerInter, &DBusPower::BatteryPercentageChanged, this, &PowerStatusWidget::refreshIcon);
    connect(m_powerInter, &DBusPower::BatteryStateChanged, this, &PowerStatusWidget::refreshIcon);
    connect(m_powerInter, &DBusPower::OnBatteryChanged, this, &PowerStatusWidget::refreshIcon);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &PowerStatusWidget::refreshIcon);
    refreshIcon();
}

void PowerStatusWidget::refreshIcon()
{
    const BatteryPercentageMap data = m_powerInter->batteryPercentage();
    const uint value = uint(qMin(100.0, qMax(0.0, data.value("Display"))));
    const int percentage = int(std::round(value));
    // onBattery应该表示的是当前是否使用电池在供电，为true表示没插入电源
    const bool plugged = !m_powerInter->onBattery();
    const auto &stateMap = m_powerInter->batteryState();
    const BatteryState batteryState = static_cast<BatteryState>(stateMap["Display"]);

    /*根据新需求，电池电量显示分别是*/
    /* 0-5%、6-10%、11%-20%、21-30%、31-40%、41-50%、51-60%、61%-70%、71-80%、81-90%、91-100% */
    QString percentageStr;
    if (percentage <= 5) {
        percentageStr = "000";
    } else if (percentage <= 10) {
        percentageStr = "010";
    } else if (percentage <= 20) {
        percentageStr = "020";
    } else if (percentage <= 30) {
        percentageStr = "030";
    } else if (percentage <= 40) {
        percentageStr = "040";
    } else if (percentage <= 50) {
        percentageStr = "050";
    } else if (percentage <= 60) {
        percentageStr = "060";
    } else if (percentage <= 70) {
        percentageStr = "070";
    } else if (percentage <= 80) {
        percentageStr = "080";
    } else if (percentage <= 90) {
        percentageStr = "090";
    } else {
        percentageStr = "100";
    }

    QString iconStr;
    if ((batteryState == BatteryState::FULLY_CHARGED || percentage == 100) && plugged) {
        iconStr = QString("battery-full-charged-symbolic");
    } else if (batteryState == BatteryState::NOT_CHARGED && plugged) {
        iconStr = QString("battery-%1-plugged-symbolic").arg(percentageStr);
    } else {
        iconStr = QString("battery-%1-%2")
                  .arg(percentageStr, plugged ? "plugged-symbolic" : "symbolic");
    }

    m_iconWidget->setIcon(iconStr, ":/batteryicons/resources/batteryicons/" + iconStr + ".svg");
    m_applet->refreshBatteryIcon(iconStr);
}

void PowerStatusWidget::refreshBatteryPercentage(const QString &percentage, const QString &tips)
{
    m_applet->refreshBatteryPercentage(percentage, tips);
}

void PowerStatusWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    // 保持横纵比
    if (position == Dock::Bottom || position == Dock::Top) {
        setMaximumWidth(height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    } else {
        setMaximumHeight(width());
        setMaximumWidth(QWIDGETSIZE_MAX);
    }
}
