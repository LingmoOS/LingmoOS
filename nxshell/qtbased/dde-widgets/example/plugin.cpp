// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include <QDebug>

QString ExampleWidgetPlugin::title() const
{
    return "Example";
}

QString ExampleWidgetPlugin::description() const
{
    return "Normal Example Widget";
}

IWidget *ExampleWidgetPlugin::createWidget()
{
    return new ExampleWidget();
}

void ExampleWidget::typeChanged(const IWidget::Type type)
{
    qDebug() << Q_FUNC_INFO << type << handler()->size();
    m_view->setFixedSize(handler()->size());
    if (type == IWidget::Small) {
        m_view->setIcon(QIcon::fromTheme("dde-file-manager"));
    } else if (type == IWidget::Middle) {
        m_view->setIcon(QIcon::fromTheme("dde-printer"));
    } else if (type == IWidget::Large) {
        m_view->setIcon(QIcon::fromTheme("deepin-album"));
    }
}

bool ExampleWidget::initialize(const QStringList &arguments) {
    Q_UNUSED(arguments);
    m_view = new QPushButton();
    m_view->setText(QString::number(handler()->value("content").toInt()));
    QObject::connect(m_view, &QPushButton::clicked, m_view, [this]() {
        auto content = handler()->value("content").toInt();
        handler()->setValue("content", ++content);
        m_view->setText(QString::number(handler()->value("content").toInt()));
    });
    return true;
}

bool ExampleWidget::enableSettings()
{
    qDebug(dwLog()) << Q_FUNC_INFO << handler()->pluginId() << handler()->id();
    return true;
}

void ExampleWidget::settings()
{
    qDebug(dwLog()) << Q_FUNC_INFO << handler()->pluginId() << handler()->id();
}
