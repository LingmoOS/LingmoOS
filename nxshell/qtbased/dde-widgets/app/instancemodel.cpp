// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "instancemodel.h"
#include "pluginspec.h"
#include "widgethandler.h"
#include "instanceproxy.h"
#include "widgetmanager.h"

#include <QSettings>
#include <QDir>
#include <QDebug>

WIDGETS_FRAME_BEGIN_NAMESPACE
namespace Store {
static const char *Instances = "instances";
static const char *Positions = "positions";
static const char *PluginId = "pluginId";
static const char *Version = "version";
static const char *Type = "type";
}

InstanceModel::InstanceModel(WidgetManager *manager, QObject *parent)
    : QObject(parent)
    , m_manager(manager)
    , m_dataStore(m_manager->dataStore())
{
}

InstanceModel::~InstanceModel()
{
}

void InstanceModel::loadPrePanelInstances()
{
    QVariantMap savedInstances = m_dataStore->value(Store::Instances).toMap();
    const auto &savedPositions = m_dataStore->value(Store::Positions).toMap();
    // recode existed instance and it's order.
    QMap<InstancePos, Instance *> existedInstances;
    for (auto iter = savedInstances.begin(); iter != savedInstances.end(); iter++) {
        const auto &item = iter.value();
        const InstanceId &key = iter.key();
        const QVariantMap &info = item.toMap();
        const auto &pluginId = info[Store::PluginId].toString();

        Instance *instance = nullptr;
        do {
            // plugin changed.
            auto plugin = m_manager->getPlugin(pluginId);
            if (!plugin) {
                m_manager->clearDataStore(pluginId);
                qDebug(dwLog()) << "plugin has not exist." << pluginId;
                break;
            }

            const auto &version = info[Store::Version].toString();
            // plugin's Version changed.
            if (!m_manager->matchVersion(version)) {
                qWarning(dwLog()) << QString("plugin's version [%1] has not matched by [%2].").arg(version).arg(m_manager->currentVersion()) << pluginId;
                break;
            }

            auto type = static_cast<IWidget::Type>(info[Store::Type].toInt());
            // plugin's Type changed.
            instance = plugin->createWidget(type, key);
            if (!instance)
                break;
        } while (false);

        if (!instance) {
            // clear the cache of not existed instance.
            removeMapItem(Store::Instances, key);
            removeMapItem(Store::Positions, key);
            m_manager->clearDataStore(pluginId, key);
            continue;
        }

        InstancePos position = savedPositions[key].toInt();
        existedInstances[position] = instance;
    }

    const auto &failedInstances = m_manager->initialize(existedInstances.values().toVector());

    // save instance.
    m_instances.resize(existedInstances.size() - failedInstances.size());
    InstancePos position = 0;
    for (auto instance : existedInstances.values()) {
        if (failedInstances.contains(instance))
            continue;

        m_instances[position] = instance;
        ++position;
    }

    m_manager->typeChanged(m_instances);

    qDeleteAll(failedInstances);

    // update position's cache.
    const bool positionChanged = m_instances.size() != existedInstances.size();
    if (positionChanged)
        updatePositions();

    loadOrCreateResidentInstance();

    loadOrCreateAloneInstance();
}

QVector<Instance *> InstanceModel::instances() const
{
    return m_instances;
}

void InstanceModel::moveInstance(const InstanceId &source, InstancePos index)
{
    InstancePos sourceIndex = instancePosition(source);
    if (sourceIndex < 0)
        return;

    auto instance = m_instances.takeAt(sourceIndex);

    InstancePos targetIndex = index < 0 || index >= m_instances.count() ? m_instances.count() : index;
    m_instances.insert(targetIndex, instance);
    updatePositions();

    qDebug(dwLog()) << QString("model move instance from [%1] to [%2]").arg(sourceIndex).arg(targetIndex);
    Q_EMIT moved(sourceIndex, targetIndex);
}

Instance *InstanceModel::replaceInstance(const InstanceId &key, const IWidget::Type &type)
{
    auto instance = getInstance(key);
    Q_ASSERT(instance);
    if (type == instance->handler()->type())
        return instance;

    const InstancePos pos = instancePosition(key);
    const IWidget::Type oldType = instance->handler()->type();
    m_manager->typeChanged(key, type);
    updateContentItem(key, Store::Type, type);

    qDebug(dwLog()) << QString("model replace instance from [%1] to [%2]").arg(WidgetHandlerImpl::typeString(oldType)).arg(WidgetHandlerImpl::typeString(type));
    Q_EMIT replaced(key, pos);
    return instance;
}

InstancePos InstanceModel::addInstance(Instance *instance, const InstancePos expectedIndex)
{
    QVariantMap info;
    info[Store::Type] = instance->handler()->type();
    info[Store::PluginId] = instance->handler()->pluginId();
    info[Store::Version] = m_manager->currentVersion();
    addMapItem(Store::Instances, instance->handler()->id(), info);

    const InstancePos index = expectedIndex < 0 ? m_instances.count() : expectedIndex;
    addMapItem(Store::Positions, instance->handler()->id(), index);
    m_instances.insert(index, instance);
    updatePositions();
    return index;
}

