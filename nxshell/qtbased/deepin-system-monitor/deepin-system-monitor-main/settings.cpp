// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include <QStandardPaths>
#include <QApplication>
#include <QSettings>

Q_GLOBAL_STATIC(Settings, theInstance)
Settings::Settings()
{
    const QString &filepath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + qApp->organizationName() + "/" + qApp->applicationName() + "/" + "config.conf";
    m_settings = new QSettings(filepath, QSettings::IniFormat);
}

Settings::~Settings()
{
    m_settings->deleteLater();
}

Settings *Settings::instance()
{
    return theInstance();
}

void Settings::flush()
{
    m_settings->sync();
}

QVariant Settings::getOption(const QString &key, const QVariant &defaultValue)
{
    m_settings->beginGroup("settings");
    const QVariant &result = m_settings->value(key, defaultValue);
    m_settings->endGroup();
    return result;
}

void Settings::setOption(const QString &key, const QVariant &value)
{
    m_settings->beginGroup("settings");
    m_settings->setValue(key, value);
    m_settings->endGroup();
}
