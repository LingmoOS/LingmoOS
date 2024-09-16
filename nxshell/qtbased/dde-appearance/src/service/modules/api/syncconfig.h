// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNCCONFIG_H
#define SYNCCONFIG_H
#include <QObject>

class SyncConfig : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.sync.Config")

public Q_SLOTS:
    virtual QByteArray Get();
    virtual void Set(QByteArray);

public:
    SyncConfig(QString name, QString path);
    void registerConfig();
    virtual ~SyncConfig() =default;

private Q_SLOTS:
    void handleNameOwnerChanged(QString name, QString oldOwner, QString newOwner);

private:
    QString         name;
    QString         path;
};

#endif // SYNCCONFIG_H
