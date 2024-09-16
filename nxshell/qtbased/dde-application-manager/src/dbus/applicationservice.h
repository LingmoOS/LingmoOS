// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONSERVICE_H
#define APPLICATIONSERVICE_H

#include "applicationmanager1service.h"
#include "applicationmanagerstorage.h"
#include "dbus/applicationmanager1service.h"
#include "dbus/instanceservice.h"
#include "dbus/jobmanager1service.h"
#include "desktopentry.h"
#include "global.h"
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QFile>
#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTextStream>
#include <QUuid>
#include <memory>

struct AutostartSource
{
    QString m_filePath;
    DesktopEntry m_entry;
};

class ApplicationService : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    ~ApplicationService() override;
    ApplicationService(const ApplicationService &) = delete;
    ApplicationService(ApplicationService &&) = delete;
    ApplicationService &operator=(const ApplicationService &) = delete;
    ApplicationService &operator=(ApplicationService &&) = delete;

    Q_PROPERTY(QStringList Categories READ categories NOTIFY categoriesChanged)
    [[nodiscard]] QStringList categories() const noexcept;

    Q_PROPERTY(QStringList Actions READ actions NOTIFY actionsChanged)
    [[nodiscard]] QStringList actions() const noexcept;

    Q_PROPERTY(PropMap ActionName READ actionName NOTIFY actionNameChanged)
    [[nodiscard]] PropMap actionName() const noexcept;

    Q_PROPERTY(QString ID READ id CONSTANT)
    [[nodiscard]] QString id() const noexcept;

    Q_PROPERTY(QStringMap Name READ name NOTIFY nameChanged)
    [[nodiscard]] QStringMap name() const noexcept;

    Q_PROPERTY(QStringMap GenericName READ genericName NOTIFY genericNameChanged)
    [[nodiscard]] QStringMap genericName() const noexcept;

    Q_PROPERTY(QStringMap Icons READ icons NOTIFY iconsChanged)
    [[nodiscard]] QStringMap icons() const noexcept;

    Q_PROPERTY(qint64 LastLaunchedTime READ lastLaunchedTime NOTIFY lastLaunchedTimeChanged)
    [[nodiscard]] qint64 lastLaunchedTime() const noexcept;

    Q_PROPERTY(qint64 LaunchedTimes READ launchedTimes NOTIFY launchedTimesChanged)
    [[nodiscard]] qint64 launchedTimes() const noexcept;

    Q_PROPERTY(QString Environ READ environ WRITE setEnviron NOTIFY environChanged)
    [[nodiscard]] QString environ() const noexcept;
    void setEnviron(const QString &value) noexcept;

    Q_PROPERTY(bool Terminal READ terminal NOTIFY terminalChanged)
    [[nodiscard]] bool terminal() const noexcept;

    Q_PROPERTY(QString StartupWMClass READ startupWMClass NOTIFY startupWMClassChanged)
    [[nodiscard]] QString startupWMClass() const noexcept;

    // FIXME:
    // This property should implement with fuse guarded
    // $XDG_CONFIG_HOME/autostart/. Current implementation has some problems,
    Q_PROPERTY(bool AutoStart READ isAutoStart WRITE setAutoStart NOTIFY autostartChanged)
    [[nodiscard]] bool isAutoStart() const noexcept;
    void setAutoStart(bool autostart) noexcept;

    Q_PROPERTY(QStringList MimeTypes READ mimeTypes WRITE setMimeTypes)
    [[nodiscard]] QStringList mimeTypes() const noexcept;
    void setMimeTypes(const QStringList &value) noexcept;

    Q_PROPERTY(QList<QDBusObjectPath> Instances READ instances NOTIFY instanceChanged)
    [[nodiscard]] QList<QDBusObjectPath> instances() const noexcept;

    Q_PROPERTY(bool X_Flatpak READ x_Flatpak NOTIFY x_FlatpakChanged)
    [[nodiscard]] bool x_Flatpak() const noexcept;

    Q_PROPERTY(bool X_linglong READ x_linglong NOTIFY x_linglongChanged)
    [[nodiscard]] bool x_linglong() const noexcept;

    Q_PROPERTY(QString X_Deepin_Vendor READ X_Deepin_Vendor)
    [[nodiscard]] QString X_Deepin_Vendor() const noexcept;

    Q_PROPERTY(qint64 InstalledTime READ installedTime NOTIFY installedTimeChanged)
    [[nodiscard]] qint64 installedTime() const noexcept;

    Q_PROPERTY(bool isOnDesktop READ isOnDesktop NOTIFY isOnDesktopChanged)
    [[nodiscard]] bool isOnDesktop() const noexcept;

    Q_PROPERTY(bool NoDisplay READ noDisplay NOTIFY noDisplayChanged)
    [[nodiscard]] bool noDisplay() const noexcept;

    [[nodiscard]] QDBusObjectPath findInstance(const QString &instanceId) const;

    [[nodiscard]] const QString &getLauncher() const noexcept { return m_launcher; }
    void setLauncher(const QString &launcher) noexcept { m_launcher = launcher; }

    bool addOneInstance(const QString &instanceId,
                        const QString &application,
                        const QString &systemdUnitPath,
                        const QString &launcher) noexcept;
    void recoverInstances(const QList<QDBusObjectPath> &instanceList) noexcept;
    void removeOneInstance(const QDBusObjectPath &instance) noexcept;
    void removeAllInstance() noexcept;
    [[nodiscard]] const QDBusObjectPath &applicationPath() const noexcept { return m_applicationPath; }
    [[nodiscard]] DesktopFile &desktopFileSource() noexcept { return m_desktopSource; }
    [[nodiscard]] const QHash<QDBusObjectPath, QSharedPointer<InstanceService>> &applicationInstances() const noexcept
    {
        return m_Instances;
    }
    void resetEntry(DesktopEntry *newEntry) noexcept;
    void detachAllInstance() noexcept;
    [[nodiscard]] QVariant findEntryValue(const QString &group,
                                          const QString &valueKey,
                                          EntryValueType type,
                                          const QLocale &locale = getUserLocale()) const noexcept;

    [[nodiscard]] LaunchTask unescapeExec(const QString &str, const QStringList &fields) noexcept;
    [[nodiscard]] static std::optional<QStringList> unescapeExecArgs(const QString &str) noexcept;

