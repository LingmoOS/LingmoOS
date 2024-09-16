// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearancedbusproxy.h"

#include <QDBusPendingReply>

const QString DaemonService = QStringLiteral("org.deepin.dde.Daemon1");
const QString DaemonPath = QStringLiteral("/org/deepin/dde/Daemon1");
const QString DaemonInterface = QStringLiteral("org.deepin.dde.Daemon1");

AppearanceDBusProxy::AppearanceDBusProxy(QObject *parent)
    : QObject(parent)
    , m_wmInterface(new DDBusInterface("com.deepin.wm", "/com/deepin/wm", "com.deepin.wm", QDBusConnection::sessionBus(), this))
    , m_displayInterface(new DDBusInterface("org.deepin.dde.Display1", "/org/deepin/dde/Display1", "org.deepin.dde.Display1", QDBusConnection::sessionBus(), this))
    , m_xSettingsInterface(new DDBusInterface("org.deepin.dde.XSettings1", "/org/deepin/dde/XSettings1", "org.deepin.dde.XSettings1", QDBusConnection::sessionBus(), this))
    , m_timeDateInterface(new DDBusInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus(), this))
    , m_nid(0)
{
    registerScaleFactorsMetaType();
    QDBusConnection::systemBus().connect(DaemonService, DaemonPath, DaemonInterface, "HandleForSleep", this, SIGNAL(HandleForSleep(bool)));
    QDBusConnection::sessionBus().connect(QStringLiteral("org.deepin.dde.Timedate1"), QStringLiteral("/org/deepin/dde/Timedate1"), QStringLiteral("org.deepin.dde.Timedate1"), "TimeUpdate", this, SIGNAL(TimeUpdate()));
}

void AppearanceDBusProxy::setUserInterface(const QString &userPath)
{
    m_userInterface = QSharedPointer<DDBusInterface>(new DDBusInterface("org.deepin.dde.Accounts1",
                                                                            userPath,
                                                                            "org.deepin.dde.Accounts1.User",
                                                                            QDBusConnection::systemBus(), this));
}

void AppearanceDBusProxy::Notify(const QString &in0, const QString &in2, const QString &summary, const QString &body, const QStringList &options, const QVariantMap &optionMap, int expireTimeout)
{
    QDBusMessage notifyMessage = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                                "/org/freedesktop/Notifications",
                                                                "org.freedesktop.Notifications", "Notify");
    notifyMessage << in0 << m_nid << in2
                  << summary << body << options
                  << optionMap << expireTimeout;
    QDBusConnection::sessionBus().callWithCallback(notifyMessage, this, SLOT(NotifyNid(uint)));
}

void AppearanceDBusProxy::NotifyNid(uint id)
{
    m_nid = id;
}
// wmInterface
QString AppearanceDBusProxy::cursorTheme()
{
    return qvariant_cast<QString>(m_wmInterface->property("cursorTheme"));
}

void AppearanceDBusProxy::setcursorTheme(const QString &cursorTheme)
{
    m_wmInterface->setProperty("cursorTheme", cursorTheme);
}

int AppearanceDBusProxy::WorkspaceCount()
{
    return QDBusPendingReply<int>(m_wmInterface->asyncCall(QStringLiteral("WorkspaceCount")));
}
QString AppearanceDBusProxy::GetWorkspaceBackgroundForMonitor(int index, const QString &strMonitorName)
{
    return QDBusPendingReply<QString>(m_wmInterface->asyncCall(QStringLiteral("GetWorkspaceBackgroundForMonitor"), index, strMonitorName));
}
void AppearanceDBusProxy::SetCurrentWorkspaceBackgroundForMonitor(const QString &uri, const QString &strMonitorName)
{
    m_wmInterface->asyncCall(QStringLiteral("SetCurrentWorkspaceBackgroundForMonitor"), uri, strMonitorName);
}
void AppearanceDBusProxy::SetDecorationDeepinTheme(const QString &deepinThemeName)
{
    m_wmInterface->asyncCall(QStringLiteral("SetDecorationDeepinTheme"), deepinThemeName);
}
void AppearanceDBusProxy::ChangeCurrentWorkspaceBackground(const QString &uri)
{
    m_wmInterface->asyncCall(QStringLiteral("ChangeCurrentWorkspaceBackground"), uri);
}
int AppearanceDBusProxy::GetCurrentWorkspace()
{
    return QDBusPendingReply<int>(m_wmInterface->asyncCall(QStringLiteral("GetCurrentWorkspace")));
}

void AppearanceDBusProxy::SetWorkspaceBackgroundForMonitor(int index, const QString &strMonitorName, const QString &uri)
{
    m_wmInterface->asyncCall(QStringLiteral("SetWorkspaceBackgroundForMonitor"), index, strMonitorName, uri);
}
// displayInterface
QString AppearanceDBusProxy::primary()
{
    return qvariant_cast<QString>(m_displayInterface->property("Primary"));
}

QList<QDBusObjectPath> AppearanceDBusProxy::monitors()
{
    return qvariant_cast<QList<QDBusObjectPath>>(m_displayInterface->property("Monitors"));
}

