// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INHIBITOR_H
#define INHIBITOR_H

#include <QObject>
#include <QDBusObjectPath>

class Inhibitor : public QObject
{
    Q_OBJECT
public:
    explicit Inhibitor(const QString &appId, uint topLevelXid, const QString &reason, uint flags, uint index);

public Q_SLOTS:
    inline const QString &GetAppId() { return m_appId; }
    inline const QDBusObjectPath GetClientId() { return QDBusObjectPath(QString("/org/deepin/dde/Inhibitors/Inhibitor_%1").arg(m_index)); }
    inline const uint &GetFlags() { return m_flags; }
    inline const QString &GetReason() { return m_reason; }
    inline const uint &GetToplevelXid() { return m_topLevelXid; }

private:
    QString m_appId;
    QString m_reason;
    uint m_index;
    uint m_topLevelXid;
    uint m_flags;
};
#endif // INHIBITOR_H
