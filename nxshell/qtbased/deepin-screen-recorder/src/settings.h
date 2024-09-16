// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();
    QString configPath();

    QVariant getOption(const QString &key);
    void setOption(const QString &key, const QVariant &value);

private:
    QSettings *settings;
    QString groupName;
};

#endif // SETTINGS_H
