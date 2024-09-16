// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <widgetsinterface.h>

WIDGETS_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
class WidgetManager;
class InstanceModel : public QObject{
    Q_OBJECT
public:
    explicit InstanceModel(WidgetManager *manager, QObject *parent = nullptr);
    virtual ~InstanceModel() override;

    void loadPrePanelInstances();
    QVector<Instance *> instances() const;
    InstancePos instancePosition(const InstanceId &key);

    Instance *addInstance(const PluginId &pluginId, const IWidget::Type &type, const InstancePos expectedIndex = -1);
    void moveInstance(const InstanceId &source, InstancePos index = -1);
    Instance *replaceInstance(const InstanceId &key, const IWidget::Type &type);
    Instance *getInstance(const InstancePos pos) const;
    Instance *getInstance(const InstanceId &key) const;
    bool existInstanceByInstanceId(const InstanceId &key) const;
    bool existInstance(const PluginId &pluginId);

    QVector<IWidget::Type> pluginTypes(const PluginId &pluginId) const;
    void removePlugin(const PluginId &pluginId);

    int count() const;

public Q_SLOTS:

    void removeInstance(const InstanceId &key);

Q_SIGNALS:
    void added(const InstanceId &key, InstancePos pos);
    void removed(const InstanceId &key, InstancePos pos);
    void moved(const InstancePos &source, InstancePos target);
    void replaced(const InstanceId &key, InstancePos target);

private:
    void removeMapItem(const QString &dataStoreKey, const InstanceId &key);
    void addMapItem(const QString &dataStoreKey, const InstanceId &key, const QVariant &value);
    void updateContentItem(const InstanceId &key, const QString &contentKey, const QVariant &value);
    InstancePos addInstance(Instance *instance, const InstancePos expectedIndex = -1);

    void updatePositions();

    void loadOrCreateResidentInstance();
    void loadOrCreateAloneInstance();
    bool existInstanceInDataStore(const PluginId &pluginId);

private:
    // current instances in Panel.
    QVector<Instance *> m_instances;
    WidgetManager *m_manager;
    DataStore *m_dataStore;
};
WIDGETS_FRAME_END_NAMESPACE
