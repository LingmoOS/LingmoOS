// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginspec.h"
#include "widgethandler.h"
#include "instanceproxy.h"
#include "widgetsinterface_p.h"
#include "utils.h"

#include <QUuid>
#include <QDebug>
#include <QSettings>

WIDGETS_FRAME_USE_NAMESPACE
WIDGETS_BEGIN_NAMESPACE

WidgetPluginSpec::WidgetPluginSpec(const PluginInfo &info)
    : m_plugin (info.plugin)
    , m_pluginId(info.id)
    , m_fileName(info.fileName)
    , m_version(info.version)
{
    Q_ASSERT(m_plugin);
    m_supportTypes = m_plugin->supportTypes();
}

WidgetPluginSpec::~WidgetPluginSpec()
{
    if (m_plugin) {
        m_plugin->deleteLater();
        m_plugin = nullptr;
    }
    if (m_dataStore) {
        m_dataStore->deleteLater();
        m_dataStore = nullptr;
    }
}

Instance *WidgetPluginSpec::createWidget(const IWidget::Type &type)
{
    return createWidgetImpl(type, QUuid::createUuid().toString());
}

Instance *WidgetPluginSpec::createWidget(const IWidget::Type &type, const InstanceId &key)
{
    return createWidgetImpl(type, key);
}

Instance *WidgetPluginSpec::createWidgetForWidgetStore(const IWidget::Type &type)
{
    auto instance = createWidget(type);
    if (instance) {
        WidgetHandlerImpl::get(instance->handler())->m_isUserAreaInstance = false;
    }
    return instance;
}

Instance *WidgetPluginSpec::createWidgetImpl(const IWidget::Type &type, const InstanceId &key)
{
    if (!m_supportTypes.contains(type))
        return nullptr;

    auto instance = m_plugin->createWidget();
    if (!instance)
        return nullptr;

    auto handler = new WidgetHandlerImpl();
    instance->d->handler = handler;
    handler->m_type = type;
    handler->m_id = key;
    handler->m_pluginId = m_pluginId;
    handler->m_pluginType = m_plugin->type();
    handler->setDataStore(m_dataStore);
    qDebug(dwLog()) << "created widget." << m_pluginId << type << key;
    return new Instance(instance);
}

QString WidgetPluginSpec::title() const
{
    return m_plugin->title();
}

QString WidgetPluginSpec::description() const
{
    return m_plugin->description();
}

QString WidgetPluginSpec::aboutDescription() const
{
    return m_plugin->aboutDescription();
}

IWidgetPlugin::Type WidgetPluginSpec::type() const
{
    return m_plugin->type();
}

QString WidgetPluginSpec::version() const
{
    return m_version;
}

QIcon WidgetPluginSpec::logo() const
{
    return m_plugin->logo();
}

QStringList WidgetPluginSpec::contributors() const
{
    return m_plugin->contributors();
}

QVector<IWidget::Type> WidgetPluginSpec::supportTypes() const
{
    return m_supportTypes;
}

void WidgetPluginSpec::removeSupportType(const IWidget::Type type)
{
    m_supportTypes.removeOne(type);
}

void WidgetPluginSpec::setDataStore(DataStore *store)
{
    m_dataStore = store;
}

PluginId WidgetPluginSpec::id() const
{
    return m_pluginId;
}

WIDGETS_END_NAMESPACE
