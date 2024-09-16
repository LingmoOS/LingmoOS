// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageDriverManager.h"
#include "drivertableview.h"
#include "DetectedStatusWidget.h"
#include "MacroDefinition.h"
#include "driveritem.h"
#include "DriverScanWidget.h"
#include "PageDriverTableView.h"
#include "DBusDriverInterface.h"
#include "DriverScanner.h"
#include "DeviceManager.h"
#include "DevicePrint.h"
#include "DeviceNetwork.h"
#include "commontools.h"
#include "PageListView.h"
#include "PageDriverInstallInfo.h"
#include "PageDriverBackupInfo.h"
#include "PageDriverRestoreInfo.h"
#include "DDLog.h"

#include <DScrollArea>
#include <DApplicationHelper>
#include <DApplication>
#include <DDialog>

#include <QBoxLayout>
#include <QMetaType>
#include <QNetworkConfigurationManager>
#include <QScrollBar>
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QProcess>

#include <unistd.h>
#include <QLoggingCategory>
using namespace DDLog;
PageDriverManager::PageDriverManager(DWidget *parent)
    : DWidget(parent)
    , mp_StackWidget(new DStackedWidget(this))
    , mp_ListView(new PageListView(this))
    , mp_CurDriverInfo(nullptr)
    , mp_CurBackupDriverInfo(nullptr)
    , mp_CurRestoreDriverInfo(nullptr)
    , m_CurIndex(-1)
    , m_CurBackupIndex(-1)
    , m_CancelIndex(-1)
    , mp_scanner(new DriverScanner(this))
    , mp_DriverInstallInfoPage(new PageDriverInstallInfo(this))
    , mp_DriverBackupInfoPage(new PageDriverBackupInfo(this))
    , mp_DriverRestoreInfoPage(new PageDriverRestoreInfo(this))
    , mp_BackupThread(new DriverBackupThread(this))
{
    mp_ListView->setCurType(tr("Driver Install"));

    // 初始化界面
    initWidget();
    qRegisterMetaType<QList<DriverInfo *>>("QList<DriverInfo*>");
    qRegisterMetaType<ScanResult>("ScanResult");

    // 连接dbus处理信号
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::downloadProgressChanged, this, &PageDriverManager::slotDownloadProgressChanged);//安装
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::downloadFinished, this, &PageDriverManager::slotDownloadFinished);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::installProgressChanged, this, &PageDriverManager::slotInstallProgressChanged);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::installProgressFinished, this, &PageDriverManager::slotInstallProgressFinished);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::installProgressDetail, this, &PageDriverManager::slotRestoreProgress);//还原
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::installFinished, this, &PageDriverManager::slotRestoreFinished);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::backupProgressFinished, this, [=](bool success){
        if (success) {
            mp_BackupThread->setStatus(DriverBackupThread::Success);
        } else {
            mp_BackupThread->setStatus(DriverBackupThread::Failed);
        }
    });

    connect(mp_DriverInstallInfoPage, &PageDriverInstallInfo::operatorClicked, this, &PageDriverManager::slotDriverOperationClicked);
    connect(mp_DriverBackupInfoPage, &PageDriverBackupInfo::operatorClicked, this, &PageDriverManager::slotDriverOperationClicked);
    connect(mp_DriverRestoreInfoPage, &PageDriverRestoreInfo::operatorClicked, this, &PageDriverManager::slotDriverOperationClicked);

    connect(mp_DriverInstallInfoPage, &PageDriverInstallInfo::itemChecked, this, &PageDriverManager::slotItemCheckedClicked);
    connect(mp_DriverBackupInfoPage, &PageDriverBackupInfo::itemChecked, this, &PageDriverManager::slotBackupItemCheckedClicked);

    connect(mp_DriverInstallInfoPage, &PageDriverInstallInfo::installAll, this, &PageDriverManager::slotInstallAllDrivers);
    connect(mp_DriverInstallInfoPage, &PageDriverInstallInfo::undoInstall, this, &PageDriverManager::slotUndoInstall);
    connect(mp_DriverInstallInfoPage, &PageDriverInstallInfo::redetected, this, &PageDriverManager::startScanning);

    connect(mp_DriverBackupInfoPage, &PageDriverBackupInfo::backupAll, this, &PageDriverManager::slotBackupAllDrivers);
    connect(mp_DriverBackupInfoPage, &PageDriverBackupInfo::undoBackup, this, &PageDriverManager::slotUndoBackup);
    connect(mp_DriverBackupInfoPage, &PageDriverBackupInfo::redetected, this, &PageDriverManager::startScanning);

    connect(mp_DriverRestoreInfoPage, &PageDriverRestoreInfo::gotoBackup, this, [=](){
        mp_ListView->setCurType(tr("Driver Backup"));
        mp_ListView->itemClicked(tr("Driver Backup"));
    });
    connect(mp_DriverRestoreInfoPage, &PageDriverRestoreInfo::redetected, this, &PageDriverManager::startScanning);

    connect(mp_ListView, &PageListView::itemClicked, this, &PageDriverManager::slotListViewWidgetItemClicked);

    // 将扫描动作放到线程里面
    connect(mp_scanner, &DriverScanner::scanInfo, this, &PageDriverManager::scanInfo);
    connect(mp_scanner, &DriverScanner::scanFinished, this, &PageDriverManager::slotScanFinished);

    connect(mp_BackupThread, &DriverBackupThread::backupProgressChanged, this, &PageDriverManager::slotBackupProgressChanged);
    connect(mp_BackupThread, &DriverBackupThread::backupProgressFinished, this, &PageDriverManager::slotBackupFinished);

    // 左侧list，内容固定
    QList<QPair<QString, QString> > list;
    list.append(QPair<QString, QString>(tr("Driver Install"), "driverinstall##Overview"));
    list.append(QPair<QString, QString>(tr("Driver Backup"), "driverbackup##Overview"));
    list.append(QPair<QString, QString>(tr("Driver Restore"), "driverrestore##Overview"));
    updateListView(list);
}

