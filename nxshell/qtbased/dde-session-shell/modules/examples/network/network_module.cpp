// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "network_module.h"

#include <QWidget>
#include <QLabel>
#include <QIcon>

namespace dss {
namespace module {

NetworkModule::NetworkModule(QObject *parent)
    : QObject(parent)
    , m_networkWidget(nullptr)
    , m_tipLabel(nullptr)
    , m_itemLabel(nullptr)
{
    setObjectName(QStringLiteral("NetworkModule"));
}

NetworkModule::~NetworkModule()
{
    if (m_networkWidget) {
        delete m_networkWidget;
    }

    if (m_itemLabel) {
        delete m_itemLabel;
    }
}

void NetworkModule::init()
{
    initUI();

    m_itemLabel = new QLabel;
    m_itemLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(20, 20));
    m_itemLabel->setFixedSize(20, 20);
}

QWidget *NetworkModule::itemWidget() const
{
    return m_itemLabel;
}

QWidget *NetworkModule::itemTipsWidget() const
{
    return m_tipLabel;
}

const QString NetworkModule::itemContextMenu() const
{
    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> shift;
    shift["itemId"] = "SHIFT";
    shift["itemText"] = tr("Turn on");
    shift["isActive"] = true;
    items.push_back(shift);

    QMap<QString, QVariant> settings;
    settings["itemId"] = "SETTINGS";
    settings["itemText"] = tr("Turn off");
    settings["isActive"] = true;
    items.push_back(settings);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;
    return QJsonDocument::fromVariant(menu).toJson();
}

void NetworkModule::invokedMenuItem(const QString &menuId, const bool checked) const
{
    Q_UNUSED(checked)

    if (menuId == "SHIFT") {
        qDebug() << "shift clicked";
    } else if (menuId == "SETTINGS") {
        qDebug() << "settings clicked";
    }
}

void NetworkModule::initUI()
{
    if (m_networkWidget) {
        return;
    }
    m_networkWidget = new QWidget;
    m_networkWidget->setAccessibleName(QStringLiteral("NetworkWidget"));
    m_networkWidget->setStyleSheet("background-color: red");
    m_networkWidget->setFixedSize(200, 100);

    m_tipLabel = new QLabel("network info");
}

} // namespace module
} // namespace dss
