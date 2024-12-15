// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <widgetsinterface.h>

#include <QObject>
#include <QtPlugin>
#include <QStringList>
#include <QImage>
#include <QPushButton>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>

#include "notification/persistence.h"
#include "notifycenterwidget.h"

WIDGETS_USE_NAMESPACE

class NotificationWidget : public IWidget {
public:
    virtual QWidget *view() override {
        return m_view.get();
    }
    virtual ~NotificationWidget() override {
        if (m_persistence) {
            m_persistence->deleteLater();
            m_persistence = nullptr;
        }
    }
private:

    QScopedPointer<NotifyCenterWidget> m_view;
    AbstractPersistence* m_persistence = nullptr;

public:
    virtual bool initialize(const QStringList &arguments) override;

    virtual void delayInitialize() override;
};

class NotificationWidgetPlugin : public IWidgetPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DdeWidgetsPlugin_iid FILE "plugin.json")
    Q_INTERFACES(WIDGETS_NAMESPACE::IWidgetPlugin)

public:
    QString title() const override;
    virtual QString description() const override;
    virtual IWidget *createWidget() override;
    virtual QVector<IWidget::Type> supportTypes() const { return {IWidget::Custom};}
    virtual IWidgetPlugin::Type type() const override { return IWidgetPlugin::Alone; }

    virtual QIcon logo() const override;
    virtual QStringList contributors() const override;
};