PageDriverManager::~PageDriverManager()
{
    // 扫描驱动时关闭线程
    if (mp_scanner->isRunning()) {
        mp_scanner->terminate();
        mp_scanner->wait();
    }

    if (mp_BackupThread->isRunning()) {
        mp_BackupThread->terminate();
        mp_BackupThread->wait();
    }

    DELETE_PTR(mp_ListView);
    DELETE_PTR(mp_StackWidget);
    DELETE_PTR(mp_CurDriverInfo);
    DELETE_PTR(mp_CurBackupDriverInfo);
    DELETE_PTR(mp_CurRestoreDriverInfo);
}

void PageDriverManager::addDriverInfo(DriverInfo *info)
{
    m_ListDriverInfo.append(info);
}

bool PageDriverManager::isFirstScan()
{
    if (m_IsFirstScan)
        return true;
    if (m_Scanning || networkIsOnline())
        return false;
    return true;
}

bool PageDriverManager::isInstalling()
{
    return mp_CurDriverInfo != nullptr;
}

bool PageDriverManager::isBackingup()
{
    return mp_CurBackupDriverInfo != nullptr;

}

bool PageDriverManager::isRestoring()
{
    return mp_CurRestoreDriverInfo != nullptr;
}

bool PageDriverManager::isScanning()
{
    return m_Scanning;
}

void PageDriverManager::updateListView(const QList<QPair<QString, QString> > &lst)
{
    // 更新左边的列表
    if (mp_ListView)
        mp_ListView->updateListItems(lst);
}

void PageDriverManager::scanDriverInfo()
{
    if (m_Scanning)
        return;
    m_IsFirstScan = false;
    m_Scanning = true;
    // 如果在安装、备份、还原过程中则不扫描
    if (mp_CurDriverInfo || mp_CurBackupDriverInfo || mp_CurRestoreDriverInfo) {
        return;
    }

    // 重新扫描之前清空所有数据
    clearAllData();

    // 获取需要扫描的设备的信息
    scanDevices();
//    testScanDevices();

    // 扫描驱动信息线程
    mp_scanner->setDriverList(m_ListDriverInfo);
    mp_scanner->start();
}

void PageDriverManager::slotDriverOperationClicked(int index, int itemIndex, DriverOperationItem::Mode mode)
{
    mp_DriverInstallInfoPage->headWidget()->setReDetectEnable(false);
    mp_DriverBackupInfoPage->headWidget()->setReDetectEnable(false);
    mp_DriverRestoreInfoPage->headWidget()->setReDetectEnable(false);

    switch (mode) {
    case DriverOperationItem::INSTALL:
    case DriverOperationItem::UPDATE:
        // 如果已经在安装过程中，则直接添加到list
        // 如果不在安装过程中则需要安装
        addToDriverIndex(index);
        if (! mp_CurDriverInfo) {
            installNextDriver();
        }
        break;

    case DriverOperationItem::BACKUP:
        ///驱动备份
        addToBackupIndex(index);
        if (!mp_CurBackupDriverInfo) {
            backupNextDriver();
        }
        break;

    case DriverOperationItem::RESTORE:
        ///驱动还原
        mp_CurRestoreDriverInfo = m_ListDriverInfo[index];
        DBusDriverInterface::getInstance()->installDriver(mp_CurRestoreDriverInfo->backupFileName());
        //将其它项置灰
        for (int restorableIndex : m_ListRestorableIndex) {
            mp_DriverRestoreInfoPage->setItemOperationEnable(restorableIndex, false);
        }

        mp_DriverRestoreInfoPage->headWidget()->setRestoringUI(0, mp_CurRestoreDriverInfo->name());
        break;
    }
}

void PageDriverManager::slotItemCheckedClicked(int index, bool checked)
{
    if (!checked) {
        // 取消选中则从list中删除
        removeFromDriverIndex(index);
    } else {
        // 如果不在安装过程中则直接添加到list
        // 如果在安装过程中：1. 添加到list  2. 将item置灰
        addToDriverIndex(index);
        if (mp_CurDriverInfo) {
            mp_DriverInstallInfoPage->setCheckedCBDisnable();
        }
    }
}

