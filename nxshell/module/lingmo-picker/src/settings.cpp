// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include "settings.h"

#include <QDebug>

Settings::Settings(QObject *parent) : QObject(parent)
{
    // Init.
    settings = new QSettings(configPath());
    groupName = "Configure";
}

Settings::~Settings()
{
    delete settings;
}

QString Settings::configPath()
{
    QString path = QString("%1/%2/config.conf").
                   arg(qApp->organizationName()).
                   arg(qApp->applicationName());
    return path;
}

QVariant Settings::getOption(const QString &key, const QVariant &defaultValue)
{
    QVariant result;
    settings->beginGroup(groupName);
    result = settings->value(key, defaultValue);
    settings->endGroup();
    return result;
}

void Settings::setOption(const QString &key, const QVariant &value)
{
    settings->beginGroup(groupName);
    settings->setValue(key, value);
    settings->endGroup();

    settings->sync();
}
