// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPushButton>
#include <widgetsinterface.h>

WIDGETS_USE_NAMESPACE

class ExampleWidget : public IWidget {
public:
    virtual QWidget *view() override {
        return m_view;
    }
    virtual ~ExampleWidget() { }
private:
    QPushButton *m_view = nullptr;

public:
    virtual bool initialize(const QStringList &arguments) override;

    virtual void typeChanged(const IWidget::Type type) override;

    virtual bool enableSettings() override;
    virtual void settings() override;
};

class ExampleWidgetPlugin : public IWidgetPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DdeWidgetsPlugin_iid FILE "plugin.json")
    Q_INTERFACES(WIDGETS_NAMESPACE::IWidgetPlugin)

public:
    QString title() const override;
    virtual QString description() const override;
    IWidget *createWidget() override;
    virtual QVector<IWidget::Type> supportTypes() const { return {IWidget::Small, IWidget::Middle, IWidget::Large};}
};
