// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "brightnessitem.h"
#include "constants.h"
#include "brightnessmodel.h"

#include <DDBusSender>
#include <DGuiApplicationHelper>

#include <QDBusConnection>
#include <QIcon>
#include <QJsonDocument>
#include <QPainter>
#include <QVBoxLayout>

DGUI_USE_NAMESPACE

#define SETTINGS "settings"

BrightnessItem::BrightnessItem(QObject* parent)
    : QObject(parent)
    , m_tipsLabel(nullptr)
    , m_applet(new BrightnessApplet)
    , m_icon(new CommonIconButton)
{
    init();
}

BrightnessItem::~BrightnessItem()
{
    if (m_tipsLabel) {
        m_tipsLabel->deleteLater();
        m_tipsLabel = nullptr;
    }

    if (m_applet) {
        m_applet->deleteLater();
        m_applet = nullptr;
    }

    if (m_icon) {
        m_icon->deleteLater();
        m_icon = nullptr;
    }
}

void BrightnessItem::init()
{
    m_icon->setFixedSize(Dock::DOCK_PLUGIN_ITEM_FIXED_SIZE);
    m_icon->setIcon(QIcon::fromTheme("display-brightness-control"));
    connect(m_applet, &BrightnessApplet::requestHideApplet, this, &BrightnessItem::requestHideApplet);
}

const QString BrightnessItem::contextMenu() const
{
    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> settings;
    settings["itemId"] = SETTINGS;
    settings["itemText"] = tr("Display settings");
    settings["isActive"] = true;
    items.push_back(settings);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void BrightnessItem::invokeMenuItem(const QString menuId, const bool checked)
{
    Q_UNUSED(menuId);
    Q_UNUSED(checked);

    if (menuId == SETTINGS) {
        DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QString("ShowPage"))
            .arg(QString("display"))
            .arg(QString(""))
            .call();

        Q_EMIT requestHideApplet();
    }
}

QWidget *BrightnessItem::tipsWidget()
{
    if (!m_tipsLabel)
        updateTips();
    return m_tipsLabel;
}

void BrightnessItem::updateTips()
{
    if (!m_tipsLabel) {
        m_tipsLabel = new QLabel;
        m_tipsLabel->setForegroundRole(QPalette::BrightText);
        m_tipsLabel->setContentsMargins(0, 0, 0, 0);
        connect(qApp, &QGuiApplication::fontChanged, this, &BrightnessItem::updateTips);
        connect(&BrightnessModel::ref(), &BrightnessModel::enabledMonitorListChanged, this, &BrightnessItem::updateTips);
        connect(&BrightnessModel::ref(), &BrightnessModel::monitorBrightnessChanged, this, &BrightnessItem::updateTips);
    }

    QString tips;
    for (const auto &monitor : BrightnessModel::ref().enabledMonitors()) {
        tips += QString("%1: %2%<br/>").arg(monitor->name()).arg(QString::number(monitor->brightness() * 100));
    }

    m_tipsLabel->setText(tips);
    m_tipsLabel->adjustSize();
}