void PageDriverManager::slotBackupItemCheckedClicked(int index, bool checked)
{
    if (!checked) {
        // 取消选中则从list中删除
        removeFromBackupIndex(index);
    } else {
        // 如果不在安装过程中则直接添加到list
        // 如果在备份过程中：1. 添加到list  2. 将item置灰
        addToBackupIndex(index);
        if (mp_CurBackupDriverInfo) {
            mp_DriverBackupInfoPage->setCheckedCBDisnable();
        }
    }
}

void PageDriverManager::slotDownloadProgressChanged(QStringList msg)
{
    if (! mp_CurDriverInfo)
        return;
    // 将下载过程时时更新到表格上方的状态里面 qCInfo(appLog) << "Download ********** " << msg[0] << " , " << msg[1] << " , " << msg[2];
    mp_DriverInstallInfoPage->headWidget()->setDownloadUI(mp_CurDriverInfo->type(), msg[2], msg[1], mp_CurDriverInfo->size(), msg[0].toInt());
    // 设置表格下载中的状态
}

void PageDriverManager::slotDownloadFinished()
{
    if (! mp_CurDriverInfo)
        return;

    mp_DriverInstallInfoPage->slotDownloadFinished(m_CurIndex, mp_CurDriverInfo->status());
}

void PageDriverManager::slotInstallProgressChanged(int progress)
{
    if (! mp_CurDriverInfo)
        return;
    // 当进度小于50时，apt处于下载过程
    if (progress <= 50) {
        if (progress > 45)
            mp_CurDriverInfo->m_Status = ST_INSTALL;
        QString speed = "";
        QString size = "";
        getDownloadInfo(progress * 2, mp_CurDriverInfo->m_Byte, speed, size);
        mp_DriverInstallInfoPage->headWidget()->setDownloadUI(mp_CurDriverInfo->type(), speed, size, mp_CurDriverInfo->size(), progress * 2);
    } else {
        mp_CurDriverInfo->m_Status = ST_INSTALL;
        // 设置表头状态
        mp_DriverInstallInfoPage->headWidget()->setInstallUI(mp_CurDriverInfo->type(), mp_CurDriverInfo->name(), (progress - 50) * 2);
    }

    // 设置表格安装中的状态
    mp_DriverInstallInfoPage->updateItemStatus(m_CurIndex, mp_CurDriverInfo->status());
}

void PageDriverManager::slotInstallProgressFinished(bool bsuccess, int err)
{
    static int successNum = 0;
    static int failedNum = 0;
    if (! mp_CurDriverInfo)
        return;

    // 成功
    if (bsuccess) {
        successNum += 1;
        m_ListUpdateIndex.removeAt(m_ListUpdateIndex.indexOf(m_CurIndex));
    } else { // 失败
        // 通知网络错误
        if (err == EC_NOTIFY_NETWORK) {
            mp_DriverInstallInfoPage->headWidget()->setNetworkErrorUI("0.00MB/s", 0);
            return;
        }

        // 通知重新安装
        if (err == EC_REINSTALL) {
            DBusDriverInterface::getInstance()->installDriver(mp_CurDriverInfo->packages(), mp_CurDriverInfo->debVersion());
            return;
        }

        // 网络错误
        if (err == EC_NETWORK) {
            failAllIndex();
            failedNum += m_ListDriverIndex.size();
            m_ListDriverIndex.clear();
        }

        failedNum += 1;
    }

    // 安装结束后，对应的表格需要设置相应的状态
    mp_CurDriverInfo->m_Status = bsuccess ? ST_SUCESS : ST_FAILED;

    QString errS = DApplication::translate("QObject", CommonTools::getErrorString(err).toStdString().data());
    mp_DriverInstallInfoPage->updateItemStatus(m_CurIndex, mp_CurDriverInfo->status(), errS);


    // 当前驱动安装结束，如果没有其它驱动需要安装，则显示安装结果
    // 如果有其它驱动，则显示安装内容
    if (m_ListDriverIndex.size() > 0) {
        sleep(1);
        installNextDriver();
    } else {
        // 设置头部显示效果
        if (successNum > 0) {
            mp_DriverInstallInfoPage->headWidget()->setInstallSuccessUI(QString::number(successNum), QString::number(failedNum));
        } else {
            mp_DriverInstallInfoPage->headWidget()->setInstallFailedUI();
        }

        if (m_ListUpdateIndex.isEmpty()) {
            mp_DriverInstallInfoPage->setHeaderCbEnable(false);
        } else {
            mp_DriverInstallInfoPage->setHeaderCbEnable(true);
        }
        mp_DriverInstallInfoPage->headWidget()->setReDetectEnable(true);
        mp_DriverBackupInfoPage->headWidget()->setReDetectEnable(true);
        mp_DriverRestoreInfoPage->headWidget()->setReDetectEnable(true);

        mp_CurDriverInfo = nullptr;
        m_CurIndex = -1;
        m_CancelIndex = -1;
        successNum = 0;
        failedNum = 0;
    }
}

