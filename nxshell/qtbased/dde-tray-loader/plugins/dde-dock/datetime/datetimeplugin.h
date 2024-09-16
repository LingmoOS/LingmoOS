// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DATETIMEPLUGIN_H
#define DATETIMEPLUGIN_H

#include "pluginsiteminterface_v2.h"
#include "datetimewidget.h"
#include "sidebarcalendarwidget.h"
#include "tipswidget.h"
#include "regionFormat.h"

#include <QTimer>
#include <QLabel>
#include <QSettings>

class QDBusInterface;
class DatetimePlugin : public QObject, PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "datetime.json")

public:
    explicit DatetimePlugin(QObject *parent = nullptr);
    ~DatetimePlugin() override;

    PluginSizePolicy pluginSizePolicy() const override;

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;

    const QString itemContextMenu(const QString &itemKey) override;

    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

    void pluginSettingsChanged() override;
    virtual Dock::PluginFlags flags() const override;

    virtual QString message(const QString &) override;
    virtual void positionChanged(const Dock::Position position) override;

private slots:
    void updateCurrentTimeString();
    void refreshPluginItemsVisible();

private:
    void loadPlugin();

private:
    QScopedPointer<DatetimeWidget> m_centralWidget;
    QScopedPointer<Dock::TipsWidget> m_dateTipsLabel;
    QScopedPointer<SidebarCalendarWidget> m_calendarPopup;
    QTimer *m_refershTimer;
    QString m_currentTimeString;
    QDBusInterface *m_interface;
    bool m_pluginLoaded;

    RegionFormat* m_RegionFormatModel;

};


#endif // DATETIMEPLUGIN_H
