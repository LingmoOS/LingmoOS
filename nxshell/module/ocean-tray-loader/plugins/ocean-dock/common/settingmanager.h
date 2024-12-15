// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QMap>

class SettingManager : public QObject
{
    Q_OBJECT

public:
    static SettingManager *instance();
    SettingManager(const SettingManager &) = delete;
    SettingManager &operator=(const SettingManager &) = delete;

    bool enableSafeMode() const;
    int delayIntervalOnHide() const;
    int showWindowName() const;
    int toggleDesktopInterval() const;
    bool alwaysHideDock() const;
    bool enableShowDesktop() const;
    QStringList dockedPlugins() const;

// Setter
public:
    void setPluginDocked(const QString &pluginName, bool docked);

signals:
    void onlyShowPrimaryChanged(bool);
    void alwaysHideDockChanged(bool);
    void toggleDesktopIntervalChanged(int);
    void dockedPluginsChanged(const QStringList &);
    void enableShowDesktopChanged(bool);

protected:
    explicit SettingManager(QObject *parent = nullptr);
    ~SettingManager();

private slots:
    void onDockConfigChanged(const QString &key);
    void onQuickPanelConfigChanged(const QString &key);

private:
    bool m_enableSafeMode;
    int m_delayIntervalOnHide;
    int m_showWindowName;
    int m_toggleDesktopInterval;
    bool m_alwaysHideDock;
    bool m_enableShowDesktop;
    QStringList m_dockedQuickPlugins;
};

#endif // CONFIGWATCHER_H