void PageDriverManager::slotInstallAllDrivers()
{
    mp_DriverBackupInfoPage->headWidget()->setReDetectEnable(false);
    mp_DriverRestoreInfoPage->headWidget()->setReDetectEnable(false);

    // 开始安装驱动
    installNextDriver();
}

void PageDriverManager::slotBackupAllDrivers()
{
    mp_DriverInstallInfoPage->headWidget()->setReDetectEnable(false);
    mp_DriverRestoreInfoPage->headWidget()->setReDetectEnable(false);

    backupNextDriver();
}

void PageDriverManager::slotScanFinished(ScanResult sr)
{
//     testDevices();

    if (SR_SUCESS == sr) {
        foreach (DriverInfo *info, m_ListDriverInfo) {
            getDebBackupInfo(info);

            mp_DriverInstallInfoPage->addDriverInfoToTableView(info, m_ListDriverInfo.indexOf(info));

            if (!info->debVersion().isEmpty() && info->status() != ST_NOT_INSTALL) {
                //debVersion不为空且debBackupVersion为空，非未安装，表示可备份
                mp_DriverBackupInfoPage->addDriverInfoToTableView(info, m_ListDriverInfo.indexOf(info));
            }

            if (!info->debBackupVersion().isEmpty() && info->debBackupVersion() != info->version())
                mp_DriverRestoreInfoPage->addDriverInfoToTableView(info, m_ListDriverInfo.indexOf(info));

            if (ST_NOT_INSTALL == info->status()) {
                m_ListInstallIndex.append(m_ListDriverInfo.indexOf(info));
            } else if (ST_CAN_UPDATE == info->status()) {
                m_ListUpdateIndex.append(m_ListDriverInfo.indexOf(info));
            } else if (ST_DRIVER_IS_NEW == info->status()) {
                m_ListNewIndex.append(m_ListDriverInfo.indexOf(info));
            }

            if (!info->debVersion().isEmpty() && info->debBackupVersion() != info->debVersion()) {
                m_ListBackableIndex.append(m_ListDriverInfo.indexOf(info));
            }
            if (!info->debBackupVersion().isEmpty()) {
                m_ListBackedupeIndex.append(m_ListDriverInfo.indexOf(info));
            }
            if (!info->debBackupVersion().isEmpty() && info->debBackupVersion() != info->version()) {
                m_ListRestorableIndex.append(m_ListDriverInfo.indexOf(info));
            }
        }

        // 决定显示哪些列表，可安装，可更新，无需安装
        mp_DriverInstallInfoPage->showTables(m_ListInstallIndex.size(), m_ListUpdateIndex.size(), m_ListNewIndex.size());
        mp_DriverBackupInfoPage->showTables(m_ListBackableIndex.size(), m_ListBackedupeIndex.size());
        mp_DriverRestoreInfoPage->showTables(m_ListRestorableIndex.size());

        // 获取已经勾选的驱动index
        m_ListDriverIndex.clear();
        mp_DriverInstallInfoPage->getCheckedDriverIndex(m_ListDriverIndex);
        m_ListBackupIndex.clear();
        mp_DriverBackupInfoPage->getCheckedDriverIndex(m_ListBackupIndex);
    } else if (SR_NETWORD_ERR == sr) {
        mp_scanner->quit();
    }

    // 扫描结束，可以继续扫描
    m_Scanning = false;
    emit scanFinished(sr);
}

void PageDriverManager::slotUndoInstall()
{
    if (m_CancelIndex != m_CurIndex) {
        m_CancelIndex = m_CurIndex;
        DBusDriverInterface::getInstance()->undoInstallDriver();
    }

    for (int index : m_ListDriverIndex) {
        mp_DriverInstallInfoPage->updateItemStatus(index, m_ListDriverInfo[index]->status());
    }
    m_ListDriverIndex.clear();
}

void PageDriverManager::slotUndoBackup()
{
    mp_BackupThread->undoBackup();

    // 直接清空列表，不再备份剩余的驱动
    for (int index : m_ListBackupIndex) {
        mp_DriverBackupInfoPage->updateItemStatus(index, ST_DRIVER_NOT_BACKUP);
    }
    m_ListBackupIndex.clear();
}

void PageDriverManager::slotListViewWidgetItemClicked(const QString &itemStr)
{
    m_CurItemStr = itemStr;
    if (tr("Driver Install") == itemStr) {
        mp_StackWidget->setCurrentIndex(0);
    } else if (tr("Driver Backup") == itemStr) {
        mp_StackWidget->setCurrentIndex(1);
    } else if (tr("Driver Restore") == itemStr) {
        mp_StackWidget->setCurrentIndex(2);
    }
}

void PageDriverManager::slotBackupProgressChanged(int progress)
{

}

