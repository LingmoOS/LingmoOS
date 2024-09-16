// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONMANAGER1SERVICE_H
#define APPLICATIONMANAGER1SERVICE_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QSharedPointer>
#include <QScopedPointer>
#include <memory>
#include <QMap>
#include <QHash>
#include <QFileSystemWatcher>
#include <QTimer>
#include "applicationmanagerstorage.h"
#include "dbus/jobmanager1service.h"
#include "dbus/mimemanager1service.h"
#include "desktopentry.h"
#include "identifier.h"

class ApplicationService;

class ApplicationManager1Service final : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    explicit ApplicationManager1Service(std::unique_ptr<Identifier> ptr,
                                        std::weak_ptr<ApplicationManager1Storage> storage) noexcept;
    ~ApplicationManager1Service() override;
    ApplicationManager1Service(const ApplicationManager1Service &) = delete;
    ApplicationManager1Service(ApplicationManager1Service &&) = delete;
    ApplicationManager1Service &operator=(const ApplicationManager1Service &) = delete;
    ApplicationManager1Service &operator=(ApplicationManager1Service &&) = delete;

    Q_PROPERTY(QList<QDBusObjectPath> List READ list NOTIFY listChanged)
    [[nodiscard]] QList<QDBusObjectPath> list() const;

    void initService(QDBusConnection &connection) noexcept;
    QSharedPointer<ApplicationService> addApplication(DesktopFile desktopFileSource) noexcept;
    void removeOneApplication(const QString &appId) noexcept;
    void removeAllApplication() noexcept;
    [[nodiscard]] QHash<QDBusObjectPath, QSharedPointer<ApplicationService>>
    findApplicationsByIds(const QStringList &appIds) const noexcept;
    void updateApplication(const QSharedPointer<ApplicationService> &destApp, DesktopFile desktopFile) noexcept;

    [[nodiscard]] const auto &Applications() const noexcept { return m_applicationList; }
    [[nodiscard]] JobManager1Service &jobManager() noexcept { return *m_jobManager; }
    [[nodiscard]] const JobManager1Service &jobManager() const noexcept { return *m_jobManager; }
    [[nodiscard]] const QStringList &applicationHooks() const noexcept { return m_hookElements; }
    [[nodiscard]] MimeManager1Service &mimeManager() noexcept { return *m_mimeManager; }
    [[nodiscard]] const MimeManager1Service &mimeManager() const noexcept { return *m_mimeManager; }
    [[nodiscard]] const QStringList &systemdPathEnv() const noexcept { return m_systemdPathEnv; }

public Q_SLOTS:
    QString Identify(const QDBusUnixFileDescriptor &pidfd,
                     QDBusObjectPath &instance,
                     ObjectInterfaceMap &application_instance_info) const noexcept;
    void ReloadApplications();
    QString addUserApplication(const QVariantMap &desktop_file, const QString &name) noexcept;
    [[nodiscard]] ObjectMap GetManagedObjects() const;

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &object_path, const ObjectInterfaceMap &interfaces);
    void InterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void listChanged();

private Q_SLOTS:
    void doReloadApplications();
private:
    std::unique_ptr<Identifier> m_identifier;
    std::weak_ptr<ApplicationManager1Storage> m_storage;
    QScopedPointer<MimeManager1Service> m_mimeManager{nullptr};
    QScopedPointer<JobManager1Service> m_jobManager{nullptr};
    QStringList m_hookElements;
    QStringList m_systemdPathEnv;
    QFileSystemWatcher m_watcher;
    QTimer m_reloadTimer;
    QHash<QString, QSharedPointer<ApplicationService>> m_applicationList;

    void scanMimeInfos() noexcept;
    void scanApplications() noexcept;
    void scanInstances() noexcept;
    QHash<QSharedPointer<ApplicationService>, QString> scanAutoStart() noexcept;
    void loadHooks() noexcept;
    void addInstanceToApplication(const QString &unitName, const QDBusObjectPath &systemdUnitPath);
    void removeInstanceFromApplication(const QString &unitName, const QDBusObjectPath &systemdUnitPath);
};

#endif
