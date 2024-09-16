// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syscheckadaptormodel.h"

#include <QProcess>

#define SEPERATOR ", "

SysCheckAdaptorModel::SysCheckAdaptorModel(HomePageModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_issuePoint(0)
    , m_processingIndex(0)
{
    if (m_model) {
        initConnection();
    }
}

SysCheckAdaptorModel::~SysCheckAdaptorModel()
{
    qDeleteAll(m_resultItems);
    m_resultItems.clear();
}

bool SysCheckAdaptorModel::isFastFixValid()
{
    return (m_totalTrashSize >= 100 * MB_COUNT);
}

void SysCheckAdaptorModel::startExam()
{
    m_issuePoint = 0;
    m_issueCount = 0;
    m_autoStartCount = 0;
    m_isDevMode = false;
    m_isDiskNormal = true;
    m_isSysOutDate = false;
    m_totalTrashSize = 0.0;

    m_logProblemItemsInfo = "";
    m_fixingItems.clear();

    Q_EMIT checkStarted();
    resetResultItems();
    startSSHExam();
}

void SysCheckAdaptorModel::startDiskExam()
{
    if (m_processingIndex != SysCheckItemID::Trash) {
        return;
    }

    Q_EMIT stageChanged(SysCheckItemID::Disk);
    QStandardItem *diskItem = m_resultItems[SysCheckItemID::Disk];
    diskItem->setData(SysCheckItemID::Disk, CheckResultModelRole::ID);

    if (m_model->isDiskManagerInterValid()) {
        m_isDiskNormal = m_model->getDiskStatus();
        diskItem->setData(CheckProgressStatus::Success, CheckResultModelRole::CheckProgressFlag);
        diskItem->setData(m_isDiskNormal != true, CheckResultModelRole::IsIssueHappen);
        diskItem->setData(m_isDiskNormal, CheckResultModelRole::Details);
        diskItem->setData(false, CheckResultModelRole::IsIgnored);
    } else {
        diskItem->setData(CheckProgressStatus::Masked, CheckResultModelRole::CheckProgressFlag);
    }

    m_processingIndex = SysCheckItemID::Disk;
    Q_EMIT DiskExamFinished();
}

void SysCheckAdaptorModel::startSSHExam()
{
    // SSH为禁用项，不显示开始状态
    QStandardItem *sshItem = m_resultItems[SysCheckItemID::SSH];
    sshItem->setData(SysCheckItemID::SSH, CheckResultModelRole::ID);
    sshItem->setData(CheckProgressStatus::Masked, CheckResultModelRole::CheckProgressFlag);
    m_processingIndex = SysCheckItemID::SSH;
    Q_EMIT SSHExamFinished();
}

void SysCheckAdaptorModel::startTrashExam()
{
    if (m_processingIndex != SysCheckItemID::SystemUpdate) {
        return;
    }

    Q_EMIT stageChanged(SysCheckItemID::Trash);
    Q_EMIT m_model->requestStartTrashScan();
    m_processingIndex = SysCheckItemID::Trash;
}

void SysCheckAdaptorModel::startSysVersionExam()
{
    if (m_processingIndex != SysCheckItemID::AutoStartApp) {
        return;
    }

    Q_EMIT stageChanged(SysCheckItemID::SystemUpdate);
    Q_EMIT m_model->NotifyCheckSysUpdate();

    m_sysVersionTimer.stop();
    m_sysVersionTimer.setSingleShot(true);
    m_sysVersionTimer.start(SYS_VER_CHECK_TIMEOUT);
    // 设置正在检查系统更新中
    m_model->SetSysVersionChecking(true);
    m_processingIndex = SysCheckItemID::SystemUpdate;
}

void SysCheckAdaptorModel::startDevModeExam()
{
    if (m_processingIndex != SysCheckItemID::Disk) {
        return;
    }

    Q_EMIT stageChanged(SysCheckItemID::DevMode);

    m_isDevMode = m_model->IsDeveloperMode();

    QStandardItem *devItem = m_resultItems[SysCheckItemID::DevMode];
    devItem->setData(SysCheckItemID::DevMode, CheckResultModelRole::ID);
    devItem->setData(CheckProgressStatus::Success, CheckResultModelRole::CheckProgressFlag);
    devItem->setData(m_isDevMode, CheckResultModelRole::IsIssueHappen);
    devItem->setData(m_isDevMode, CheckResultModelRole::Details);
    devItem->setData(m_model->ignoreDevModeChecking(), CheckResultModelRole::IsIgnored);
    m_processingIndex = SysCheckItemID::DevMode;

    Q_EMIT DevModeExamFinished();
}

void SysCheckAdaptorModel::startAutoStartExam()
{
    if (m_processingIndex != SysCheckItemID::SSH) {
        return;
    }

    Q_EMIT stageChanged(SysCheckItemID::AutoStartApp);
    m_model->checkAutoStart();
    m_model->SetAutoStartAppChecking(true);
    m_processingIndex = SysCheckItemID::AutoStartApp;
}

// 将结果拼凑成一个standardModel统一接口
const QStandardItemModel &SysCheckAdaptorModel::getResultModel()
{
    m_resultModel.clear();
    m_resultModel.appendRow(m_resultItems);
    return m_resultModel;
}

bool SysCheckAdaptorModel::isProcessingFastFixing()
{
    return m_fixingItems.size() > 0;
}

void SysCheckAdaptorModel::onCheckStarted() { }

void SysCheckAdaptorModel::onCheckDone()
{
    // 统计分数
    updateIssuePoint();
    // 记录检查完成时间
    m_model->SetLastCheckTime(QDateTime::currentDateTime());

    Q_EMIT checkFinished();
}

void SysCheckAdaptorModel::onCheckCanceled()
{
    m_processingIndex = SysCheckItemID::MaxLimit;
    qDeleteAll(m_resultItems);
    m_resultItems.clear();
}

void SysCheckAdaptorModel::onRequestFixAll()
{
    fixTrash();
}

void SysCheckAdaptorModel::onRequestFixItem(SysCheckItemID itemId)
{
    switch (itemId) {
    case SysCheckItemID::Disk:
        fixDisk();
        break;
    case SysCheckItemID::Trash:
        fixTrash();
        break;
    case SysCheckItemID::SystemUpdate:
        fixSysUpdate();
        break;
    case SysCheckItemID::DevMode:
        fixDevMode();
        break;
    case SysCheckItemID::AutoStartApp:
        fixAutoStart();
        break;
    default:
        break;
    }
}

void SysCheckAdaptorModel::onRequestSetIgnore(SysCheckItemID id, bool isNeedIgnore)
{
    if (SysCheckItemID::DevMode == id) {
        QString logInfo = kDevModeDismissed;
        m_model->setIgnoreDevModeChecking(isNeedIgnore);
        if (isNeedIgnore) {
            m_model->addSecurityLog(logInfo);
        }
        return;
    }

    if (SysCheckItemID::AutoStartApp == id) {
        QString logInfo = isNeedIgnore ? kAutoStartDismissed : kAutoStartResumed;
        m_model->addSecurityLog(logInfo);
        m_model->setIgnoreAutoStartChecking(isNeedIgnore);
        return;
    }
}

void SysCheckAdaptorModel::onSysVersionCheckTimeout()
{
    if (m_processingIndex != SysCheckItemID::SystemUpdate) {
        return;
    }

    m_sysVersionTimer.stop();
    if (m_model->sysVersionChecking()) {
        Q_EMIT m_model->notifyTimeroutSysUpdate("failed");
    }

    QStandardItem *sysVersionItem = m_resultItems[SysCheckItemID::SystemUpdate];
    sysVersionItem->setData(SysCheckItemID::SystemUpdate, CheckResultModelRole::ID);
    // 超时，不显示该项
    sysVersionItem->setData(CheckProgressStatus::Masked, CheckResultModelRole::CheckProgressFlag);
    Q_EMIT SysVersionExamFinished();
}

void SysCheckAdaptorModel::fixDisk()
{
    QProcess::startDetached("deepin-diskmanager", {});
    m_model->addSecurityLog(KDiskChecked);
}

void SysCheckAdaptorModel::fixTrash()
{
    if (m_totalTrashSize >= 100 * MB_COUNT) {
        m_fixingItems.append(SysCheckItemID::Trash);
        Q_EMIT fixItemStarted(SysCheckItemID::Trash);
        Q_EMIT m_model->requestCleanSelectTrash();
        m_model->addSecurityLog(kTrashFixed);
    }
}

void SysCheckAdaptorModel::fixSysUpdate()
{
    if (m_isSysOutDate) {
        m_model->openSysUpdatePage();
    }
}

void SysCheckAdaptorModel::fixDevMode()
{
    // do nothing
}

void SysCheckAdaptorModel::fixAutoStart()
{
    m_model->openStartupControlPage();
}

void SysCheckAdaptorModel::initConnection()
{
    connect(this,
            &SysCheckAdaptorModel::SSHExamFinished,
            this,
            &SysCheckAdaptorModel::startAutoStartExam);
    connect(this,
            &SysCheckAdaptorModel::AutoStartExamFinished,
            this,
            &SysCheckAdaptorModel::startSysVersionExam);
    connect(this,
            &SysCheckAdaptorModel::SysVersionExamFinished,
            this,
            &SysCheckAdaptorModel::startTrashExam);
    connect(this,
            &SysCheckAdaptorModel::TrashExamFinished,
            this,
            &SysCheckAdaptorModel::startDiskExam);
    connect(this,
            &SysCheckAdaptorModel::DiskExamFinished,
            this,
            &SysCheckAdaptorModel::startDevModeExam);
    connect(this,
            &SysCheckAdaptorModel::DevModeExamFinished,
            this,
            &SysCheckAdaptorModel::onCheckDone);

    connect(&m_sysVersionTimer,
            &QTimer::timeout,
            this,
            &SysCheckAdaptorModel::onSysVersionCheckTimeout);
    connect(m_model, &HomePageModel::trashScanFinished, this, [this](double size) {
        if (m_processingIndex != SysCheckItemID::Trash) {
            return;
        }
        m_totalTrashSize = size;
        QStandardItem *trashItem = m_resultItems[SysCheckItemID::Trash];
        trashItem->setData(SysCheckItemID::Trash, CheckResultModelRole::ID);
        trashItem->setData(CheckProgressStatus::Success, CheckResultModelRole::CheckProgressFlag);
        trashItem->setData(m_totalTrashSize >= (100 * MB_COUNT),
                           CheckResultModelRole::IsIssueHappen);
        trashItem->setData(m_totalTrashSize, CheckResultModelRole::Details);

        Q_EMIT TrashExamFinished();
    });
    // SSH DISK没有相关信号可转发
    connect(m_model, &HomePageModel::CheckSysUpdateFinished, this, [this](bool isOutOfDate) {
        if (m_processingIndex != SysCheckItemID::SystemUpdate) {
            return;
        }
        m_sysVersionTimer.stop();
        m_isSysOutDate = isOutOfDate;
        m_model->SetSysVersionChecking(false);

        QStandardItem *sysVersionItem = m_resultItems[SysCheckItemID::SystemUpdate];
        sysVersionItem->setData(SysCheckItemID::SystemUpdate, CheckResultModelRole::ID);
        sysVersionItem->setData(CheckProgressStatus::Success,
                                CheckResultModelRole::CheckProgressFlag);
        sysVersionItem->setData(m_isSysOutDate, CheckResultModelRole::IsIssueHappen);
        sysVersionItem->setData(m_isSysOutDate, CheckResultModelRole::Details);
        Q_EMIT SysVersionExamFinished();
    });
    // 开发者模式没有信号可转发
    connect(m_model, &HomePageModel::checkAutoStartFinished, this, [this](int count) {
        if (m_processingIndex != SysCheckItemID::AutoStartApp) {
            return;
        }
        m_autoStartCount = count;
        m_model->SetAutoStartAppChecking(false);
        QStandardItem *autoStartItem = m_resultItems[SysCheckItemID::AutoStartApp];
        autoStartItem->setData(SysCheckItemID::AutoStartApp, CheckResultModelRole::ID);
        autoStartItem->setData(CheckProgressStatus::Success,
                               CheckResultModelRole::CheckProgressFlag);
        autoStartItem->setData(m_autoStartCount > 0, CheckResultModelRole::IsIssueHappen);
        autoStartItem->setData(m_model->ignoreAutoStartChecking(), CheckResultModelRole::IsIgnored);
        autoStartItem->setData(m_autoStartCount, CheckResultModelRole::Details);
        Q_EMIT AutoStartExamFinished();
    });

    connect(m_model, &HomePageModel::cleanSelectTrashFinished, this, [this] {
        if (!m_resultItems.size()) {
            return;
        }
        m_totalTrashSize = 0.0;
        m_resultItems[SysCheckItemID::Trash]->setData(false, CheckResultModelRole::IsIssueHappen);
        m_resultItems[SysCheckItemID::Trash]->setData(m_totalTrashSize,
                                                      CheckResultModelRole::Details);
        m_fixingItems.removeAll(SysCheckItemID::Trash);
        Q_EMIT fixItemFinished(SysCheckItemID::Trash);
    });

    connect(m_model,
            &HomePageModel::autoStartAppCountChanged,
            this,
            &SysCheckAdaptorModel::onAutoStartAppCountChanged);
}

// 更新检查结果
void SysCheckAdaptorModel::updateIssuePoint()
{
    m_issuePoint = 0;
    m_issueCount = 0;

    // disk
    m_issuePoint += !m_isDiskNormal ? kIssuePointInfoList[SysCheckItemID::Disk] : 0;
    m_issueCount += !m_isDiskNormal ? 1 : 0;
    if (!m_isDiskNormal) {
        if (!m_logProblemItemsInfo.isEmpty()) {
            m_logProblemItemsInfo.append(SEPERATOR);
        }
        m_logProblemItemsInfo.append(kTypename4);
    }

    // trash
    m_issuePoint +=
        m_totalTrashSize >= (100 * MB_COUNT) ? kIssuePointInfoList[SysCheckItemID::Trash] : 0;
    m_issueCount += m_totalTrashSize >= (100 * MB_COUNT) ? 1 : 0;
    // trash总量大于1G时，有修正值
    m_issuePoint += m_totalTrashSize > (1 * GB_COUNT) ? kTrashSizeTooLargeAdjustPoint : 0;
    if (m_totalTrashSize >= (100 * MB_COUNT)) {
        if (!m_logProblemItemsInfo.isEmpty()) {
            m_logProblemItemsInfo.append(SEPERATOR);
        }
        m_logProblemItemsInfo.append(kTypename5);
    }

    // sys update
    m_issuePoint += m_isSysOutDate ? kIssuePointInfoList[SysCheckItemID::SystemUpdate] : 0;
    m_issueCount += m_isSysOutDate ? 1 : 0;
    if (m_isSysOutDate) {
        if (!m_logProblemItemsInfo.isEmpty()) {
            m_logProblemItemsInfo.append(SEPERATOR);
        }
        m_logProblemItemsInfo.append(kTypename6);
    }

    // devmod
    if (!m_model->ignoreDevModeChecking()) {
        m_issuePoint += m_isDevMode ? kIssuePointInfoList[SysCheckItemID::DevMode] : 0;
        m_issueCount += m_isDevMode ? 1 : 0;
        if (m_isDevMode) {
            if (!m_logProblemItemsInfo.isEmpty()) {
                m_logProblemItemsInfo.append(SEPERATOR);
            }
            m_logProblemItemsInfo.append(kTypename7);
        }
    }
    // autostart
    if (!m_model->ignoreAutoStartChecking()) {
        m_issuePoint += m_autoStartCount ? kIssuePointInfoList[SysCheckItemID::AutoStartApp] : 0;
        m_issueCount += m_autoStartCount ? 1 : 0;
        if (m_autoStartCount) {
            if (!m_logProblemItemsInfo.isEmpty()) {
                m_logProblemItemsInfo.append(SEPERATOR);
            }
            m_logProblemItemsInfo.append(kTypename8);
        }
    }

    m_model->setSafetyScore(CHECK_MAX_POINT > m_issuePoint ? CHECK_MAX_POINT - m_issuePoint : 0);
    m_model->addSecurityLog(kErrorInfomation.arg(m_issueCount).arg(m_logProblemItemsInfo));
}

void SysCheckAdaptorModel::resetResultItems()
{
    qDeleteAll(m_resultItems);
    m_resultItems.clear();
    for (int i = SysCheckItemID::SSH; i <= SysCheckItemID::AutoStartApp; i++) {
        m_resultItems.push_back(new QStandardItem);
    }
}