void PageDriverManager::slotBackupFinished(bool bsuccess)
{
    if (! mp_CurBackupDriverInfo)
        return;

    if (bsuccess) {
        m_backupSuccessNum += 1;
        // 备份成功, 直接添加到可还原列表,并在可备份列表中移除该项
        m_ListBackedupeIndex.append(m_CurBackupIndex);
        getDebBackupInfo(m_ListDriverInfo[m_CurBackupIndex]);
        if (m_ListDriverInfo[m_CurBackupIndex]->debBackupVersion() != m_ListDriverInfo[m_CurBackupIndex]->version()) {
            m_ListRestorableIndex.append(m_CurBackupIndex);
            mp_DriverRestoreInfoPage->addDriverInfoToTableView(m_ListDriverInfo[m_CurBackupIndex], m_CurBackupIndex);
        }
        mp_DriverRestoreInfoPage->showTables(m_ListRestorableIndex.size());
        m_ListBackableIndex.removeAt(m_ListBackableIndex.indexOf(m_CurBackupIndex));
    } else {
        m_backupFailedNum += 1;
    }

    // 安装结束后，对应的表格需要设置相应的状态
    mp_CurBackupDriverInfo->m_Status = bsuccess ? ST_DRIVER_BACKUP_SUCCESS : ST_DRIVER_BACKUP_FAILED;
    mp_DriverBackupInfoPage->updateItemStatus(m_CurBackupIndex, mp_CurBackupDriverInfo->status());

    if (!m_ListBackupIndex.isEmpty()) {
        sleep(1);
        backupNextDriver();
    } else {
        if (!m_ListBackableIndex.isEmpty()) {
            mp_DriverBackupInfoPage->headWidget()->setBackableDriverUI(m_ListBackableIndex.size(), m_ListBackedupeIndex.size());
            mp_DriverBackupInfoPage->setHeaderCbEnable(true);
        } else {
            mp_DriverBackupInfoPage->headWidget()->setNoBackupDriverUI(m_ListBackableIndex.size(), m_ListBackedupeIndex.size());
            mp_DriverBackupInfoPage->setHeaderCbEnable(false);
        }

        mp_DriverInstallInfoPage->headWidget()->setReDetectEnable(true);
        mp_DriverBackupInfoPage->headWidget()->setReDetectEnable(true);
        mp_DriverRestoreInfoPage->headWidget()->setReDetectEnable(true);

        mp_CurBackupDriverInfo = nullptr;
        m_CurBackupIndex = -1;
        m_backupSuccessNum = 0;
        m_backupFailedNum = 0;
    }
}

void PageDriverManager::slotRestoreProgress(int progress, QString strDeatils)
{
    if(mp_CurRestoreDriverInfo == nullptr)
        return;
    if (progress >= 100) {
        mp_DriverRestoreInfoPage->headWidget()->setRestoreDriverUI(m_ListRestorableIndex.size());
    } else {
        mp_DriverRestoreInfoPage->headWidget()->setRestoringUI(progress, mp_CurRestoreDriverInfo->name());
    }
}

void PageDriverManager::slotRestoreFinished(bool success, QString msg)
{
    int index = -1;
    if (mp_CurRestoreDriverInfo) {
        index = m_ListDriverInfo.indexOf(mp_CurRestoreDriverInfo);
        for (int i = 0; i < m_ListRestorableIndex.size(); i++) {
            if (index == m_ListRestorableIndex[i]) {
                if (success) {
                    //移除已还原项
                    m_ListRestorableIndex.removeAt(i);
                    mp_DriverRestoreInfoPage->headWidget()->setRestoreDriverUI(m_ListRestorableIndex.size());
                } else {
                    //还原失败，弹出提示窗口
                    int clickedButtonIndex = mp_FailedDialog->exec();
                    if (1 == clickedButtonIndex) {
                        //反馈
                        qCDebug(appLog) << __func__ << "fedback....";
                        CommonTools::feedback();
                    }
                }

                //恢复其它置灰项
                for (int backedupeIndex : m_ListRestorableIndex) {
                    if (backedupeIndex != i)
                        mp_DriverRestoreInfoPage->setItemOperationEnable(backedupeIndex, true);
                }
                break;
            }
        }
    }

    mp_DriverInstallInfoPage->headWidget()->setReDetectEnable(true);
    mp_DriverBackupInfoPage->headWidget()->setReDetectEnable(true);
    mp_DriverRestoreInfoPage->headWidget()->setReDetectEnable(true);

    mp_CurRestoreDriverInfo = nullptr;

    //刷新表格内容
    mp_DriverRestoreInfoPage->clearAllData();
    for (int backedupeIndex : m_ListRestorableIndex) {
        mp_DriverRestoreInfoPage->addDriverInfoToTableView(m_ListDriverInfo[backedupeIndex], backedupeIndex);
    }
    mp_DriverRestoreInfoPage->showTables(m_ListRestorableIndex.size());
}

void PageDriverManager::initWidget()
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(mp_ListView);
    mainLayout->addWidget(mp_StackWidget);
    this->setLayout(mainLayout);

    mp_StackWidget->setContentsMargins(10, 10, 10, 10);
    mp_StackWidget->addWidget(mp_DriverInstallInfoPage);
    mp_StackWidget->addWidget(mp_DriverBackupInfoPage);
    mp_StackWidget->addWidget(mp_DriverRestoreInfoPage);

    mp_FailedDialog = new DDialog(this);
    DWidget *contentFrame = new DWidget(this);

    DLabel *retryLabel = new DLabel(this);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    retryLabel->setElideMode(Qt::ElideMiddle);
    retryLabel->setText(QObject::tr("Please try again or give us feedback"));
    vLayout->addWidget(retryLabel, 0, Qt::AlignHCenter);
    contentFrame->setLayout(vLayout);
    mp_FailedDialog->setIcon(QIcon::fromTheme("cautious"));
    mp_FailedDialog->setTitle(QObject::tr("Driver restore failed!"));
    mp_FailedDialog->addContent(contentFrame);
    mp_FailedDialog->addButton(tr("OK"), false, DDialog::ButtonNormal);
    mp_FailedDialog->addButton(tr("Feedback"), false, DDialog::ButtonNormal);
}