QStringList AppearanceDBusProxy::ListOutputNames()
{
    return QDBusPendingReply<QStringList>(m_displayInterface->asyncCall(QStringLiteral("ListOutputNames")));
}
// xSettingsInterface
void AppearanceDBusProxy::SetString(const QString &prop, const QString &v)
{
    m_xSettingsInterface->asyncCall(QStringLiteral("SetString"), prop, v);
}
void AppearanceDBusProxy::SetInteger(const QString &prop, const int &v)
{
    m_xSettingsInterface->asyncCall(QStringLiteral("SetInteger"), prop, v);
}
double AppearanceDBusProxy::GetScaleFactor()
{
    return QDBusPendingReply<double>(m_xSettingsInterface->asyncCall(QStringLiteral("GetScaleFactor")));
}
void AppearanceDBusProxy::SetScaleFactor(double scale)
{
    m_xSettingsInterface->asyncCall(QStringLiteral("SetScaleFactor"), scale);
}
ScaleFactors AppearanceDBusProxy::GetScreenScaleFactors()
{
    return QDBusPendingReply<ScaleFactors>(m_xSettingsInterface->asyncCall(QStringLiteral("GetScreenScaleFactors")));
}
void AppearanceDBusProxy::SetScreenScaleFactors(const ScaleFactors &factors)
{
    m_xSettingsInterface->asyncCall(QStringLiteral("SetScreenScaleFactors"), QVariant::fromValue(factors));
}

QString AppearanceDBusProxy::FindUserById(const QString &uid)
{
    QDBusMessage accountsMessage = QDBusMessage::createMethodCall("org.deepin.dde.Accounts1", "/org/deepin/dde/Accounts1", "org.deepin.dde.Accounts1", "FindUserById");
    accountsMessage << uid;
    return QDBusPendingReply<QString>(QDBusConnection::systemBus().asyncCall(accountsMessage));
}
// userInterface
QStringList AppearanceDBusProxy::desktopBackgrounds()
{
    if (m_userInterface.isNull())
        return QStringList();
    return qvariant_cast<QStringList>(m_userInterface->property("DesktopBackgrounds"));
}

QString AppearanceDBusProxy::greeterBackground()
{
    if (m_userInterface.isNull())
        return QString();
    return qvariant_cast<QString>(m_userInterface->property("GreeterBackground"));
}

void AppearanceDBusProxy::SetCurrentWorkspace(int currentWorkspace)
{
    if (!m_userInterface.isNull())
        m_userInterface->asyncCall(QStringLiteral("SetCurrentWorkspace"), currentWorkspace);
}

void AppearanceDBusProxy::SetDesktopBackgrounds(const QStringList &val)
{
    if (!m_userInterface.isNull())
        m_userInterface->asyncCall(QStringLiteral("SetDesktopBackgrounds"), val);
}

void AppearanceDBusProxy::SetGreeterBackground(const QString &bg)
{
    if (!m_userInterface.isNull())
        m_userInterface->asyncCall(QStringLiteral("SetGreeterBackground"), bg);
}
// timeDateInterface
QString AppearanceDBusProxy::timezone()
{
    return qvariant_cast<QString>(m_timeDateInterface->property("Timezone"));
}

bool AppearanceDBusProxy::nTP()
{
    return qvariant_cast<bool>(m_timeDateInterface->property("NTP"));
}

// imageBlurInterface
void AppearanceDBusProxy::Delete(const QString &file)
{
    QDBusMessage imageBlurMessage = QDBusMessage::createMethodCall("org.deepin.dde.ImageBlur1", "/org/deepin/dde/ImageBlur1", "org.deepin.dde.ImageBlur1", "Delete");
    imageBlurMessage << file;
    QDBusConnection::systemBus().asyncCall(imageBlurMessage);
}

QString AppearanceDBusProxy::Get(const QString &file)
{
    QDBusMessage imageBlurMessage = QDBusMessage::createMethodCall("org.deepin.dde.ImageBlur1", "/org/deepin/dde/ImageBlur1", "org.deepin.dde.ImageBlur1", "Get");
    imageBlurMessage << file;
    return QDBusPendingReply<QString>(QDBusConnection::systemBus().asyncCall(imageBlurMessage));
}
// imageEffectInterface
void AppearanceDBusProxy::Delete(const QString &effect, const QString &filename)
{
    QDBusMessage imageEffectMessage = QDBusMessage::createMethodCall("org.deepin.dde.ImageEffect1", "/org/deepin/dde/ImageEffect1", "org.deepin.dde.ImageEffect1", "Delete");
    imageEffectMessage << effect << filename;
    QDBusConnection::systemBus().asyncCall(imageEffectMessage);
}

QString AppearanceDBusProxy::Get(const QString &effect, const QString &filename)
{
    QDBusMessage imageEffectMessage = QDBusMessage::createMethodCall("org.deepin.dde.ImageEffect1", "/org/deepin/dde/ImageEffect1", "org.deepin.dde.ImageEffect1", "Get");
    imageEffectMessage << effect << filename;
    QDBusConnection::systemBus().asyncCall(imageEffectMessage);

    return QDBusPendingReply<QString>(QDBusConnection::systemBus().asyncCall(imageEffectMessage));
}
// Daemon1
void AppearanceDBusProxy::DeleteCustomWallPaper(const QString &username, const QString &file)
{
    QDBusMessage daemonMessage = QDBusMessage::createMethodCall(DaemonService, DaemonPath, DaemonInterface, "DeleteCustomWallPaper");
    daemonMessage << username << file;
    QDBusConnection::systemBus().asyncCall(daemonMessage);
}

QStringList AppearanceDBusProxy::GetCustomWallPapers(const QString &username)
{
    QDBusMessage daemonMessage = QDBusMessage::createMethodCall(DaemonService, DaemonPath, DaemonInterface, "GetCustomWallPapers");
    daemonMessage << username;
    return QDBusPendingReply<QStringList>(QDBusConnection::systemBus().asyncCall(daemonMessage));
}

QString AppearanceDBusProxy::SaveCustomWallPaper(const QString &username, const QString &file)
{
    QDBusMessage daemonMessage = QDBusMessage::createMethodCall(DaemonService, DaemonPath, DaemonInterface, "SaveCustomWallPaper");
    daemonMessage << username << file;
    return QDBusPendingReply<QString>(QDBusConnection::systemBus().asyncCall(daemonMessage));
}
