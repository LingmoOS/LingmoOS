// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globals.h"
#include "abstractwindow.h"
#include "desktopfileamparser.h"
#include "desktopfileabstractparser.h"
#include "objectmanager1interface.h"

#include <unistd.h>
#include <sys/syscall.h>

#include <DDBusSender>
#include <QDBusConnection>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(amdesktopfileLog, "dde.shell.dock.amdesktopfile")

// AM static string
static const QString AM_DBUS_PATH = "org.desktopspec.ApplicationManager1";
static const QString DESKTOP_ENTRY_ICON_KEY = "Desktop Entry";
static const QString DEFAULT_KEY = "default";

static int pidfd_open(pid_t pid, uint flags)
{
    // WARNING pidfd_open only support on linux-5.3 or later
    return syscall(SYS_pidfd_open, pid, flags);
}

namespace dock {
static QDBusServiceWatcher dbusWatcher(AM_DBUS_PATH, QDBusConnection::sessionBus(),
                                                QDBusServiceWatcher::WatchForOwnerChange);
static ObjectManager desktopobjectManager(AM_DBUS_PATH, "/org/desktopspec/ApplicationManager1", QDBusConnection::sessionBus());


DesktopFileAMParser::DesktopFileAMParser(QString id, QObject* parent)
    : DesktopfileAbstractParser(id, parent)
{
    if (!m_amIsAvaliable) m_amIsAvaliable = QDBusConnection::sessionBus().
        interface()->isServiceRegistered(AM_DBUS_PATH);
    
    connect(&desktopobjectManager, &ObjectManager::InterfacesRemoved, this, [this] (const QDBusObjectPath& path, const QStringList& interfaces) {
        if (m_applicationInterface->path() == path.path()) {
            getAppItem()->setDocked(false);
            return;
        }
    });

    connect(&dbusWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](){
        m_amIsAvaliable = true;
        Q_EMIT iconChanged();
    });

    connect(&dbusWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](){
        m_amIsAvaliable = false;
        Q_EMIT iconChanged();
    });

    qCDebug(amdesktopfileLog()) << "create a am desktopfile object: " << m_id;
    m_applicationInterface.reset(new Application(AM_DBUS_PATH, id2dbusPath(id), QDBusConnection::sessionBus(), this));
    m_isValid = !m_id.isEmpty() && (m_applicationInterface->iD() == m_id);
}

DesktopFileAMParser::~DesktopFileAMParser()
{
    qCDebug(amdesktopfileLog()) << "destroy a am desktopfile object: " << m_id;
}

QString DesktopFileAMParser::id()
{
    if (!m_amIsAvaliable) return DesktopfileAbstractParser::id();

    if (m_id.isEmpty() && m_applicationInterface) {
        m_id = m_applicationInterface->iD();
    }
    return m_id;
}

QString DesktopFileAMParser::name()
{
    if (!m_amIsAvaliable) return DesktopfileAbstractParser::name();
    if (m_name.isEmpty() && m_applicationInterface) {
        updateLocalName();
    }
    return m_name;
}

QString DesktopFileAMParser::desktopIcon()
{
    if (!m_amIsAvaliable) return DesktopfileAbstractParser::desktopIcon();

    if(m_icon.isEmpty() && m_applicationInterface) {
        updateDesktopIcon();
    }

    if (m_icon.isEmpty()) return DesktopfileAbstractParser::desktopIcon();

    return m_icon;
}

QString DesktopFileAMParser::xDeepinVendor()
{
    if (!m_amIsAvaliable) return DesktopfileAbstractParser::xDeepinVendor();

    if (m_xDeepinVendor.isEmpty() && m_applicationInterface) {
       m_xDeepinVendor = m_applicationInterface->x_Deepin_Vendor();
    }

    return m_xDeepinVendor;
}

QString DesktopFileAMParser::genericName()
{
    if (!m_amIsAvaliable) return DesktopfileAbstractParser::genericName();

    if(m_genericName.isEmpty() && m_applicationInterface) {
        updateLocalGenericName();
    }

    return m_genericName;
}

std::pair<bool, QString> DesktopFileAMParser::isValied()
{
    // TODO:    1. am service may be unavailable and get dbus timeout
    //          2. show call path existed not id equeals
    return m_isValid ? std::make_pair(true, QStringLiteral("has am as desktopfileparser backend")) :
        std::make_pair(false, QStringLiteral("am do not contains app %1").arg(id()));
}

QList<QPair<QString, QString>> DesktopFileAMParser::actions()
{
    if (!m_amIsAvaliable) return DesktopfileAbstractParser::actions();

    if(m_actions.isEmpty() && m_applicationInterface) {
        updateActions();
    }
    return m_actions;
}