public Q_SLOTS:
    // NOTE: 'realExec' only for internal implementation
    QDBusObjectPath
    Launch(const QString &action, const QStringList &fields, const QVariantMap &options, const QString &realExec = {});
    [[nodiscard]] ObjectMap GetManagedObjects() const;
    [[nodiscard]] bool SendToDesktop() const noexcept;
    [[nodiscard]] bool RemoveFromDesktop() const noexcept;

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &object_path, const ObjectInterfaceMap &interfaces);
    void InterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void autostartChanged();
    void noDisplayChanged();
    void isOnDesktopChanged();
    void installedTimeChanged();
    void x_FlatpakChanged();
    void x_linglongChanged();
    void instanceChanged();
    void lastLaunchedTimeChanged();
    void iconsChanged();
    void nameChanged();
    void genericNameChanged();
    void actionNameChanged();
    void actionsChanged();
    void categoriesChanged();
    void MimeTypesChanged();
    void terminalChanged();
    void environChanged();
    void launchedTimesChanged();
    void startupWMClassChanged();

private:
    friend class ApplicationManager1Service;
    explicit ApplicationService(DesktopFile source,
                                ApplicationManager1Service *parent,
                                std::weak_ptr<ApplicationManager1Storage> storage);
    static QSharedPointer<ApplicationService> createApplicationService(
        DesktopFile source, ApplicationManager1Service *parent, std::weak_ptr<ApplicationManager1Storage> storage) noexcept;
    qint64 m_lastLaunch{0};
    qint64 m_installedTime{0};
    qint64 m_launchedTimes{0};
    QString m_environ;
    std::weak_ptr<ApplicationManager1Storage> m_storage;
    AutostartSource m_autostartSource;
    QDBusObjectPath m_applicationPath;
    QString m_launcher{getApplicationLauncherBinary()};
    DesktopFile m_desktopSource;
    QSharedPointer<DesktopEntry> m_entry{nullptr};
    QHash<QDBusObjectPath, QSharedPointer<InstanceService>> m_Instances;
    void updateAfterLaunch(bool isLaunch) noexcept;
    static bool shouldBeShown(const std::unique_ptr<DesktopEntry> &entry) noexcept;
    [[nodiscard]] bool autostartCheck(const QString &filePath) const noexcept;
    void setAutostartSource(AutostartSource &&source) noexcept;
    void appendExtraEnvironments(QVariantMap &runtimeOptions) const noexcept;
    [[nodiscard]] ApplicationManager1Service *parent() { return dynamic_cast<ApplicationManager1Service *>(QObject::parent()); }
    [[nodiscard]] const ApplicationManager1Service *parent() const
    {
        return dynamic_cast<ApplicationManager1Service *>(QObject::parent());
    }
};

#endif
