// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <widgetsinterface.h>
#include <QPointer>
#include "mainview.h"

WIDGETS_USE_NAMESPACE
namespace dwclock {
class WorldClockWidget : public IWidget {
public:
    virtual QWidget *view() override;
    virtual ~WorldClockWidget() { }

    virtual bool initialize(const QStringList &arguments) override;
    virtual void delayInitialize() override;
    virtual void typeChanged(const IWidget::Type type) override;

    virtual bool enableSettings() override;
    virtual void settings() override;
    virtual void hideWidgets() override;

private:

    QPointer<ViewManager> m_viewManager = nullptr;
};

class WorldClockWidgetPlugin : public IWidgetPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DdeWidgetsPlugin_iid FILE "plugin.json")
    Q_INTERFACES(WIDGETS_NAMESPACE::IWidgetPlugin)

public:
    QString title() const override;
    virtual QString description() const override;
    virtual QString aboutDescription() const override;
    IWidget *createWidget() override;
    virtual QVector<IWidget::Type> supportTypes() const { return {IWidget::Middle, IWidget::Small};}

    virtual QIcon logo() const override;
    virtual QStringList contributors() const override;
};
}
