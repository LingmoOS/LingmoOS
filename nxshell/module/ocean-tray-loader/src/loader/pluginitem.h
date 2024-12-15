// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINSITEM_H
#define PLUGINSITEM_H

#include "pluginsiteminterface_v2.h"
#include "dockdbusproxy.h"

#include <QWidget>

const int Attribute_ForceUnDock = 0x800000;

class QMenu;
class PluginItem : public QWidget
{
    Q_OBJECT

public:
    explicit PluginItem(PluginsItemInterface *pluginsItemInterface, const QString &itemKey, QWidget *parent = nullptr);
    ~PluginItem() override;

    void updateItemWidgetSize(const QSize &size);

    int pluginFlags() const;
    void setPluginFlags(int flags);
    virtual void init();

    QString pluginId() const { return m_pluginsItemInterface->pluginName(); }
    virtual QString itemKey() const { return m_itemKey; }

    void updatePluginContentMargin(int spacing);

    static bool isForceUndock(int pluginFlag);

signals:
    void recvMouseEvent(int type);
    void sigRequestshutdown();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void moveEvent(QMoveEvent *e) override;

    virtual QWidget *centralWidget();
    virtual QMenu *pluginContextMenu();
    virtual QWidget *pluginTooltip();

    PluginsItemInterface * pluginsItemInterface();
    void initPluginMenu();
    QWidget *itemTooltip(const QString &itemKey);
    bool executeCommand();

private:
    QWidget *itemPopupApplet();
    bool panelPopupExisted() const;
    bool embedPanelPopupExisted() const;
    void closeToolTip();

private:
    void updatePopupSize(const QRect &rect);

protected:
    QString m_itemKey;
    QMenu *m_menu;
    QScopedPointer<DockDBusProxy> m_dbusProxy;

private:
    PluginsItemInterface *m_pluginsItemInterface;
    QTimer* m_tooltipTimer;
    QPointer<QWidget> m_tipsWidget;

    QAction *m_unDockAction = nullptr;
    int m_pluginFlags = 0;
};

#endif // PLUGINSITEM_H