void PageDriverManager::installNextDriver()
{
    if (!m_ListDriverIndex.isEmpty()) {
        m_CurIndex = m_ListDriverIndex[0];
        m_ListDriverIndex.removeAt(0);
        DriverInfo *info = m_ListDriverInfo[m_CurIndex];
        if (info) {
            mp_CurDriverInfo = info;
            mp_CurDriverInfo->m_Status = ST_DOWNLOADING;
            mp_DriverInstallInfoPage->updateItemStatus(m_CurIndex, mp_CurDriverInfo->status());
            mp_DriverInstallInfoPage->headWidget()->setDownloadUI(mp_CurDriverInfo->type(), "0MB/s", "0MB", mp_CurDriverInfo->size(), 0);
            DBusDriverInterface::getInstance()->installDriver(info->packages(), info->debVersion());
        }
    }
}

void PageDriverManager::backupNextDriver()
{
    if (!m_ListBackupIndex.isEmpty()) {
        m_CurBackupIndex = m_ListBackupIndex[0];
        m_ListBackupIndex.removeAt(0);
        DriverInfo *info = m_ListDriverInfo[m_CurBackupIndex];
        if (info) {
            mp_CurBackupDriverInfo = info;
            qCDebug(appLog) << __func__ << "backup driver: " << mp_CurBackupDriverInfo->name();
            mp_CurBackupDriverInfo->m_Status = ST_DRIVER_BACKING_UP;
            mp_DriverBackupInfoPage->updateItemStatus(m_CurBackupIndex, mp_CurBackupDriverInfo->status());
            mp_DriverBackupInfoPage->headWidget()->setBackingUpDriverUI(mp_CurBackupDriverInfo->name(),
                                                                        m_ListBackupIndex.size() + m_backupSuccessNum + m_backupFailedNum + 1,
                                                                        m_backupSuccessNum + m_backupFailedNum + 1);
            if (!mp_BackupThread->isRunning()) {
                mp_BackupThread->setBackupDriverInfo(info);
                mp_BackupThread->start();
            }
        }
    }
}

void PageDriverManager::scanDevices()
{
    // 显卡
    scanDevicesInfo(QObject::tr("Display Adapter"), DR_Gpu);

    // 声卡
    scanDevicesInfo(QObject::tr("Sound Adapter"), DR_Sound);

    // 蓝牙
    scanDevicesInfo(QObject::tr("Bluetooth"), DR_Bluetooth);

    // 网卡
    scanDevicesInfo(QObject::tr("Network Adapter"), DR_Network);

    // 图像设备
//    scanDevicesInfo(QObject::tr("Camera"), DR_Camera);

    // 打印机
//    scanDevicesInfo(QObject::tr("Printer"), DR_Printer); //由于task 353401 要求

//    scanDevicesInfo(QObject::tr("Other Devices"), DR_OtherDevice); //由于task 353401 要求
}

void PageDriverManager::testScanDevices()
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (Notebooks) 001";
    info->m_Size = "124.36";
    info->m_Type = DR_Gpu;
    info->m_Status = ST_NOT_INSTALL;
    info->m_Checked = true;
    info->m_DriverName = "lenovo-image-g-series";
    info->m_Packages = "lenovo-image-g-series";
    info->m_DebVersion = "1.0-17";
    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (Notebooks) 002";
    info->m_Size = "124.36";
    info->m_Type = DR_WiFi;
    info->m_Status = ST_NOT_INSTALL;
    info->m_Checked = true;
    info->m_DriverName = "deepin-log-viewer";
    info->m_Packages = "deepin-log-viewer";
    info->m_DebVersion = "5.9.2-1";
    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (Notebooks) 003";
    info->m_Size = "124.36";
    info->m_Type = DR_Scaner;
    info->m_Status = ST_NOT_INSTALL;
    info->m_Checked = true;
    info->m_DriverName = "deepin-reader";
    info->m_Packages = "deepin-reader";
    info->m_DebVersion = "5.10.11-1";
    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (Notebooks) 004";
    info->m_Size = "124.36";
    info->m_Type = DR_Bluetooth;
    info->m_Status = ST_NOT_INSTALL;
    info->m_Checked = true;
    info->m_DriverName = "deepin-diskmanager";
    info->m_Packages = "deepin-diskmanager";
    info->m_DebVersion = "1.3.10-1";
    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (Notebooks) 005";
    info->m_Size = "124.36";
    info->m_Type = DR_Sound;
    info->m_Status = ST_CAN_UPDATE;
    info->m_Checked = true;
    info->m_DriverName = "uos-remote-assistance";
    info->m_Packages = "uos-remote-assistance";
    info->m_DebVersion = "1.0.7-1";
    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (update)";
    info->m_Size = "124.36";
    info->m_Type = DR_Printer;
    info->m_Status = ST_CAN_UPDATE;
    info->m_Checked = false;
    info->m_DriverName = "deepin-calculator";
    info->m_Packages = "deepin-calculator";
    info->m_DebVersion = "5.7.12-1";
    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (update)";
    info->m_Size = "124.36";
    info->m_Type = DR_Sound;
    info->m_Status = ST_CAN_UPDATE;
    info->m_Checked = false;
    info->m_DriverName = "deepin-defender";
    info->m_Packages = "deepin-defender";
    info->m_DebVersion = "3.0.21-1";
