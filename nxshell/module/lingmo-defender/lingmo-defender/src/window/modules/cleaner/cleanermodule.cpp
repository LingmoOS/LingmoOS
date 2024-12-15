// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleanermodule.h"
#include "trashcleanwidget.h"
#include "cleanerdbusadaptorimpl.h"

#include <QModelIndex>
#include <QStandardItemModel>

CleanerModule::CleanerModule(FrameProxyInterface *frameProxy, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frameProxy)
    , m_widget(nullptr)
{
}

CleanerModule::~CleanerModule()
{
    if (m_widget) {
        m_widget->deleteLater();
        m_widget = nullptr;
    }
}

void CleanerModule::initialize()
{
}

void CleanerModule::preInitialize()
{
}

const QString CleanerModule::name() const
{
    return MODULE_DISK_CLEANER_NAME;
}

void CleanerModule::active(int index)
{
    Q_UNUSED(index);
    if (nullptr == m_widget) {
        m_widget = new TrashCleanWidget(nullptr, new CleanerDBusAdaptorImpl(this));
    }

    m_widget->show();
    m_frameProxy->pushWidget(this, m_widget);
}

void CleanerModule::deactive()
{
}
