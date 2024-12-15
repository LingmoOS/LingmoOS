// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <widgetsinterface.h>
#include "memorywidget.h"

#include <QObject>
#include <QBasicTimer>
#include <QPointer>

WIDGETS_USE_NAMESPACE

class MemoryMonitorWidget : public QObject, public IWidget {
    Q_OBJECT
public:
    virtual QWidget *view() override {
        return m_view.data();
    }
    virtual ~MemoryMonitorWidget() override {
        if (m_timer)
            m_timer->stop();
    }
private:
    void updateMemory();

    QPointer<MemoryWidget> m_view;
    QScopedPointer<QBasicTimer> m_timer;
    bool m_isPressed = false;

public:
    virtual bool initialize(const QStringList &arguments) override;

    virtual void delayInitialize() override;

    virtual void typeChanged(const IWidget::Type type) override;

    virtual void showWidgets() override;
    virtual void hideWidgets() override;

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private Q_SLOTS:
    void showSystemMonitorDetail();

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

class MemoryMonitorWidgetPlugin : public IWidgetPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DdeWidgetsPlugin_iid FILE "plugin.json")
    Q_INTERFACES(WIDGETS_NAMESPACE::IWidgetPlugin)

public:
    QString title() const override;
    virtual QString description() const override;
    virtual IWidget *createWidget() override;
    virtual QVector<IWidget::Type> supportTypes() const { return {IWidget::Small};}
    virtual IWidgetPlugin::Type type() const override { return IWidgetPlugin::Normal; }

    virtual QIcon logo() const override;
    virtual QStringList contributors() const override;
};
