//SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DOCKDBUSPROXY_H
#define DOCKDBUSPROXY_H

#include <QObject>
#include <QDBusArgument>

class QDBusInterface;
class QDBusMessage;

struct DockItemInfo
{
    QString name;
    QString displayName;
    QString itemKey;
    QString settingKey;
    QString dcc_icon;
    bool visible;
};

QDBusArgument &operator<<(QDBusArgument &arg, const DockItemInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &arg, DockItemInfo &info);

Q_DECLARE_METATYPE(DockItemInfo)

typedef QList<DockItemInfo> DockItemInfos;

Q_DECLARE_METATYPE(DockItemInfos)

class DockDBusProxy : public QObject
{
    Q_OBJECT

public:
    explicit DockDBusProxy(QObject *parent = nullptr);

public Q_SLOTS:
    DockItemInfos plugins();
    void setItemOnDock(const QString &settingKey, const QString &itemKey, bool visible);

Q_SIGNALS:
    void pluginVisibleChanged(const QString &itemKey, bool visible) const;

private:
    QDBusInterface *m_dockInter;
};

#endif // DOCKDBUSPROXY_H