void InstanceModel::removeInstance(const InstanceId &key)
{
    if (!existInstanceByInstanceId(key))
        return;

    InstancePos position = instancePosition(key);

    if (position >= 0) {

        m_instances.remove(position);
        updatePositions();
        Q_EMIT removed(key, position);
    }

    removeMapItem(Store::Instances, key);
    removeMapItem(Store::Positions, key);

    auto instance = m_manager->getInstance(key);
    WidgetHandlerImpl::get(instance->handler())->clear();
    m_manager->removeWidget(key);
}

void InstanceModel::removeMapItem(const QString &dataStoreKey, const InstanceId &key)
{
    QVariantMap saved = m_dataStore->value(dataStoreKey).toMap();
    saved.remove(key);
    m_dataStore->setValue(dataStoreKey, saved);
}

void InstanceModel::addMapItem(const QString &dataStoreKey, const InstanceId &key, const QVariant &value)
{
    QVariantMap saved = m_dataStore->value(dataStoreKey).toMap();
    saved[key] = value;
    m_dataStore->setValue(dataStoreKey, saved);
}

void InstanceModel::updateContentItem(const InstanceId &key, const QString &contentKey, const QVariant &value)
{
    QVariantMap saved = m_dataStore->value(Store::Instances).toMap();
    auto item = saved[key].toMap();
    item[contentKey] = value;
    saved[key] = item;
    m_dataStore->setValue(Store::Instances, saved);
}

void InstanceModel::updatePositions()
{
    QVariantMap saved = m_dataStore->value(Store::Positions).toMap();
    for (int i = 0; i < m_instances.count(); i++) {
        const auto instance = m_instances[i];
        saved[instance->handler()->id()] = i;
    }
    m_dataStore->setValue(Store::Positions, saved);
}

void InstanceModel::loadOrCreateResidentInstance()
{
    for (auto plugin : m_manager->plugins(IWidgetPlugin::Resident)) {

        if (existInstance(plugin->id()))
            continue;

        if (existInstanceInDataStore(plugin->id()))
            continue;

        const auto types = plugin->supportTypes();
        if (types.empty()) {
            qWarning(dwLog()) << "Resident plugins not existed any types widget." << plugin->id();
            continue;
        }
        const IWidget::Type type = types.first();
        if (auto instance = m_manager->createWidget(plugin->id(), type)) {
            addInstance(instance, 0);
        }
    }
}

void InstanceModel::loadOrCreateAloneInstance()
{
    for (auto plugin : m_manager->plugins(IWidgetPlugin::Alone)) {

        if (existInstance(plugin->id()))
            continue;

        if (existInstanceInDataStore(plugin->id()))
            continue;

        const auto types = plugin->supportTypes();
        if (types.empty()) {
            qWarning(dwLog()) << "Alone plugins not existed any types widget." << plugin->id();
            continue;
        }
        const IWidget::Type type = types.first();
        if (auto instance = m_manager->createWidget(plugin->id(), type)) {
            addInstance(instance, 0);
        }
    }
}

bool InstanceModel::existInstanceInDataStore(const PluginId &pluginId)
{
    for (const auto &item : m_dataStore->value(Store::Instances).toMap()) {
        const QVariantMap &info = item.toMap();
        if (info[Store::PluginId].toString() == pluginId)
            return true;
    }
    return false;
}

InstancePos InstanceModel::instancePosition(const InstanceId &key)
{
    bool ok = false;
    InstancePos position = m_dataStore->value(Store::Positions).toMap()[key].toInt(&ok);
    if (ok)
        return position;

    return -1;
}

Instance *InstanceModel::addInstance(const PluginId &pluginId, const IWidget::Type &type, const InstancePos expectedIndex)
{
    if (auto instance = m_manager->createWidget(pluginId, type)) {
        const InstancePos index = addInstance(instance, expectedIndex);

        Q_EMIT added(instance->handler()->id(), index);
        // addWidegt to Model, This hint the `instance` should be shown.
        m_manager->showWidgets({instance});
        return instance;
    }
    return nullptr;
}

Instance *InstanceModel::getInstance(const InstancePos pos) const
{
    return m_instances[pos];
}

Instance *InstanceModel::getInstance(const InstanceId &key) const
{
    for (auto item : qAsConst(m_instances)) {
        if (item->handler()->id() == key)
            return item;
    }
    return nullptr;
}

bool InstanceModel::existInstanceByInstanceId(const InstanceId &key) const
{
    for (auto item : qAsConst(m_instances)) {
        if (item->handler()->id() == key)
            return true;
    }
    qWarning(dwLog()) << "not exist instance:" << key;
    return false;
}

bool InstanceModel::existInstance(const PluginId &pluginId)
{
    for (auto item : qAsConst(m_instances)) {
        if (item->handler()->pluginId() == pluginId)
            return true;
    }
    return false;
}

QVector<IWidget::Type> InstanceModel::pluginTypes(const PluginId &pluginId) const
{
    return m_manager->getPlugin(pluginId)->supportTypes();
}

void InstanceModel::removePlugin(const PluginId &pluginId)
{
    const auto tmpInstances = m_instances;
    for (auto item : tmpInstances) {
        if (item->handler()->pluginId() == pluginId) {
            removeInstance(item->handler()->id());
        }
    }
}

int InstanceModel::count() const
{
    return m_instances.count();
}
WIDGETS_FRAME_END_NAMESPACE
