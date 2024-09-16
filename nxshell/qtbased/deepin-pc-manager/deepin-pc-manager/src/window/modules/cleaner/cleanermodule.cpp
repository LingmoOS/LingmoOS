// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleanermodule.h"

#include "cleanerdbusadaptorimpl.h"
#include "widgets/trashcleangreetingwidget.h"
#include "widgets/trashcleanresultwidget.h"
#include "window/modules/common/gsettingkey.h"

#include <QModelIndex>
#include <QStandardItemModel>

CleanerModule::CleanerModule(FrameProxyInterface *frameProxy, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frameProxy)
    , m_dbusAdaptorImpl(nullptr)
    , m_widget(nullptr)
    , m_greetingWidget(nullptr)
    , m_resutlWidget(nullptr)
    , m_framProxy(frameProxy)
{
}

CleanerModule::~CleanerModule()
{
    // 在m_frameProxy->pushWidget方法调用中，已经将显示页的父指针指向MainWindow对象，各显示页是否释放由MainWindow对象决定
    // 所以此处不能手动删除各显示页
    //    if (m_resutlWidget) {
    //        m_resutlWidget->haltScan();
    //        m_resutlWidget->deleteLater();
    //        m_resutlWidget = nullptr;
    //    }

    //    if (m_greetingWidget) {
    //        m_greetingWidget->deleteLater();
    //        m_greetingWidget = nullptr;
    //    }
}

void CleanerModule::initialize() { }

void CleanerModule::preInitialize() { }

const QString CleanerModule::name() const
{
    return MODULE_DISK_CLEANER_NAME;
}

void CleanerModule::active(int index)
{
    Q_UNUSED(index);
    if (!m_dbusAdaptorImpl) {
        m_dbusAdaptorImpl = new CleanerDBusAdaptorImpl(this);
    }

    QString lastScanSize = m_dbusAdaptorImpl->GetValueFromeGSettings(CLEANER_LAST_TIME_CLEANED);

    if (!m_greetingWidget) {
        m_greetingWidget = new TrashCleanGreetingWidget(nullptr);
        connect(m_greetingWidget,
                &TrashCleanGreetingWidget::notifyStartScan,
                this,
                &CleanerModule::showResultWidget);
        connect(m_greetingWidget, &QWidget::destroyed, this, [&] {
            m_greetingWidget = nullptr;
        });
    }
    m_greetingWidget->setGreeing(lastScanSize);

    m_frameProxy->pushWidget(this, m_greetingWidget);
    if (m_resutlWidget) {
        TrashCleanResultWidget::ScanStages stage = m_resutlWidget->GetCurrentStage();

        if (stage > TrashCleanResultWidget::ScanStages::PREPARING) {
            m_greetingWidget->hide();
            m_frameProxy->pushWidget(this, m_resutlWidget);

            if (stage == TrashCleanResultWidget::ScanStages::SCAN_STARTED) {
                m_framProxy->setBackForwardButtonStatus(false);
            }
        }
    }
}

void CleanerModule::deactive() { }

void CleanerModule::showResultWidget()
{
    // 为避免资源问题不要使用旧窗口
    // 重新new一个出来
    if (!m_resutlWidget) {
        m_resutlWidget = new TrashCleanResultWidget(m_dbusAdaptorImpl);
        connect(m_resutlWidget, &TrashCleanResultWidget::destroyed, this, [this] {
            m_resutlWidget = nullptr;
        });

        connect(m_resutlWidget, &TrashCleanResultWidget::notifyBackToGreetingWidget, this, [this] {
            m_framProxy->popAndDelWidget(this);
            m_resutlWidget = nullptr;
            m_greetingWidget->show();
        });

        connect(m_resutlWidget,
                &TrashCleanResultWidget::notifyUpdateLastCleaned,
                this,
                [this](const QString &lastScanSize) {
                    if (m_greetingWidget) {
                        m_greetingWidget->setGreeing(lastScanSize);
                    }
                });

        connect(m_resutlWidget,
                &TrashCleanResultWidget::notifySetBackForwardBtnStatus,
                this,
                [this](bool status) {
                    m_framProxy->setBackForwardButtonStatus(status);
                });
    }

    m_greetingWidget->hide();
    m_frameProxy->pushWidget(this, m_resutlWidget);
    m_resutlWidget->setScanConfigList(m_greetingWidget->getScanConfig());
    m_resutlWidget->processScan();
}
