// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <widgetsinterface.h>

WIDGETS_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
class WidgetHandlerImpl : public WidgetHandler {
public:
    explicit WidgetHandlerImpl();
    virtual ~WidgetHandlerImpl() override;

    virtual void setValue(const QString &key, const QVariant &value) override;
    virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const override;
    virtual void resetValue(const QString &key) override;
    virtual void removeValue(const QString &key) override;
    virtual bool containsValue(const QString &key) override;
    virtual QString id() const override;
    virtual QString pluginId() const override { return m_pluginId;}
    virtual IWidget::Type type() const override { return m_type;}
    virtual QSize size() const override;
    int roundedCornerRadius() const override;
    static QSize size(const IWidget::Type type, const bool instance = true);
    QString typeString() const;
    static QString typeString(const Widgets::IWidget::Type type);
    void clear();
    bool unavailableDS() const;

    static WidgetHandlerImpl *get(WidgetHandler *handler);

    IWidgetPlugin::Type pluginType() const;
    bool isFixted() const;
    bool isCustom() const;
    void setDataStore(DataStore *store);

    bool m_isUserAreaInstance = true;
    InstanceId m_id;
    PluginId m_pluginId;
    IWidget::Type m_type;
    IWidgetPlugin::Type m_pluginType;
    DataStore *m_dataStore = nullptr;
};
WIDGETS_FRAME_END_NAMESPACE
