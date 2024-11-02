/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "applistwidget.h"
#include <QDebug>

AppListWidget::AppListWidget(QString path, QWidget *parent)
    : m_path(path), QWidget(parent)
{
    initUI();
    initDbus();
}

AppListWidget::~AppListWidget()
{
    delete m_dbusInterface;
}

/**
 * @brief AppListWidget::setAppChecked
 * @param flag 是否使用应用代理配置
 */
void AppListWidget::setAppChecked(bool flag)
{
    m_checkBox->setChecked(flag);
}

/**
 * @brief AppListWidget::setAppIcon
 * @param icon 应用图标
 */
void AppListWidget::setAppIcon(const QIcon &icon)
{
    m_iconBtn->setIcon(icon);
}

/**
 * @brief AppListWidget::setAppName
 * @param text 应用名称
 */
void AppListWidget::setAppName(const QString &text)
{
    m_nameLabel->setText(text);
}

/**
 * @brief AppListWidget::onAppCheckStateChanged
 * checkBox是否选中 关联是否配置应用代理
 */
void AppListWidget::onAppCheckStateChanged()
{
    if (m_checkBox->isChecked()) {
        AddAppProxyConfig();
    } else {
        RemoveAppProxyConfig();
    }
}

/**
 * @brief AppListWidget::AddAppProxyConfig
 * 将该路径下的应用加入应用代理
 */
void AppListWidget::AddAppProxyConfig()
{
    if(m_dbusInterface == nullptr || !m_dbusInterface->isValid()) {
        qWarning ()<< "init AppProxy dbus error";
        return;
    }

    qDebug() << "call QDBusInterface addAppIntoProxy";
    m_dbusInterface->call("addAppIntoProxy", m_path);
}

/**
 * @brief AppListWidget::RemoveAppProxyConfig
 * 将该路径下的应用移出应用代理
 */
void AppListWidget::RemoveAppProxyConfig()
{
    if(m_dbusInterface == nullptr || !m_dbusInterface->isValid()) {
        qWarning ()<< "init AppProxy dbus error";
        return;
    }

    qDebug() << "call QDBusInterface delAppIntoProxy";
    m_dbusInterface->call("delAppIntoProxy", m_path);
}

void AppListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_checkBox->setChecked(!m_checkBox->isChecked());
        onAppCheckStateChanged();
    }
    return QWidget::mousePressEvent(event);
}

void AppListWidget::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(17, 0, 17, 0);
    mainLayout->setSpacing(8);
    m_checkBox = new QCheckBox(this);
    m_checkBox->setAttribute(Qt::WA_TransparentForMouseEvents, true); //m_checkBox不响应鼠标事件,将其传递给父窗口

    m_iconBtn = new QToolButton(this);
    m_iconBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
    m_iconBtn->setAttribute(Qt::WA_TranslucentBackground, true); //透明
    m_iconBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_iconBtn->setAutoRaise(true);
    m_iconBtn->setFixedSize(24, 24);
    m_iconBtn->setIconSize(QSize(24, 24));

    m_nameLabel = new QLabel(this);

    mainLayout->addWidget(m_checkBox);
    mainLayout->addWidget(m_iconBtn);
    mainLayout->addWidget(m_nameLabel);
    mainLayout->addStretch();
}

void AppListWidget::initDbus()
{
    m_dbusInterface = new QDBusInterface("org.lingmo.SettingsDaemon",
                       "/org/lingmo/SettingsDaemon/AppProxy",
                       "org.lingmo.SettingsDaemon.AppProxy",
                       QDBusConnection::sessionBus());
    if(!m_dbusInterface->isValid()) {
        qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}
