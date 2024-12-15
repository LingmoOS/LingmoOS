// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICK_PLUGIN_ITEM_H
#define QUICK_PLUGIN_ITEM_H

#include "pluginitem.h"

class QuickPluginItem : public PluginItem
{
    Q_OBJECT
public:
    explicit QuickPluginItem(PluginsItemInterface *pluginInterface, const QString &itemKey, QWidget *parent = nullptr);
    ~QuickPluginItem() override = default;

    void requestActiveState();

    virtual void init() override;
    virtual QWidget *centralWidget() override;
protected:

    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual QMenu *pluginContextMenu() override;
    virtual QWidget *pluginTooltip() override;
    virtual QString itemKey() const override;

private:
    bool pluginIsVisible();

private:
    QAction *m_onDockAction;
    bool m_isActive = false;
};

#endif
