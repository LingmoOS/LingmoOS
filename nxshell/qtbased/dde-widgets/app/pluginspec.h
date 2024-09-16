// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <widgetsinterface.h>

WIDGETS_FRAME_BEGIN_NAMESPACE
class WidgetManager;
WIDGETS_FRAME_END_NAMESPACE

struct PluginInfo {
    WIDGETS_NAMESPACE::IWidgetPlugin *plugin = nullptr;
    WIDGETS_FRAME_NAMESPACE::PluginId id;
    QString fileName;
    QString version;
};
WIDGETS_BEGIN_NAMESPACE

class WidgetPluginSpec {
public:
    explicit WidgetPluginSpec(const PluginInfo &info);
    ~WidgetPluginSpec();

    WIDGETS_FRAME_NAMESPACE::PluginId id() const;
    QString title() const;
    QString description() const;
    QString aboutDescription() const;
    IWidgetPlugin::Type type() const;
    QString version() const;
    QIcon logo() const;
    QStringList contributors() const;

    QVector<IWidget::Type> supportTypes() const;
    void removeSupportType(const IWidget::Type type);

    WIDGETS_FRAME_NAMESPACE::Instance *createWidget(const IWidget::Type &type);
    WIDGETS_FRAME_NAMESPACE::Instance *createWidget(const IWidget::Type &type, const WIDGETS_FRAME_NAMESPACE::InstanceId &key);
private:

    WIDGETS_FRAME_NAMESPACE::Instance *createWidgetForWidgetStore(const IWidget::Type &type);

    WIDGETS_FRAME_NAMESPACE::Instance *createWidgetImpl(const IWidget::Type &type, const WIDGETS_FRAME_NAMESPACE::InstanceId &key);
    void setDataStore(WIDGETS_FRAME_NAMESPACE::DataStore *store);

    IWidgetPlugin *m_plugin = nullptr;
    WIDGETS_FRAME_NAMESPACE::PluginId m_pluginId;
    WIDGETS_FRAME_NAMESPACE::DataStore *m_dataStore = nullptr;
    QString m_fileName;
    QString m_version;
    QVector<IWidget::Type> m_supportTypes;

    friend class WIDGETS_FRAME_NAMESPACE::WidgetManager;
};

WIDGETS_END_NAMESPACE
