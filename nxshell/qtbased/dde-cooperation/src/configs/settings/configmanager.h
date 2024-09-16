// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>

class Settings;
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager *instance();

    QVariant appAttribute(const QString &group, const QString &key);
    void setAppAttribute(const QString &group, const QString &key, const QVariant &value);
    bool syncAppAttribute();

    Settings *appSetting();

Q_SIGNALS:
    void appAttributeChanged(const QString &group, const QString &key, const QVariant &value);
    void appAttributeEdited(const QString &group, const QString &key, const QVariant &value);

protected:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    void init();

private:
    Settings *appSettings { nullptr };   // app config
};

#endif   // CONFIGMANAGER_H