//    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (update)";
    info->m_Size = "124.36";
    info->m_Type = DR_Sound;
    info->m_Status = ST_CAN_UPDATE;
    info->m_Checked = false;
    info->m_DriverName = "deepin-system-monitor";
    info->m_Packages = "deepin-system-monitor";
    info->m_DebVersion = "5.8.17-1";
//    addDriverInfo(info);

    info = new DriverInfo();
    info->m_Name = "GeFore RTX30 Series (update)";
    info->m_Size = "124.36";
    info->m_Type = DR_Sound;
    info->m_Status = ST_DRIVER_IS_NEW;
    info->m_Checked = false;
    info->m_DriverName = "dde-introduction";
    info->m_Packages = "dde-introduction";
    info->m_DebVersion = "5.6.18-1";
    addDriverInfo(info);

}

void PageDriverManager::scanDevicesInfo(const QString &deviceType, DriverType driverType)
{
    // 显卡
    QList<DeviceBaseInfo *> lst;
    bool ret = DeviceManager::instance()->getDeviceList(deviceType, lst);
    if (ret) {
        foreach (DeviceBaseInfo *device, lst) {
            DriverInfo *info = new DriverInfo();
            info->m_Name = device->name();        // 设备名称
            info->m_Type = driverType;
            info->m_Status = ST_DRIVER_IS_NEW;    // 默认已安装驱动为最新
            info->m_VendorId = device->getVendorOrModelId(device->sysPath(), true);    // vendor id
            info->m_ModelId = device->getVendorOrModelId(device->sysPath(), false);    // model id
            info->m_DriverName = device->driver();                 // 驱动名称
            info->m_Version = device->getDriverVersion();          // 驱动版本

            qCInfo(appLog) << "m_Name" << info->m_Name;
            qCInfo(appLog) << "m_VendorId" << info->m_VendorId;
            qCInfo(appLog) << "m_ModelId" << info->m_ModelId;
            qCInfo(appLog) << "m_Status" << info->m_Status;
            qCInfo(appLog) << "m_Type" << info->m_Type;

            DevicePrint *print = dynamic_cast<DevicePrint *>(device);
            if (print != nullptr) {
                info->m_VendorName = device->vendor();           // vendor name
                info->m_ModelName = print->makeAndeModel();      // model name
            }

            DeviceNetwork *network = dynamic_cast<DeviceNetwork *>(device);
            if (network != nullptr) {
                if (network->isWireless()) {
                    info->m_Type = DR_WiFi;
                }

            }

            addDriverInfo(info);
        }
    }
}

void PageDriverManager::clearAllData()
{
    m_ListNewIndex.clear();
    m_ListDriverIndex.clear();
    m_ListUpdateIndex.clear();
    m_ListInstallIndex.clear();
    m_ListBackupIndex.clear();
    m_ListBackableIndex.clear();
    m_ListBackedupeIndex.clear();
    m_ListRestorableIndex.clear();

    foreach (DriverInfo *info, m_ListDriverInfo) {
        delete info;
    }
    m_ListDriverInfo.clear();

    mp_DriverInstallInfoPage->clearAllData();
    mp_DriverBackupInfoPage->clearAllData();
    mp_DriverRestoreInfoPage->clearAllData();
}

void PageDriverManager::addToDriverIndex(int index)
{
    // 如果第一次添加到index，一键安装按钮置灰
    if (m_ListDriverIndex.size() <= 0) {
        mp_DriverInstallInfoPage->headWidget()->setInstallBtnEnable(true);
    }

    bool add = true;
    for (int i = 0; i < m_ListDriverIndex.size(); i++) {
        if (index == m_ListDriverIndex[i]) {
            add = false;
            break;
        }
    }

    if (add) {
        m_ListDriverIndex.append(index);
    }
}

void PageDriverManager::addToBackupIndex(int index)
{
    if (m_ListBackupIndex.isEmpty()) {
        mp_DriverBackupInfoPage->headWidget()->setBackupBtnEnable(true);
    }

    int ret = m_ListBackupIndex.indexOf(index);

    if (ret < 0) {
        m_ListBackupIndex.append(index);
    }
}