QString DesktopFileAMParser::id2dbusPath(const QString& id)
{
    return QStringLiteral("/org/desktopspec/ApplicationManager1/") + escapeToObjectPath(id);
}

QString DesktopFileAMParser::identifyWindow(QPointer<AbstractWindow> window)
{
    if (!m_amIsAvaliable) m_amIsAvaliable = QDBusConnection::sessionBus().
        interface()->isServiceRegistered(AM_DBUS_PATH);

    if (!m_amIsAvaliable) return QString();

    auto pidfd = pidfd_open(window->pid(),0);
    auto res = DDBusSender().service("org.desktopspec.ApplicationManager1")
                                         .interface("org.desktopspec.ApplicationManager1")
                                         .path("/org/desktopspec/ApplicationManager1")
                                         .method("Identify")
                                         .arg(QDBusUnixFileDescriptor(pidfd))
                                         .call();
    res.waitForFinished();
    close(pidfd);
    if (res.isValid()) {
        auto reply = res.reply();
        QList<QVariant> data = reply.arguments();
        return data.first().toString();
    }

    qCDebug(amdesktopfileLog()) << "AM failed to identify, reason is: " << res.error().message();

    return QString();
}

QString DesktopFileAMParser::identifyType()
{
    return QStringLiteral("amAPP");
}

QString DesktopFileAMParser::type()
{
    return identifyType();
}

void DesktopFileAMParser::launch()
{
    return launchByAMTool();
}

void DesktopFileAMParser::launchWithAction(const QString& action)
{
    return launchByAMTool(action);
}

void DesktopFileAMParser::requestQuit()
{

}

void DesktopFileAMParser::connectToAmDBusSignal(const QString& signalName, const char *slot)
{
    QDBusConnection::sessionBus().connect(
        m_applicationInterface->service(),
        m_applicationInterface->path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        "sa{sv}as",
        this,
        SLOT(onPropertyChanged(const QDBusMessage &))
    );
}

void DesktopFileAMParser::launchByAMTool(const QString &action)
{
    QProcess process;
    const auto path = m_applicationInterface->path();
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("dde-am", {"--by-user", path, action});
    if (!process.waitForFinished()) {
        qWarning() << "Failed to launch the path:" << path << process.errorString();
        return;
    } else if (process.exitCode() != 0) {
        qWarning() << "Failed to launch the path:" << path << process.readAll();
        return;
    }
    qDebug() << "Launch the application path:" << path;
}

void DesktopFileAMParser::updateActions()
{
    m_actions.clear();

    QString currentLanguageCode = QLocale::system().name();
    QList<QPair<QString, QString>> array;
    auto actions = m_applicationInterface->actions();
    auto actionNames = m_applicationInterface->actionName();

    for (auto action : actions) {
        auto localeName = actionNames.value(action).value(currentLanguageCode);
        auto fallbackDefaultName = actionNames.value(action).value(DEFAULT_KEY);
        m_actions.append({action, localeName.isEmpty() ? fallbackDefaultName : localeName});
    }
}

void DesktopFileAMParser::updateLocalName()
{
    QString currentLanguageCode = QLocale::system().name();
    auto names = m_applicationInterface->name();
    auto localeName = names.value(currentLanguageCode);
    auto fallbackName = names.value(DEFAULT_KEY);
    m_name = localeName.isEmpty() ? fallbackName : localeName;
}

void DesktopFileAMParser::updateDesktopIcon()
{
    m_icon = m_applicationInterface->icons().value(DESKTOP_ENTRY_ICON_KEY);
}

void DesktopFileAMParser::updateLocalGenericName()
{
    QString currentLanguageCode = QLocale::system().name();
    auto genericNames = m_applicationInterface->genericName();
    auto localeGenericName = genericNames.value(currentLanguageCode);
    auto fallBackGenericName = genericNames.value(DEFAULT_KEY);
    m_genericName = localeGenericName.isEmpty() ? fallBackGenericName : localeGenericName;
}

void DesktopFileAMParser::onPropertyChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != QStringLiteral("org.desktopspec.ApplicationManager1.Application"))
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    if (changedProps.contains("Name")) {
        updateLocalName();
        Q_EMIT nameChanged();
    } else if (changedProps.contains("Actions")) {
        updateActions();
        Q_EMIT actionsChanged();
    } else if (changedProps.contains("GenericName")) {
        updateLocalGenericName();
        Q_EMIT genericNameChanged();
    } else if (changedProps.contains("Name")) {
        updateLocalName();
        Q_EMIT nameChanged();
    } else if (changedProps.contains("X_Deepin_Vendor")) {
        m_xDeepinVendor = m_applicationInterface->x_Deepin_Vendor();
        Q_EMIT xDeepinVendorChanged();
    }
}
}
