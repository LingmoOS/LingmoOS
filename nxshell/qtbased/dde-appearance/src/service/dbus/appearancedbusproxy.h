// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPEARANCEDBUSPROXY_H
#define APPEARANCEDBUSPROXY_H

#include "scaleFactors.h"

#include <DDBusInterface>
#include <QSharedPointer>

using Dtk::Core::DDBusInterface;

class AppearanceDBusProxy : public QObject
{
    Q_OBJECT
public:
    explicit AppearanceDBusProxy(QObject *parent = nullptr);
    void setUserInterface(const QString &userPath);
    // Notifications
public Q_SLOTS:
    void Notify(const QString &in0, const QString &in2, const QString &summary, const QString &body, const QStringList &options, const QVariantMap &optionMap, int expireTimeout);
private Q_SLOTS:
    void NotifyNid(uint id);

    // wmInterface
public:
    Q_PROPERTY(QString cursorTheme READ cursorTheme WRITE setcursorTheme NOTIFY cursorThemeChanged)
    QString cursorTheme();
    void setcursorTheme(const QString &cursorTheme);

public Q_SLOTS:
    int WorkspaceCount();
    QString GetWorkspaceBackgroundForMonitor(int index, const QString &strMonitorName);
    void SetCurrentWorkspaceBackgroundForMonitor(const QString &uri, const QString &strMonitorName);
    void SetDecorationDeepinTheme(const QString &deepinThemeName);
    void ChangeCurrentWorkspaceBackground(const QString &uri);
    int GetCurrentWorkspace();
    void SetWorkspaceBackgroundForMonitor(int index, const QString &strMonitorName, const QString &uri);

Q_SIGNALS:
    void workspaceCountChanged(int count);
    void WorkspaceSwitched(int from, int to);
    void cursorThemeChanged(const QString &cursorTheme);

    // displayInterface
public:
    Q_PROPERTY(QString Primary READ primary NOTIFY PrimaryChanged)
    QString primary();
    Q_PROPERTY(QList<QDBusObjectPath> Monitors READ monitors NOTIFY MonitorsChanged)
    QList<QDBusObjectPath> monitors();

public Q_SLOTS:
    QStringList ListOutputNames();

Q_SIGNALS:
    void PrimaryChanged(const QString &Primary);
    void MonitorsChanged(QList<QDBusObjectPath> monitors);

    // xSettingsInterface
public Q_SLOTS:
    void SetString(const QString &prop, const QString &v);
    void SetInteger(const QString &prop, const int &v);
    double GetScaleFactor();
    void SetScaleFactor(double scale);
    ScaleFactors GetScreenScaleFactors();
    void SetScreenScaleFactors(const ScaleFactors &factors);

Q_SIGNALS:
    void SetScaleFactorStarted();
    void SetScaleFactorDone();

    // accountInter
public Q_SLOTS:
    static QString FindUserById(const QString &uid);

    // userInterface
public:
    Q_PROPERTY(QStringList DesktopBackgrounds READ desktopBackgrounds NOTIFY DesktopBackgroundsChanged)
    QStringList desktopBackgrounds();
    Q_PROPERTY(QString GreeterBackground READ greeterBackground NOTIFY GreeterBackgroundChanged)
    QString greeterBackground();

public Q_SLOTS:
    void SetCurrentWorkspace(int currentWorkspace);
    void SetDesktopBackgrounds(const QStringList &val);
    void SetGreeterBackground(const QString &bg);
Q_SIGNALS:
    void DesktopBackgroundsChanged(const QStringList &desktopBackgrounds);
    void GreeterBackgroundChanged(const QString &greeterBackground);
    // timeDateInterface
public:
    Q_PROPERTY(QString Timezone READ timezone NOTIFY TimezoneChanged)
    QString timezone();
    Q_PROPERTY(bool NTP READ nTP NOTIFY NTPChanged)
    bool nTP();

Q_SIGNALS:
    void TimezoneChanged(QString timezone);
    void NTPChanged(bool NTP);

public:
Q_SIGNALS:
    void TimeUpdate();

    // imageBlurInterface
public:
    static void Delete(const QString &file);
    static QString Get(const QString &file);

    // imageEffectInterface
public:
    static void Delete(const QString &effect, const QString &filename);
    static QString Get(const QString &effect, const QString &filename);

    // Daemon
public:
    static void DeleteCustomWallPaper(const QString &username, const QString &file);
    static QStringList GetCustomWallPapers(const QString &username);
    static QString SaveCustomWallPaper(const QString &username, const QString &file);

Q_SIGNALS:
    void HandleForSleep(bool sleep);

private:
    DDBusInterface *m_wmInterface;
    DDBusInterface *m_displayInterface;
    DDBusInterface *m_xSettingsInterface;
    DDBusInterface *m_timeDateInterface;
    QSharedPointer<DDBusInterface> m_userInterface;
    uint m_nid;
};

#endif // APPEARANCEDBUSPROXY_H
