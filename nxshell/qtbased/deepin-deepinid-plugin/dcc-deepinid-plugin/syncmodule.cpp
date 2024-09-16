// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncmodule.h"
#include "utils.h"
#include "syncwidget.h"

#include <DSysInfo>

#include <QIcon>

SyncModule::SyncModule(QObject *parent)
    : ModuleObject(parent)
    , m_model(nullptr)
    , m_worker(nullptr)
{
    qInfo() << "create syncmodule";
    setName("cloudsync");
    setDisplayName(DSysInfo::isCommunityEdition() ? tr("deepin ID") : tr("UOS ID"));
    setDescription(tr("Cloud sync, app store and etc"));
    setIcon(QIcon::fromTheme("deepin-id"));
    m_model = new SyncModel(this);
    m_worker = new SyncWorker(m_model, this);
}

SyncModule::~SyncModule()
{

}

//
void SyncModule::active()
{
    qInfo() << "active sync module";
    InitModuleData();
}

void SyncModule::deactive()
{
    qInfo() << "deactive sync module";
}

QWidget *SyncModule::page()
{
    SyncWidget *widget = new SyncWidget;
    widget->setVisible(false);

    connect(widget, &SyncWidget::requestLoginUser, m_worker, &SyncWorker::loginUser, Qt::UniqueConnection);

    connect(widget, &SyncWidget::requestAsyncLogoutUser, m_worker, &SyncWorker::logoutUser, Qt::QueuedConnection);
    connect(widget, &SyncWidget::requestPullMessage, m_worker, &SyncWorker::onPullMessage);
    connect(widget, &SyncWidget::requestSetFullname, m_worker, &SyncWorker::onSetFullname);

    connect(widget, &SyncWidget::requestBindAccount, m_worker, &SyncWorker::asyncBindAccount, Qt::UniqueConnection);
    connect(widget, &SyncWidget::requestUnBindAccount, m_worker, &SyncWorker::asyncUnbindAccount, Qt::UniqueConnection);
    connect(widget, &SyncWidget::requestSetAutoSync, m_worker, &SyncWorker::setAutoSync, Qt::UniqueConnection);
    connect(widget, &SyncWidget::requestLocalBindCheck, m_worker, &SyncWorker::asyncLocalBindCheck, Qt::UniqueConnection);
    connect(widget, &SyncWidget::requestSetModuleState, m_worker, &SyncWorker::setSyncState, Qt::QueuedConnection);
    connect(widget, &SyncWidget::requestSetUtcloudModuleState, m_worker, &SyncWorker::setUtcloudState, Qt::QueuedConnection);

    connect(widget, &SyncWidget::requestUOSID, m_worker, &SyncWorker::getUOSID, Qt::UniqueConnection);
    connect(widget, &SyncWidget::requestUUID, m_worker, &SyncWorker::getUUID, Qt::UniqueConnection);
    connect(widget, &SyncWidget::requestHostName, m_worker, &SyncWorker::getHostName, Qt::UniqueConnection);
    connect(widget, &SyncWidget::refreshDeviceList, m_worker, &SyncWorker::getDeviceList, Qt::UniqueConnection);
    connect(widget, &SyncWidget::removeDevice, m_worker, &SyncWorker::removeDevice, Qt::UniqueConnection);
    connect(widget, &SyncWidget::clearCloudData, m_worker, &SyncWorker::clearData, Qt::UniqueConnection);
    connect(widget, &SyncWidget::refreshSyncList, m_worker, &SyncWorker::refreshSyncInfo, Qt::UniqueConnection);

    widget->setWorker(m_worker);
    widget->setModel(m_model);
    widget->setVisible(true);

    m_worker->activate(); //refresh data
    return widget;
}

void SyncModule::InitModuleData()
{
    qInfo() << "InitModuleData begin";
    if (!DSysInfo::isDeepin()) {
        qInfo() << "module: " << displayName() << " is disable now!";
        return;
    }

    bool visible = !IsServerSystem && m_model->syncIsValid();
    setVisible(visible);

    connect(m_model, &SyncModel::syncIsValidChanged, this, [=](bool valid) {
        bool visible = valid && !IsServerSystem;
        setVisible(visible);
    });

    m_worker->initData();
    qInfo() << "InitModuleData end";
}
