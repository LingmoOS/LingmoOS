// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSETTINGS_H
#define CONFIGSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <utility>

enum SaveAction : unsigned int;
class ConfigSettings : public QObject
{
    Q_OBJECT
public:
    enum CursorType{
        NoCursor = 0,
        OnlyCursor,
        OnlyCursorClick,
        BothCursor
    };
    static ConfigSettings *instance();
    void setValue(const QString &group, const QString &key, const QVariant &val);
    QVariant getValue(const QString &group, const QString &key);
private:
    QStringList keys(const QString &group);
    QVariant getDefaultValue(const QString &group, const QString &key);
signals:
    void shapeConfigChanged(const QString &shape,  const QString &key, int index);
private:
    explicit ConfigSettings(QObject *parent = nullptr);
    ~ConfigSettings();
    const static QMap<QString, QMap<QString, QVariant>> m_defaultConfig;
    static ConfigSettings *m_configSettings;
    QSettings *m_settings;
    QMutex m_mutex;
};
#endif // CONFIGSETTINGS_H
