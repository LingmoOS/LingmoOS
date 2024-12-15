// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INSTANCESERVICE_H
#define INSTANCESERVICE_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusContext>

class InstanceService : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    ~InstanceService() override;
    InstanceService(const InstanceService &) = delete;
    InstanceService(InstanceService &&) = delete;
    InstanceService &operator=(const InstanceService &) = delete;
    InstanceService &operator=(InstanceService &&) = delete;

    Q_PROPERTY(QDBusObjectPath Application MEMBER m_Application CONSTANT)
    Q_PROPERTY(QDBusObjectPath SystemdUnitPath MEMBER m_SystemdUnitPath CONSTANT)
    Q_PROPERTY(QString Launcher MEMBER m_Launcher CONSTANT)
    Q_PROPERTY(bool Orphaned MEMBER m_orphaned NOTIFY orphanedChanged)

    [[nodiscard]] const QString &instanceId() const noexcept { return m_instanceId; }

public Q_SLOTS:
    void KillAll(int signal);

Q_SIGNALS:
    void orphanedChanged();

private:
    friend class ApplicationService;
    InstanceService(QString instanceId, QString application, QString systemdUnitPath, QString launcher);
    bool m_orphaned{false};
    QString m_Launcher;
    QString m_instanceId;
    QDBusObjectPath m_Application;
    QDBusObjectPath m_SystemdUnitPath;
};

static inline QList<QSharedPointer<InstanceService>> orphanedInstances{};

#endif
