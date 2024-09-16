// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMDBUSDOCKINTERFACT_H
#define AMDBUSDOCKINTERFACT_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#define INTERFACE_NAME "org.deepin.dde.daemon.Dock1"
#define SERVICE_PATH "/org/deepin/dde/daemon/Dock1"

class DockPrivate;
class AMDBusDockInter : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    static inline const char *staticInterfaceName()
    { return INTERFACE_NAME; }

public:
    explicit AMDBusDockInter(QObject *parent = Q_NULLPTR);
    ~AMDBusDockInter();

    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode NOTIFY DisplayModeChanged)
    int displayMode();
    void setDisplayMode(int value);

    Q_PROPERTY(QStringList DockedApps READ dockedApps NOTIFY DockedAppsChanged)
    QStringList dockedApps();

    Q_PROPERTY(QList<QDBusObjectPath> Entries READ entries NOTIFY EntriesChanged)
    QList<QDBusObjectPath> entries();

    Q_PROPERTY(QRect FrontendWindowRect READ frontendWindowRect NOTIFY FrontendWindowRectChanged)
    QRect frontendWindowRect();

    Q_PROPERTY(int HideMode READ hideMode WRITE setHideMode NOTIFY HideModeChanged)
    int hideMode();
    void setHideMode(int value);

    Q_PROPERTY(int HideState READ hideState NOTIFY HideStateChanged)
    int hideState();

    Q_PROPERTY(uint HideTimeout READ hideTimeout WRITE setHideTimeout NOTIFY HideTimeoutChanged)
    uint hideTimeout();
    void setHideTimeout(uint value);

    Q_PROPERTY(uint IconSize READ iconSize WRITE setIconSize NOTIFY IconSizeChanged)
    uint iconSize();
    void setIconSize(uint value);

    Q_PROPERTY(double Opacity READ opacity WRITE setOpacity NOTIFY OpacityChanged)
    double opacity();
    void setOpacity(double value);

    Q_PROPERTY(int Position READ position WRITE setPosition NOTIFY PositionChanged)
    int position();
    void setPosition(int value);

    Q_PROPERTY(uint ShowTimeout READ showTimeout WRITE setShowTimeout NOTIFY ShowTimeoutChanged)
    uint showTimeout();
    void setShowTimeout(uint value);

    Q_PROPERTY(uint WindowSize READ windowSize WRITE setWindowSize NOTIFY WindowSizeChanged)
    uint windowSize();
    void setWindowSize(uint value);

    Q_PROPERTY(uint WindowSizeEfficient READ windowSizeEfficient WRITE setWindowSizeEfficient NOTIFY WindowSizeEfficientChanged)
    uint windowSizeEfficient();
    void setWindowSizeEfficient(uint value);

    Q_PROPERTY(uint WindowSizeFashion READ windowSizeFashion WRITE setWindowSizeFashion NOTIFY WindowSizeFashionChanged)
    uint windowSizeFashion();
    void setWindowSizeFashion(uint value);

public Q_SLOTS: // METHODS

    inline void ActivateWindowQueued(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("ActivateWindow"), argumentList);
    }

    inline QDBusPendingReply<> CloseWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("CloseWindow"), argumentList);
    }

    inline void CloseWindowQueued(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("CloseWindow"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetDockedAppsDesktopFiles()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetDockedAppsDesktopFiles"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetEntryIDs()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetEntryIDs"), argumentList);
    }

    inline QDBusPendingReply<QString> GetPluginSettings()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetPluginSettings"), argumentList);
    }

    inline QDBusPendingReply<bool> IsDocked(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsDocked"), argumentList);
    }

    inline QDBusPendingReply<bool> IsOnDock(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsOnDock"), argumentList);
    }

    inline QDBusPendingReply<> MergePluginSettings(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MergePluginSettings"), argumentList);
    }

    inline void MergePluginSettingsQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("MergePluginSettings"), argumentList);
    }

    inline QDBusPendingReply<> MoveEntry(int in0, int in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("MoveEntry"), argumentList);
    }

    inline void MoveEntryQueued(int in0, int in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);

        CallQueued(QStringLiteral("MoveEntry"), argumentList);
    }

    inline QDBusPendingReply<> MoveWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MoveWindow"), argumentList);
    }

    inline void MoveWindowQueued(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("MoveWindow"), argumentList);
    }

    inline QDBusPendingReply<QString> QueryWindowIdentifyMethod(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("QueryWindowIdentifyMethod"), argumentList);
    }

    inline QDBusPendingReply<> RemovePluginSettings(const QString &in0, const QStringList &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RemovePluginSettings"), argumentList);
    }

    inline void RemovePluginSettingsQueued(const QString &in0, const QStringList &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);

        CallQueued(QStringLiteral("RemovePluginSettings"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestDock(const QString &in0, int in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RequestDock"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestUndock(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RequestUndock"), argumentList);
    }

    inline QDBusPendingReply<> SetFrontendWindowRect(int in0, int in1, uint in2, uint in3)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3);
        return asyncCallWithArgumentList(QStringLiteral("SetFrontendWindowRect"), argumentList);
    }

    inline void SetFrontendWindowRectQueued(int in0, int in1, uint in2, uint in3)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3);

        CallQueued(QStringLiteral("SetFrontendWindowRect"), argumentList);
    }

    inline QDBusPendingReply<> SetPluginSettings(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetPluginSettings"), argumentList);
    }

    inline void SetPluginSettingsQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("SetPluginSettings"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void DockAppSettingsSynced();
    void EntryAdded(const QDBusObjectPath &in0, int in1);
    void EntryRemoved(const QString &in0);
    void PluginSettingsSynced();
    void ServiceRestarted();
    // begin property changed signals
    void DisplayModeChanged(int value) const;
    void DockedAppsChanged(const QStringList &value) const;
    void EntriesChanged(const QList<QDBusObjectPath> &value) const;
    void FrontendWindowRectChanged(QRect value) const;
    void HideModeChanged(int value) const;
    void HideStateChanged(int value) const;
    void HideTimeoutChanged(uint value) const;
    void IconSizeChanged(uint value) const;
    void OpacityChanged(double value) const;
    void PositionChanged(int value) const;
    void ShowTimeoutChanged(uint value) const;
    void WindowSizeChanged(uint value) const;
    void WindowSizeEfficientChanged(uint value) const;
    void WindowSizeFashionChanged(uint value) const;

public Q_SLOTS:
    void CallQueued(const QString &callName, const QList<QVariant> &args);

private Q_SLOTS:
    void onPendingCallFinished(QDBusPendingCallWatcher *w);
    void onPropertyChanged(const QDBusMessage& msg);

private:
    DockPrivate *d_ptr;
};

#endif
