// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"
#include "dockiteminfo.h"

#include <QAbstractItemModel>

namespace dock {

class TrayItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* trayPluginModel READ trayPluginModel WRITE setTrayPluginModel NOTIFY trayPluginModelChanged FINAL)
    Q_PROPERTY(QAbstractItemModel* quickPluginModel READ quickPluginModel WRITE setQuickPluginModel NOTIFY quickPluginModelChanged FINAL)
    Q_PROPERTY(QAbstractItemModel* fixedPluginModel READ fixedPluginModel WRITE setFixedPluginModel NOTIFY fixedPluginModelChanged FINAL)
public:
    explicit TrayItem(QObject *parent = nullptr);

    QAbstractItemModel *trayPluginModel() const;
    void setTrayPluginModel(QAbstractItemModel *newTrayPluginModel);

    QAbstractItemModel *quickPluginModel() const;
    void setQuickPluginModel(QAbstractItemModel *newQuickPluginModel);

    QAbstractItemModel *fixedPluginModel() const;
    void setFixedPluginModel(QAbstractItemModel *newFixedPluginModel);

    Q_INVOKABLE DockItemInfos dockItemInfos();
    Q_INVOKABLE void setItemOnDock(const QString &settingKey, const QString &itemKey, bool visible);

Q_SIGNALS:
    void trayPluginModelChanged();
    void quickPluginModelChanged();
    void fixedPluginModelChanged();

private:
    bool loopDockItemInfosModel(QAbstractItemModel *model, const std::function<bool (const DockItemInfo &)> &cb);

private:
    QAbstractItemModel *m_trayPluginModel = nullptr;
    QAbstractItemModel *m_quickPluginModel = nullptr;
    QAbstractItemModel *m_fixedPluginModel = nullptr;
};

}
