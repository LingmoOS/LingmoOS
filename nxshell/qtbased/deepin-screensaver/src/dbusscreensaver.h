// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSSCREENSAVER_H
#define DBUSSCREENSAVER_H

#include <com_deepin_sessionmanager.h>

#include <DConfig>

#include <QObject>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QDBusMessage>
#include <QScreen>
#include <QAtomicInteger>

DCORE_USE_NAMESPACE

class ScreenSaverWindow;
class X11EventFilter;
class DBusScreenSaver : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList allScreenSaver READ allScreenSaver NOTIFY allScreenSaverChanged)
    Q_PROPERTY(int batteryScreenSaverTimeout READ batteryScreenSaverTimeout WRITE setBatteryScreenSaverTimeout NOTIFY batteryScreenSaverTimeoutChanged)
    Q_PROPERTY(int linePowerScreenSaverTimeout READ linePowerScreenSaverTimeout WRITE setLinePowerScreenSaverTimeout NOTIFY linePowerScreenSaverTimeoutChanged)
    Q_PROPERTY(QString currentScreenSaver READ currentScreenSaver WRITE setCurrentScreenSaver NOTIFY currentScreenSaverChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(bool lockScreenAtAwake READ lockScreenAtAwake WRITE setLockScreenAtAwake NOTIFY lockScreenAtAwakeChanged)
    Q_PROPERTY(int lockScreenDelay READ lockScreenDelay WRITE setLockScreenDelay NOTIFY lockScreenDelayChanged)

public:
    explicit DBusScreenSaver(QObject *parent = nullptr);
    ~DBusScreenSaver();

    bool Preview(const QString &name, int staysOn = 1, bool preview = true);
    QString GetScreenSaverCover(const QString &name) const;
    void RefreshScreenSaverList();
    void Start(const QString &name = QString());
    void Stop(bool lock = false);
    void stop();

    QStringList allScreenSaver() const;
    int batteryScreenSaverTimeout() const;
    int linePowerScreenSaverTimeout() const;
    QString currentScreenSaver() const;
    bool isRunning() const;

    void setBatteryScreenSaverTimeout(int batteryScreenSaverTimeout);
    void setLinePowerScreenSaverTimeout(int linePowerScreenSaverTimeout);
    void setCurrentScreenSaver(QString currentScreenSaver);

    bool lockScreenAtAwake() const;
    int lockScreenDelay() const;
    void setLockScreenAtAwake(bool lockScreenAtAwake);
    void setLockScreenDelay(int lockScreenDelay);

    bool StartCustomConfig(const QString &name);
    QStringList ConfigurableItems();
    bool IsConfigurable(const QString &name);
signals:
    void allScreenSaverChanged(QStringList allScreenSaver);
    void batteryScreenSaverTimeoutChanged(int batteryScreenSaverTimeout);
    void linePowerScreenSaverTimeoutChanged(int linePowerScreenSaverTimeout);
    void currentScreenSaverChanged(QString currentScreenSaver);
    void isRunningChanged(bool isRunning);
    void lockScreenAtAwakeChanged(bool lockScreenAtAwake);
    void lockScreenDelayChanged(int lockScreenDelay);

private:
    Q_SLOT void onDBusPropertyChanged(const QString &interface, const QVariantMap &changed_properties, const QDBusMessage &message);
    Q_SLOT void onConfigChanged(const QString &key);

    void clearResourceList();
    void ensureWindowMap();
    void onScreenAdded(QScreen *s);
    void cleanWindow(ScreenSaverWindow *w);

    void XGrabKeyBoard();
    void XUnGrabKeyBoard();
    void onLockedChanged(const bool locked);

    QList<QDir> m_resourceDirList;
    QStringList m_resourceList;

    QList<QDir> m_moduleDirList;
    QStringList m_screenSaverList;
    QMap<QString, QDir> m_screenSaverNameToDir;
    QString m_currentScreenSaver;
    bool m_lockScreenAtAwake;
    int m_lockScreenDelay;
    QTimer m_lockScreenTimer;

    QMap<QScreen*, ScreenSaverWindow*> m_windowMap;
    QTimer m_autoQuitTimer;
    QSettings m_settings;
    QScopedPointer<X11EventFilter> x11event;

    com::deepin::SessionManager *m_sessionManagerInter = nullptr;
    QAtomicInteger<bool> m_grabKeyboard = false;

    DConfig *m_dcfg;
};

#endif // DBUSSCREENSAVER_H
