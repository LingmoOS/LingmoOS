// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "dockpanel.h"
#include "constants.h"
#include "dockiteminfo.h"

#include <QObject>
#include <QDBusContext>
#include <QDBusArgument>

/** this class used for old dock api compatible
  * it will forward old dbus call to new implementation
  */
namespace dock {
class DockDBusProxy final: public QObject, public QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry FINAL)
    Q_PROPERTY(QRect FrontendWindowRect READ frontendWindowRect FINAL)
    Q_PROPERTY(Position Position READ position WRITE setPosition FINAL)

    Q_PROPERTY(HideMode HideMode READ hideMode WRITE setHideMode FINAL)
    Q_PROPERTY(HideState HideState READ hideState FINAL)
    Q_PROPERTY(uint WindowSizeEfficient READ windowSizeEfficient WRITE setWindowSizeEfficient)
    Q_PROPERTY(uint WindowSizeFashion READ windowSizeFashion WRITE setWindowSizeFashion)
    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode FINAL)
    Q_PROPERTY(bool showInPrimary READ showInPrimary WRITE setShowInPrimary FINAL)


public:
    DockDBusProxy(DockPanel* parent = nullptr);

    void setItemOnDock(const QString &settingKey, const QString &itemKey, bool visible);
    void setPluginVisible(const QString &pluginName, bool visible);
    bool getPluginVisible(const QString &pluginName);
    QString getPluginKey(const QString &pluginName);
    void resizeDock(int offset, bool dragging);
    QStringList GetLoadedPlugins();
    DockItemInfos plugins();
    void ReloadPlugins();
    void callShow();

    QRect geometry();
    QRect frontendWindowRect();

    Position position();
    void setPosition(Position position);

    HideMode hideMode();
    void setHideMode(HideMode mode);

    HideState hideState();

    uint windowSizeEfficient();
    void setWindowSizeEfficient(uint size);

    uint windowSizeFashion();
    void setWindowSizeFashion(uint size);

    int displayMode();
    void setDisplayMode(int displayMode);

    bool RequestDock(const QString &desktopFile, int index);
    bool IsDocked(const QString &desktopFile);
    bool RequestUndock(const QString &desktopFile);

    bool showInPrimary() const;
    void setShowInPrimary(bool newShowInPrimary);

Q_SIGNALS:
    void pluginVisibleChanged(const QString &pluginName, bool visible) const;

private:
    DockPanel* parent() const;
    QString getAppID(const QString &desktopfile);
    QList<DS_NAMESPACE::DApplet *> appletList(const QString &pluginId) const;
    DS_NAMESPACE::DApplet *applet(const QString &pluginId) const;
    void setPluginVisible(const QString &pluginId, const QVariantMap &pluginsVisible);

    DS_NAMESPACE::DApplet *m_oldDockApplet;
    DS_NAMESPACE::DApplet *m_clipboardApplet;
    DS_NAMESPACE::DApplet *m_searchApplet;
    DS_NAMESPACE::DApplet *m_multitaskviewApplet;
    DS_NAMESPACE::DApplet *m_trayApplet;
};
}