void PageDriverManager::removeFromDriverIndex(int index)
{
    m_ListDriverIndex.removeAt(m_ListDriverIndex.indexOf(index));

    // 移除时 如果一个都没有 则一键安装按钮置灰
    if (m_ListDriverIndex.isEmpty()) {
        mp_DriverInstallInfoPage->headWidget()->setInstallBtnEnable(false);
    }
}

void PageDriverManager::removeFromBackupIndex(int index)
{
    m_ListBackupIndex.removeAt(m_ListBackupIndex.indexOf(index));

    if (m_ListBackupIndex.isEmpty()) {
        mp_DriverBackupInfoPage->headWidget()->setBackupBtnEnable(false);
    }
}

void PageDriverManager::failAllIndex()
{
    foreach (int index, m_ListDriverIndex) {
        QString errS = DApplication::translate("QObject", CommonTools::getErrorString(EC_NETWORK).toStdString().data());
        mp_DriverInstallInfoPage->updateItemStatus(index, ST_FAILED, errS);
    }
}

bool PageDriverManager::networkIsOnline()
{
    QNetworkConfigurationManager mgr;
    return mgr.isOnline();
}

void PageDriverManager::getDownloadInfo(int progress, qint64 total, QString &speed, QString &size)
{
    static qint64 msec = QDateTime::currentMSecsSinceEpoch();
    static double pre_bytes = total * (progress / 100.0);
    double bytes = total * (progress / 100.0);
    if (pre_bytes > bytes) {
        pre_bytes = 0;
    }

    // 已下载

    if (bytes < 1024 * 1024) {
        size = QString::number(bytes / 1024, 'f', 2) + "KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        size = QString::number(bytes / 1024 / 1024, 'f', 2) + "MB";
    } else {
        size = QString::number(bytes / 1024 / 1024 / 1024, 'f', 2) + "GB";
    }

    // 速度
    double detal_bytes = bytes - pre_bytes;
    pre_bytes = bytes;
    qint64 detal_time = QDateTime::currentMSecsSinceEpoch() - msec;
    double speed_s = detal_bytes / (detal_time / 1000.0);
    if (0 == detal_time) {
        speed_s = 0;
    }
    if (speed_s < 1024 * 1024) {
        speed = QString::number(speed_s / 1024, 'f', 2) + "KB";
    } else if (speed_s < 1024 * 1024 * 1024) {
        speed = QString::number(speed_s / 1024 / 1024, 'f', 2) + "MB";
    } else {
        speed = QString::number(speed_s / 1024 / 1024 / 1024, 'f', 2) + "GB";
    }
    msec = QDateTime::currentMSecsSinceEpoch();
}

void PageDriverManager::testDevices()
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = "Sharp MX-C2622R PS, 1.1";
    info->m_Version = "0.0.1";
    info->m_DebVersion = "1.0.0";
    info->m_Size = "23.2";
    info->m_Type = DR_Tablet;
    info->m_Packages = "com.sharp.griffin2light";
    info->m_Status = ST_CAN_UPDATE;
    m_ListDriverInfo.append(info);

    DriverInfo *info1 = new DriverInfo();
    info1->m_Name = "Deli DL-888F";
    info1->m_Version = "1.0";
    info1->m_DebVersion = "1.1";
    info1->m_Size = "102.45";
    info1->m_Type = DR_Mouse;
    info1->m_Packages = "deli-printer";
    info1->m_Status = ST_CAN_UPDATE;
    m_ListDriverInfo.append(info1);

//    DriverInfo *info2 = new DriverInfo();
//    info2->m_Name = "DONNA LQ Foomatic/DONNA (recommended)";
//    info2->m_DebVersion = "1.0.0.1";
//    info2->m_Type = DR_Printer;
//    info2->m_Packages = "com.cn.donna.printer-lq-drv";
//    info2->m_Status = ST_CAN_UPDATE;
//    m_ListDriverInfo.append(info2);

    DriverInfo *info3 = new DriverInfo();
    info3->m_Name = "Aurora AD229MNA Series";
    info3->m_DebVersion = "1.0.0";
    info3->m_Type = DR_WiFi;
    info3->m_Packages = "aurora.a4-adxxx";
    info3->m_Status = ST_CAN_UPDATE;
    m_ListDriverInfo.append(info3);
}

void PageDriverManager::getDebBackupInfo(DriverInfo *info)
{
    QString debname = info->packages();
    QString backupPath =  QString("%1/driver/%2").arg(CommonTools::getBackupPath()).arg(debname);
    QDir destdir(backupPath);
    if (destdir.exists()) {
        //获取当前路径下的所有文件名
        QFileInfoList fileInfoList = destdir.entryInfoList();
        foreach (QFileInfo fileInfo, fileInfoList) {
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;
            if (fileInfo.isFile() && fileInfo.fileName().contains(".deb") && fileInfo.fileName().contains(debname)) {
                QString tmps = fileInfo.fileName().remove(debname).remove(".deb");
                QStringList  tmpsl = tmps.split('_', QString::SkipEmptyParts);
                info->m_DebBackupVersion = tmpsl.first();
                info->m_BackupFileName = fileInfo.absoluteFilePath();
            }
        }
    }

}
