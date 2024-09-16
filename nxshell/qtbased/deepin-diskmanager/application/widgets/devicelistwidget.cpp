// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "devicelistwidget.h"
#include "customcontrol/dmdiskinfobox.h"
#include "diskinfodisplaydialog.h"
#include "diskhealthdetectiondialog.h"
#include "messagebox.h"
#include "partitiontableerrorsinfodialog.h"
#include "diskbadsectorsdialog.h"
#include "createpartitiontabledialog.h"
#include "createlvwidget.h"
#include "partitiondialog.h"

#include <DPalette>
#include <DMenu>
#include <DMessageManager>

#include <QVBoxLayout>
#include <QDebug>
#include <QApplication>

DeviceListWidget::DeviceListWidget(QWidget *parent)
    : DWidget(parent)
{
    m_curChooseDevicePath = "";

    setAutoFillBackground(true);
    auto plt = this->palette();
    plt.setBrush(QPalette::Background, QBrush(Qt::white));
    setPalette(plt);
    setMaximumWidth(400);
    setMinimumWidth(180);
    initUi();
    initConnection();
}

DeviceListWidget::~DeviceListWidget()
{
    //    delete m_box;
    //    delete m_childbox;
}

void DeviceListWidget::initUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    m_treeView = new DmTreeview(this);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_treeView);
}

void DeviceListWidget::initConnection()
{
    connect(DMDbusHandler::instance(), &DMDbusHandler::updateDeviceInfo, this, &DeviceListWidget::onUpdateDeviceInfo);
    connect(m_treeView, &DmTreeview::curSelectChanged, DMDbusHandler::instance(), &DMDbusHandler::onSetCurSelect);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &DeviceListWidget::treeMenu);
    connect(DMDbusHandler::instance(), &DMDbusHandler::unmountPartitionMessage, this, &DeviceListWidget::onUnmountPartition);
    connect(DMDbusHandler::instance(), &DMDbusHandler::deletePartitionMessage, this, &DeviceListWidget::onDeletePartition);
    connect(DMDbusHandler::instance(), &DMDbusHandler::hidePartitionMessage, this, &DeviceListWidget::onHidePartition);
    connect(DMDbusHandler::instance(), &DMDbusHandler::showPartitionMessage, this, &DeviceListWidget::onShowPartition);
    connect(DMDbusHandler::instance(), &DMDbusHandler::createPartitionTableMessage, this, &DeviceListWidget::onCreatePartitionTableMessage);
    connect(DMDbusHandler::instance(), &DMDbusHandler::updateUsb, this, &DeviceListWidget::onUpdateUsb);
    connect(DMDbusHandler::instance(), &DMDbusHandler::vgDeleteMessage, this, &DeviceListWidget::onVGDeleteMessage);
    connect(DMDbusHandler::instance(), &DMDbusHandler::lvDeleteMessage, this, &DeviceListWidget::onLVDeleteMessage);
    connect(DMDbusHandler::instance(), &DMDbusHandler::createFailedMessage, this, &DeviceListWidget::onCreateFailedMessage);
}

void DeviceListWidget::treeMenu(const QPoint &pos)
{
    QModelIndex curIndex = m_treeView->indexAt(pos); //当前点击的元素的index
    QModelIndex index = curIndex.sibling(curIndex.row(), 0); //该行的第1列元素的index

    if (!index.isValid()) {
        return;
    }

    m_curDiskInfoData = m_treeView->getCurItem()->data().value<DiskInfoData>();
    if (m_curDiskInfoData.m_level == DMDbusHandler::DISK) {
        QMenu *menu = new QMenu(this);
        menu->setObjectName("treeMenu");
        menu->setAccessibleName("menu");

        QAction *actionInfo = new QAction(this);
        actionInfo->setText(tr("Disk info")); // 磁盘信息
        actionInfo->setObjectName("Disk info");
        menu->addAction(actionInfo);
        connect(actionInfo, &QAction::triggered, this, &DeviceListWidget::onDiskInfoClicked);

        //        menu->addSeparator();    //添加一个分隔线
        QMenu *itemChildMenu = new QMenu(this);
        itemChildMenu->setTitle(tr("Health management")); // 健康管理
        itemChildMenu->setObjectName("Health management");
        itemChildMenu->setAccessibleName("Health management");
        menu->addMenu(itemChildMenu);

        QAction *actionHealthDetection = new QAction(this);
        actionHealthDetection->setText(tr("Check health")); // 硬盘健康检测
        actionHealthDetection->setObjectName("Check health");
        itemChildMenu->addAction(actionHealthDetection);
        connect(actionHealthDetection, &QAction::triggered, this, &DeviceListWidget::onDiskCheckHealthClicked);

        QAction *actionCheckError = new QAction(this);
        actionCheckError->setText(tr("Check partition table error")); // 分区表错误检测
        actionCheckError->setObjectName("Check partition table error");
        itemChildMenu->addAction(actionCheckError);
        connect(actionCheckError, &QAction::triggered, this, &DeviceListWidget::onPartitionErrorCheckClicked);

        QAction *actionVerifyRepair = new QAction(this);
        actionVerifyRepair->setText(tr("Verify or repair bad sectors")); // 坏道检测与修复
        actionVerifyRepair->setObjectName("Verify or repair bad sectors");
        itemChildMenu->addAction(actionVerifyRepair);
        connect(actionVerifyRepair, &QAction::triggered, this, &DeviceListWidget::onDiskBadSectorsClicked);

        QAction *createPartitionTable = new QAction(this);
        createPartitionTable->setText(tr("Create partition table")); // 新建分区表
        createPartitionTable->setObjectName("Create partition table");
        menu->addAction(createPartitionTable);
        connect(createPartitionTable, &QAction::triggered, this, &DeviceListWidget::onCreatePartitionTableClicked);

        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        if (info.m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV || info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            createPartitionTable->setDisabled(true);
        }

        menu->exec(QCursor::pos()); //显示菜单
        delete menu;
    } else if (m_curDiskInfoData.m_level == DMDbusHandler::PARTITION) {
        QMenu *menu = new QMenu(this);
        menu->setObjectName("treeMenu");
        menu->setAccessibleName("menu");

        //        QAction *actionHidePartition = new QAction(this);
        //        actionHidePartition->setText(tr("Hide partition")); // 隐藏分区
        //        actionHidePartition->setObjectName("Hide partition");
        //        menu->addAction(actionHidePartition);
        //        connect(actionHidePartition, &QAction::triggered, this, &DeviceListWidget::onHidePartitionClicked);

        //        QAction *actionShowPartition = new QAction(this);
        //        actionShowPartition->setText(tr("Unhide partition")); // 显示分区
        //        actionShowPartition->setObjectName("Unhide partition");
        //        menu->addAction(actionShowPartition);
        //        connect(actionShowPartition, &QAction::triggered, this, &DeviceListWidget::onShowPartitionClicked);
        //        //            actionShowPartition->setDisabled(true); // 将按钮置为不可选

        QAction *actionDelete = new QAction(this);
        actionDelete->setText(tr("Delete partition")); // 删除分区
        actionDelete->setObjectName("Delete partition");
        menu->addAction(actionDelete);
        connect(actionDelete, &QAction::triggered, this, &DeviceListWidget::onDeletePartitionClicked);

        bool isJoinVg = false;
        if (m_curDiskInfoData.m_fstype == "extended") {
            DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
            for (int i = 0; i < info.m_partition.size(); i++) {
                if (info.m_partition.at(i).m_type == PartitionType::TYPE_LOGICAL) {
                    if (info.m_partition.at(i).m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV) {
                        isJoinVg = true;
                        break;
                    }
                }
            }
        }

        if (!m_curDiskInfoData.m_mountpoints.isEmpty() || isJoinVg) {
            actionDelete->setDisabled(true);
        }

        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            if (luksInfo.isDecrypt) {
                if (!luksInfo.m_mapper.m_mountPoints.isEmpty() || luksInfo.m_mapper.m_luksFs == FSType::FS_LVM2_PV) {
                    actionDelete->setDisabled(true);
                }
            }
        }

        if (m_curDiskInfoData.m_fstype == "unallocated" || m_curDiskInfoData.m_fstype == "linux-swap"
            || info.m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV) {
            //            actionHidePartition->setDisabled(true);
            //            actionShowPartition->setDisabled(true);
            actionDelete->setDisabled(true);
        } /*else {
            if (1 == m_curDiskInfoData.m_flag) {
                actionHidePartition->setDisabled(true);
                actionShowPartition->setDisabled(false);
            } else {
                actionHidePartition->setDisabled(false);
                actionShowPartition->setDisabled(true);
            }
        }*/

        menu->exec(QCursor::pos()); //显示菜单
        delete menu;
    } else if (m_curDiskInfoData.m_level == DMDbusHandler::VOLUMEGROUP) {
        QMenu *menu = new QMenu(this);
        menu->setObjectName("treeMenu");
        menu->setAccessibleName("menu");

        QAction *actionDelete = new QAction(this);
        actionDelete->setText(tr("Delete volume group")); // 删除逻辑卷组
        actionDelete->setObjectName("Delete volume group");
        menu->addAction(actionDelete);
        connect(actionDelete, &QAction::triggered, this, &DeviceListWidget::onDeleteVGClicked);

        QAction *actionCreate = new QAction(this);
        actionCreate->setText(tr("Create logical volume")); // 创建逻辑卷
        actionCreate->setObjectName("Create logical volume");
        menu->addAction(actionCreate);
        connect(actionCreate, &QAction::triggered, this, &DeviceListWidget::onCreateLVClicked);

        QMap<QString, QString> isExistUnallocated = DMDbusHandler::instance()->getIsExistUnallocated();
        if (isExistUnallocated.value(m_curDiskInfoData.m_diskPath) == "false") {
            actionCreate->setDisabled(true);
        }

        // 如果VG异常，所有相关操作禁用
        VGInfo vgInfo = DMDbusHandler::instance()->getCurVGInfo();
        if (vgInfo.isPartial()) {
            actionDelete->setDisabled(true);
            actionCreate->setDisabled(true);
        }

        menu->exec(QCursor::pos()); //显示菜单
        delete menu;
    } else if (m_curDiskInfoData.m_level == DMDbusHandler::LOGICALVOLUME) {
        QMenu *menu = new QMenu(this);
        menu->setObjectName("treeMenu");
        menu->setAccessibleName("menu");

        QAction *actionDelete = new QAction(this);
        actionDelete->setText(tr("Delete logical volume")); // 删除逻辑卷
        actionDelete->setObjectName("Delete logical volume");
        menu->addAction(actionDelete);
        connect(actionDelete, &QAction::triggered, this, &DeviceListWidget::onDeleteLVClicked);

        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();
        if (lvInfo.m_lvName.isEmpty() && lvInfo.m_lvUuid.isEmpty()) {
            actionDelete->setDisabled(true);
        }

        // 如果VG异常，删除逻辑卷操作禁用
        VGInfo vgInfo = DMDbusHandler::instance()->getCurVGInfo();
        if (vgInfo.isPartial() || lvInfo.m_lvFsType == FSType::FS_LINUX_SWAP) {
            actionDelete->setDisabled(true);
        }

        menu->exec(QCursor::pos()); //显示菜单
        delete menu;
    }
}

void DeviceListWidget::onDiskInfoClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    DiskInfoDisplayDialog diskInfoDisplayDialog(m_curDiskInfoData.m_diskPath, this);
    diskInfoDisplayDialog.setObjectName("diskInfoDisplayDialog");
    diskInfoDisplayDialog.setAccessibleName("diskInfoDialog");
    diskInfoDisplayDialog.exec();

    setCurDevicePath("");
}

void DeviceListWidget::onDiskCheckHealthClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    HardDiskStatusInfoList hardDiskStatusInfoList = DMDbusHandler::instance()->getDeviceHardStatusInfo(m_curDiskInfoData.m_diskPath);
    if (hardDiskStatusInfoList.count() < 1) {
        MessageBox warningBox(this);
        warningBox.setObjectName("messageBox");
        warningBox.setAccessibleName("warningMessageBox");
        // 获取不到硬件相应信息  关闭
        warningBox.setWarings(tr("Health checking does not support this type of device."), "", tr("Close"), "close");
        warningBox.exec();

        setCurDevicePath("");

        return;
    }

    DiskHealthDetectionDialog diskHealthDetectionDialog(m_curDiskInfoData.m_diskPath, hardDiskStatusInfoList, this);
    diskHealthDetectionDialog.setObjectName("diskHealthDetectionDialog");
    diskHealthDetectionDialog.setAccessibleName("diskHealthInfoDialog");
    diskHealthDetectionDialog.exec();

    setCurDevicePath("");
}

void DeviceListWidget::onDiskBadSectorsClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    DiskBadSectorsDialog diskBadSectorsDialog(this);
    diskBadSectorsDialog.setObjectName("diskBadSectorsDialog");
    diskBadSectorsDialog.setAccessibleName("diskBadSectorsDialog");
    diskBadSectorsDialog.exec();

    setCurDevicePath("");
}

void DeviceListWidget::onCreatePartitionTableClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    if (DMDbusHandler::instance()->isExistMountPartition(DMDbusHandler::instance()->getCurDeviceInfo())) {
        MessageBox warningBox(this);
        warningBox.setObjectName("messageBox");
        warningBox.setAccessibleName("messageBox");
        // 请先卸载当前磁盘中的所有分区  确定
        warningBox.setWarings(tr("Please unmount all partitions in the disk first"), "", tr("OK"), "ok");
        warningBox.exec();

        setCurDevicePath("");

        return;
    }

    MessageBox messageBox(this);
    messageBox.setObjectName("messageBox");
    messageBox.setAccessibleName("messageBox");
    // 新建分区表之后将会合并当前磁盘所有分区，丢失所有数据，请谨慎使用  继续  取消
    messageBox.setWarings(tr("All partitions in this disk will be merged and all data\n will be lost if creating a new partition table,\n please take it carefully"), "", tr("Proceed"), "Proceed", tr("Cancel"), "cancelBtn");
    if (messageBox.exec() == DDialog::Accepted) {
        CreatePartitionTableDialog createPartitionTableDialog(this);
        createPartitionTableDialog.setObjectName("createPartitionTable");
        createPartitionTableDialog.setAccessibleName("createPartitionTableDialog");
        createPartitionTableDialog.exec();
    }

    setCurDevicePath("");
}

void DeviceListWidget::onPartitionErrorCheckClicked()
{
    bool result = DMDbusHandler::instance()->detectionPartitionTableError(m_curDiskInfoData.m_diskPath);
    if (result) {
        QString deviceInfo = QString("%1(%2)").arg(m_curDiskInfoData.m_diskPath).arg(m_curDiskInfoData.m_diskSize);
        setCurDevicePath(m_curDiskInfoData.m_diskPath);

        PartitionTableErrorsInfoDialog partitionTableErrorsInfoDialog(deviceInfo, this);
        partitionTableErrorsInfoDialog.setObjectName("partitionErrorCheck");
        partitionTableErrorsInfoDialog.setAccessibleName("partitionErrorCheckDialog");
        partitionTableErrorsInfoDialog.exec();

        setCurDevicePath("");
    } else {
        // 分区表检测正常
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("No errors found in the partition table"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onHidePartitionClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    MessageBox messageBox(this);
    messageBox.setObjectName("messageBox");
    messageBox.setAccessibleName("hideMessageBox");
    // 您是否要隐藏该分区？ 隐藏  取消
    messageBox.setWarings(tr("Do you want to hide this partition?"), "", tr("Hide"), "hide", tr("Cancel"), "cancel");
    if (messageBox.exec() == DDialog::Accepted) {
        if (m_curDiskInfoData.m_mountpoints == "/boot/efi" || m_curDiskInfoData.m_mountpoints == "/boot"
            || m_curDiskInfoData.m_mountpoints == "/" || m_curDiskInfoData.m_mountpoints == "/data/home/opt/root/var"
            || m_curDiskInfoData.m_mountpoints == "/deepin/userdata/home/opt/root/var") {
            isHideSuccess = false;
            // 隐藏分区失败！无法锁定该分区
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Failed to hide the partition: unable to lock it"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
        } else {
            if (m_curDiskInfoData.m_mountpoints.isEmpty()) {
                DMDbusHandler::instance()->hidePartition();
            } else {
                isHideSuccess = false;
                // 只有处于卸载状态的分区才能被隐藏
                DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("You can only hide the unmounted partition"));
                DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
            }
        }
    }

    setCurDevicePath("");
}

void DeviceListWidget::onShowPartitionClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    MessageBox messageBox(this);
    messageBox.setObjectName("messageBox");
    messageBox.setAccessibleName("unhideMessageBox");
    // 您是否要显示该隐藏分区？ 显示  取消
    messageBox.setWarings(tr("Do you want to unhide this partition?"), "", tr("Unhide"), "unhide", tr("Cancel"), "cancel");
    if (messageBox.exec() == DDialog::Accepted) {
        DMDbusHandler::instance()->unhidePartition();
    }

    setCurDevicePath("");
}

void DeviceListWidget::onDeletePartitionClicked()
{
    setCurDevicePath(m_curDiskInfoData.m_diskPath);

    MessageBox messageBox(this);
    messageBox.setObjectName("messageBox");
    messageBox.setAccessibleName("deleteMessageBox");
    // 您确定要删除该分区吗？ 该分区内所有文件将会丢失  删除  取消
    messageBox.setWarings(tr("Are you sure you want to delete this partition?"), tr("You will lose all data in it"), tr("Delete"), DDialog::ButtonWarning, "delete", tr("Cancel"), "cancel");
    if (messageBox.exec() == DDialog::Accepted) {
        if (m_curDiskInfoData.m_mountpoints == "/boot/efi" || m_curDiskInfoData.m_mountpoints == "/boot"
            || m_curDiskInfoData.m_mountpoints == "/" || m_curDiskInfoData.m_mountpoints == "/data/home/opt/root/var"
            || m_curDiskInfoData.m_fstype == "linux-swap" || m_curDiskInfoData.m_mountpoints == "/deepin/userdata/home/opt/root/var") {
            isDeleteSuccess = false;
            // 删除分区失败！无法锁定该分区
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Failed to delete the partition: unable to lock it"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
        } else {
            DMDbusHandler::instance()->deletePartition();
        }
    }

    setCurDevicePath("");
}

void DeviceListWidget::onHidePartition(const QString &hideMessage)
{
    if ("1" == hideMessage) {
        // 隐藏分区成功
        isHideSuccess = true;
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Hide the partition successfully"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    } else {
        isHideSuccess = false;
        // 隐藏分区失败
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Failed to hide the partition"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onShowPartition(const QString &showMessage)
{
    if ("1" == showMessage) {
        // 显示分区成功
        isShowSuccess = true;
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Unhide the partition successfully"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    } else {
        isShowSuccess = false;
        // 显示分区失败
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Failed to unhide the partition"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onDeletePartition(const QString &deleteMessage)
{
    QStringList infoList = deleteMessage.split(":");

    if (infoList.count() <= 1) {
        return;
    }

    if ("1" == infoList.at(0)) {
        // 删除分区成功
        isDeleteSuccess = true;
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Delete the partition successfully"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    } else {
        isDeleteSuccess = false;

        QString reason;
        switch (infoList.at(1).toInt()) {
        case DISK_ERROR::DISK_ERR_DISK_INFO: {
            reason = tr("Failed to find the disk");
            break;
        }
        case DISK_ERROR::DISK_ERR_PART_INFO: {
            reason = tr("Failed to get the partition info");
            break;
        }
        case DISK_ERROR::DISK_ERR_DELETE_PART_FAILED: {
            reason = tr("Failed to delete the partition");
            break;
        }
        case DISK_ERROR::DISK_ERR_UPDATE_KERNEL_FAILED: {
            reason = tr("Failed to submit the request to the kernel");
            break;
        }
        default:
            break;
        }
        // 删除分区失败
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Failed to delete the partition: %1").arg(reason));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onUnmountPartition(const QString &unmountMessage)
{
    if ("1" == unmountMessage) {
        isUnmountSuccess = true;
        // 卸载成功
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Unmounting successful"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    } else {
        isUnmountSuccess = false;
        // 卸载失败
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Unmounting failed"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onCreatePartitionTableMessage(const bool &flag)
{
    if (flag) {
        isCreateTableSuccess = true;
        if (DMDbusHandler::instance()->getCurCreatePartitionTableType() == "create") {
            // 新建分区表成功
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Creating partition table successful"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
        } else {
            // 替换分区表成功
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Replacing partition table successful"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
        }

    } else {
        isCreateTableSuccess = false;
        if (DMDbusHandler::instance()->getCurCreatePartitionTableType() == "create") {
            // 新建分区表失败
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Creating partition table failed"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
        } else {
            // 替换分区表失败
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Replacing partition table failed"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
        }
    }
}

void DeviceListWidget::onDeleteVGClicked()
{
    VGInfo vgInfo = DMDbusHandler::instance()->getCurVGInfo();
    setCurVGName(vgInfo.m_vgName);
    if (DMDbusHandler::instance()->isExistMountLV(vgInfo)) {
        MessageBox warningBox(this);
        warningBox.setObjectName("messageBox");
        warningBox.setAccessibleName("messageBox");
        // 请先手动卸载XXX（逻辑卷组的名称）下所有的逻辑卷  确定
        warningBox.setWarings(tr("Unmount all logical volumes in %1 first").arg(vgInfo.m_vgName), "", tr("OK"), "ok");
        warningBox.exec();

        setCurVGName("");

        return;
    }

    MessageBox messageBox(this);
    messageBox.setObjectName("messageBox");
    messageBox.setAccessibleName("deleteVGMessageBox");
    // 删除后数据将无法恢复，请确认  删除  取消
    messageBox.setWarings(tr("Data cannot be recovered if deleted, please confirm before proceeding"), "",
                          tr("Delete"), DDialog::ButtonWarning, "delete", tr("Cancel"), "cancel");
    if (messageBox.exec() == DDialog::Accepted) {
        QStringList vgNameList;
        vgNameList << vgInfo.m_vgName;

        DMDbusHandler::instance()->deleteVG(vgNameList);
    }

    setCurVGName("");
}

void DeviceListWidget::onCreateLVClicked()
{
    setCurVGName(DMDbusHandler::instance()->getCurVGInfo().m_vgName);

    PartitionDialog dlg(this);
    dlg.setTitleText(tr("Create logical volume"), tr("The disks will be formatted if you create a logical volume"));
    dlg.setObjectName("createLVDialog");
    dlg.setAccessibleName("createLVDialog");

    if (dlg.exec() == 1) {
        CreateLVWidget createLVWidget(this);
        createLVWidget.setObjectName("createLVWidget");
        createLVWidget.setAccessibleName("createLVWidget");
        createLVWidget.exec();
    }

    setCurVGName("");
}

void DeviceListWidget::onDeleteLVClicked()
{
    LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();
    setCurVGName(lvInfo.m_vgName);

    if (DMDbusHandler::instance()->lvIsMount(lvInfo)) {
        MessageBox warningBox(this);
        warningBox.setObjectName("messageBox");
        warningBox.setAccessibleName("messageBox");
        // 请先手动卸载XXX（逻辑卷名称）  确定
        warningBox.setWarings(tr("Unmount %1 first").arg(lvInfo.m_lvName), "", tr("OK"), "ok");
        warningBox.exec();

        setCurVGName("");

        return;
    }

    MessageBox messageBox(this);
    messageBox.setObjectName("messageBox");
    messageBox.setAccessibleName("deleteLVMessageBox");
    // 删除后数据将无法恢复，请确认  删除  取消
    messageBox.setWarings(tr("Data cannot be recovered if deleted, please confirm before proceeding"), "",
                          tr("Delete"), DDialog::ButtonWarning, "delete", tr("Cancel"), "cancel");
    if (messageBox.exec() == DDialog::Accepted) {
        QStringList lvNameList;
        lvNameList << lvInfo.m_lvPath;

        DMDbusHandler::instance()->deleteLV(lvNameList);
    }

    setCurVGName("");
}

void DeviceListWidget::onVGDeleteMessage(const QString &vgMessage)
{
    QStringList infoList = vgMessage.split(":");

    if (infoList.count() <= 1) {
        return;
    }

    if ("1" == infoList.at(0)) {
        isDeleteVGSuccess = true;
        return;
    }

    QString reason = "";
    switch (infoList.at(1).toInt()) {
    case LVMError::LVM_ERR_VG_IN_USED: { // VG被占用
        // 逻辑卷组被占用，无法删除。请重启电脑后重试。
        reason = tr("The logical volume group is busy and cannot be deleted. Please retry after reboot.");
        break;
    }
    case LVMError::LVM_ERR_LV_IN_USED: { // VG下的LV被占用
        // 逻辑卷被占用，无法删除。请重启电脑后重试。
        reason = tr("The logical volume is busy and cannot be deleted. Please retry after reboot.");
        break;
    }
    case LVMError::LVM_ERR_VG_DELETE_FAILED: {
        // 删除逻辑卷组失败
        reason = tr("Failed to delete the logical volume group");
        break;
    }
    default:
        break;
    }

    if (!reason.isEmpty()) {
        isDeleteVGSuccess = false;
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(),
                                                 QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), reason);
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onLVDeleteMessage(const QString &lvMessage)
{
    QStringList infoList = lvMessage.split(":");

    if (infoList.count() <= 1) {
        return;
    }

    if ("1" == infoList.at(0)) {
        isDeleteLVSuccess = true;
        return;
    }

    QString reason = "";
    switch (infoList.at(1).toInt()) {
    case LVMError::LVM_ERR_LV_IN_USED: {
        // 逻辑卷被占用，无法删除。请重启电脑后重试。
        reason = tr("The logical volume is busy and cannot be deleted. Please retry after reboot.");
        break;
    }
    case LVMError::LVM_ERR_LV_DELETE_FAILED: {
        // 删除逻辑卷失败
        reason = tr("Failed to delete the logical volume");
        break;
    }
    default:
        break;
    }

    if (!reason.isEmpty()) {
        isDeleteLVSuccess = false;
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(),
                                                 QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), reason);
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onCreateFailedMessage(const QString &message)
{
    QStringList infoList = message.split(":");

    if (infoList.count() <= 2) {
        return;
    }

    QString failedReason = DMDbusHandler::instance()->getFailedMessage(infoList.at(0), infoList.at(1).toInt(), infoList.at(2));
    if (!failedReason.isEmpty()) {
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(),
                                                 QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), failedReason);
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget(), QMargins(0, 0, 0, 20));
    }
}

void DeviceListWidget::onUpdateUsb()
{
    if (m_curChooseDevicePath.isEmpty() && m_curChooseVGName.isEmpty())
        return;

    if (!m_curChooseDevicePath.isEmpty()) {
        QStringList deviceNameList = DMDbusHandler::instance()->getDeviceNameList();
        if (deviceNameList.indexOf(m_curChooseDevicePath) != -1)
            return;

        QWidgetList widgetList = QApplication::topLevelWidgets();
        for (int i = 0; i < widgetList.count(); i++) {
            QWidget *widget = widgetList.at(i);
            if (widget->objectName() == "diskBadSectorsDialog") {
                DiskBadSectorsDialog *diskBadSectorsDialog = static_cast<DiskBadSectorsDialog *>(widget);
                diskBadSectorsDialog->stopCheckRepair();

                diskBadSectorsDialog->close();
            } else if (widget->objectName() == "diskInfoDisplayDialog" || widget->objectName() == "diskHealthDetectionDialog" || widget->objectName() == "partitionErrorCheck" || widget->objectName() == "createPartitionTable" || widget->objectName() == "messageBox") {
                widget->close();
            } else if (widget->objectName() == "exitMessageBox") {
                MessageBox *messageBox = static_cast<MessageBox *>(widget);
                messageBox->accept();
                break;
            }
        }

        setCurDevicePath("");
    } else if (!m_curChooseVGName.isEmpty()) {
        QStringList vgNameList = DMDbusHandler::instance()->getVGNameList();
        if (vgNameList.indexOf(m_curChooseVGName) != -1)
            return;

        QWidgetList widgetList = QApplication::topLevelWidgets();
        for (int i = 0; i < widgetList.count(); i++) {
            QWidget *widget = widgetList.at(i);
            if (widget->objectName() == "messageBox" || widget->objectName() == "createLVDialog" || widget->objectName() == "createLVWidget") {
                widget->close();
            }
        }

        setCurVGName("");
    }
}

void DeviceListWidget::setCurDevicePath(const QString &devPath)
{
    m_curChooseDevicePath = devPath;
}

void DeviceListWidget::setCurVGName(const QString &vgName)
{
    m_curChooseVGName = vgName;
}

void DeviceListWidget::onUpdateDeviceInfo()
{
    //更新DmTreeview  lx
    //设置当前选项

    auto handler = DMDbusHandler::instance();
    m_num = handler->getCurPartititonInfo().m_partitionNumber;
    m_deviceNum = m_treeView->getCurrentTopNum();
    m_devicePath = handler->getCurPartititonInfo().m_devicePath;
    m_treeView->m_model->clear();
    m_treeView->clearData();
    DeviceInfoMap infoMap = DMDbusHandler::instance()->probDeviceInfo();
    QMap<int, int> countMap;
    int deviceCount = 0;

    LVMInfo lvmInfo = DMDbusHandler::instance()->probLVMInfo();
    QMap<QString, VGInfo> mapVGInfo = lvmInfo.m_vgInfo;
    QMap<int, int> countVGMap;
    int vgCount = 0;

    if (0 != mapVGInfo.count()) {
        auto lvmInfoBox = new DmDiskinfoBox(DMDbusHandler::OTHER, this, tr("Volume Groups"), "");
        for (auto vgInfo = mapVGInfo.begin(); vgInfo != mapVGInfo.end(); vgInfo++) {
            VGInfo vgInformation = vgInfo.value();

            if (vgInformation.m_vgName.isEmpty()) {
                continue;
            }
            qDebug() << vgInformation.m_vgName << Utils::LVMFormatSize(vgInformation.m_peCount * vgInformation.m_PESize)
                     << vgInformation.m_peCount << vgInformation.m_PESize
                     << Utils::LVMFormatSize(vgInformation.m_peCount * vgInformation.m_PESize + vgInformation.m_PESize) << vgInformation.m_vgSize;
            QString vgSize = vgInformation.m_vgSize;
            if (vgSize.contains("1024")) {
                vgSize = Utils::LVMFormatSize(vgInformation.m_peCount * vgInformation.m_PESize + vgInformation.m_PESize);
            }
            auto vgInfoBox = new DmDiskinfoBox(DMDbusHandler::VOLUMEGROUP, this, vgInformation.m_vgName, vgSize,
                                               0, vgInformation.m_luksFlag, vgInformation.m_vgName);
            int lvCount = 0;

            for (auto lvInfo = vgInformation.m_lvlist.begin(); lvInfo != vgInformation.m_lvlist.end(); lvInfo++) {
                QString used = Utils::LVMFormatSize(lvInfo->m_fsUsed);
                QString unused = Utils::LVMFormatSize(lvInfo->m_fsUnused);
                if (lvInfo->m_lvName.isEmpty() && lvInfo->m_lvUuid.isEmpty()) {
                    unused = Utils::LVMFormatSize(lvInfo->m_lvLECount * lvInfo->m_LESize);
                    lvInfo->m_lvName = "unallocated";
                }

                FSType fstype = static_cast<FSType>(lvInfo->m_lvFsType);
                QString fstypeName = Utils::fileSystemTypeToString(fstype);

                QString mountPoints;
                for (int i = 0; i < lvInfo->m_mountPoints.size(); i++) {
                    mountPoints += lvInfo->m_mountPoints[i];
                }
                qDebug() << lvInfo->m_lvName << Utils::LVMFormatSize(lvInfo->m_lvLECount * lvInfo->m_LESize)
                         << Utils::LVMFormatSize(lvInfo->m_lvLECount * lvInfo->m_LESize + lvInfo->m_LESize)
                         << lvInfo->m_lvLECount << lvInfo->m_LESize << lvInfo->m_lvSize;
                QString lvSize = lvInfo->m_lvSize;
                if (lvSize.contains("1024")) {
                    lvSize = Utils::LVMFormatSize(lvInfo->m_lvLECount * lvInfo->m_LESize + lvInfo->m_LESize);
                }
                auto lvInfoBox = new DmDiskinfoBox(DMDbusHandler::LOGICALVOLUME, this, lvInfo->m_vgName, "", 0, lvInfo->m_luksFlag,
                                                   lvInfo->m_lvPath, lvSize, used, unused, 0, 0, 0, fstypeName,
                                                   mountPoints, lvInfo->m_lvName, 0);

                vgInfoBox->m_childs.append(lvInfoBox);

                lvCount++;
            }
            lvmInfoBox->m_childs.append(vgInfoBox);

            countVGMap[vgCount] = lvCount;
            vgCount++;
        }

        auto infoBox = new DmDiskinfoBox(DMDbusHandler::OTHER, this, tr("Disks"), "");
        for (auto devInfo = infoMap.begin(); devInfo != infoMap.end(); devInfo++) {
            DeviceInfo info = devInfo.value();

            if (info.m_path.isEmpty() || info.m_path.contains("/dev/mapper")) {
                continue;
            }

            QString diskSize = Utils::formatSize(info.m_length, info.m_sectorSize);
            auto diskinfoBox = new DmDiskinfoBox(DMDbusHandler::DISK, this, info.m_path, diskSize,
                                                 info.m_vgFlag, info.m_luksFlag, info.m_path);
            int partitionCount = 0;

            for (auto it = info.m_partition.begin(); it != info.m_partition.end(); it++) {
                QString partitionSize = Utils::formatSize(it->m_sectorEnd - it->m_sectorStart + 1, it->m_sectorSize);
                QString partitionPath = it->m_path;
                if ("unallocated" != partitionPath) {
                    partitionPath = partitionPath.remove(0, 5);
                }

                QString used = Utils::formatSize(it->m_sectorsUsed, it->m_sectorSize);
                QString unused = Utils::formatSize(it->m_sectorsUnused, it->m_sectorSize);

                FSType fstype = static_cast<FSType>(it->m_fileSystemType);
                QString fstypeName = Utils::fileSystemTypeToString(fstype);
                QString mountpoints;

                for (int i = 0; i < it->m_mountPoints.size(); i++) {
                    mountpoints += it->m_mountPoints[i];
                }

                QString fileSystemLabel = it->m_fileSystemLabel;
                auto childDiskinfoBox = new DmDiskinfoBox(DMDbusHandler::PARTITION, this, it->m_devicePath, "", it->m_vgFlag, it->m_luksFlag,
                                                          partitionPath, partitionSize, used, unused,
                                                          it->m_sectorsUnallocated, it->m_sectorStart, it->m_sectorEnd, fstypeName,
                                                          mountpoints, fileSystemLabel, it->m_flag);
                diskinfoBox->m_childs.append(childDiskinfoBox);

                partitionCount++;
            }
            infoBox->m_childs.append(diskinfoBox);

            countMap[deviceCount] = partitionCount;
            deviceCount++;
        }
        m_treeView->addTopItem(lvmInfoBox, m_addItem);
        m_treeView->addTopItem(infoBox, m_addItem);

        QStringList deviceNameList = DMDbusHandler::instance()->getDeviceNameList();
        if (DMDbusHandler::DISK == DMDbusHandler::instance()->getCurLevel() || DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
            if (deviceNameList.indexOf(DMDbusHandler::instance()->getCurDevicePath()) == -1) {
                m_flag = 0;
            } else {
                m_deviceNum = deviceNameList.indexOf(DMDbusHandler::instance()->getCurDevicePath());
            }
        } else {
            QStringList vgNameList = DMDbusHandler::instance()->getVGNameList();
            if (vgNameList.indexOf(DMDbusHandler::instance()->getCurVGName()) == -1) {
                m_flag = 0;
            } else {
                m_deviceNum = vgNameList.indexOf(DMDbusHandler::instance()->getCurVGName());
            }
        }

        m_addItem = 1;

        if (m_flag == 0) {
            for (int i = 0; i < deviceNameList.count(); i++) {
                if (countMap.value(i) != 0) {
                    m_treeView->setRefreshItem(i, 0, 1);
                    m_vgIsShow = true;
                    break;
                }
            }
        } else {
            int index = m_treeView->getCurrentNum();
            if (-1 != m_treeView->getCurrentGroupNum()) {
                if (DMDbusHandler::DISK == DMDbusHandler::instance()->getCurLevel() || DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
                    if (countMap.value(m_deviceNum) == index) {
                        index = m_treeView->getCurrentNum() - 1;
                    }

                    if (countMap.value(m_deviceNum) < index) {
                        index = countMap.value(m_deviceNum) - 1;
                    }
                } else {
                    if (countVGMap.value(m_deviceNum) == index) {
                        index = m_treeView->getCurrentNum() - 1;
                    }

                    if (countVGMap.value(m_deviceNum) < index) {
                        index = countVGMap.value(m_deviceNum) - 1;
                    }
                }
            }

            int groupNum = m_treeView->getCurrentGroupNum();
            int topNum = m_treeView->getCurrentTopNum();
            // 判断是否第一次显示VG
            if (!m_vgIsShow) {
                groupNum = 1; // 设置组节点为1

                // 判断VG显示之前是否选择的是磁盘节点
                if (topNum == -1) {
                    groupNum = -1; // 若之前选择的是磁盘节点，设置组节点为-1
                    topNum = 1; // 若之前选择的是磁盘节点，设置根节点为1
                }
                m_vgIsShow = true;
            }

            // 当前选择的是逻辑卷时，判断父节点位置是否发生改变；当前是逻辑卷组时，判断位置是否发生改变
            // 针对于逻辑盘插拔时，没有默认选中节点做的处理
            if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
                topNum = topNum == m_deviceNum ? topNum : m_deviceNum;
            } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::VOLUMEGROUP) {
                index = index == m_deviceNum ? index : m_deviceNum;
            }

            m_treeView->setRefreshItem(topNum, index, groupNum);
        }

        m_flag += 1;
    } else {
        for (auto devInfo = infoMap.begin(); devInfo != infoMap.end(); devInfo++) {
            DeviceInfo info = devInfo.value();

            if (info.m_path.isEmpty() || info.m_path.contains("/dev/mapper")) {
                continue;
            }

            QString diskSize = Utils::formatSize(info.m_length, info.m_sectorSize);
            auto diskinfoBox = new DmDiskinfoBox(DMDbusHandler::DISK, this, info.m_path, diskSize,
                                                 info.m_vgFlag, info.m_luksFlag, info.m_path);
            int partitionCount = 0;

            for (auto it = info.m_partition.begin(); it != info.m_partition.end(); it++) {
                QString partitionSize = Utils::formatSize(it->m_sectorEnd - it->m_sectorStart + 1, it->m_sectorSize);
                QString partitionPath = it->m_path;
                if ("unallocated" != partitionPath) {
                    partitionPath = partitionPath.remove(0, 5);
                }

                QString used = Utils::formatSize(it->m_sectorsUsed, it->m_sectorSize);
                QString unused = Utils::formatSize(it->m_sectorsUnused, it->m_sectorSize);

                FSType fstype = static_cast<FSType>(it->m_fileSystemType);
                QString fstypeName = Utils::fileSystemTypeToString(fstype);
                QString mountpoints;

                for (int i = 0; i < it->m_mountPoints.size(); i++) {
                    mountpoints += it->m_mountPoints[i];
                }

                QString fileSystemLabel = it->m_fileSystemLabel;
                auto childDiskinfoBox = new DmDiskinfoBox(DMDbusHandler::PARTITION, this, it->m_devicePath, "", it->m_vgFlag, it->m_luksFlag,
                                                          partitionPath, partitionSize, used, unused,
                                                          it->m_sectorsUnallocated, it->m_sectorStart, it->m_sectorEnd, fstypeName,
                                                          mountpoints, fileSystemLabel, it->m_flag);
                diskinfoBox->m_childs.append(childDiskinfoBox);

                partitionCount++;
            }

            m_treeView->addTopItem(diskinfoBox, m_addItem);

            countMap[deviceCount] = partitionCount;
            deviceCount++;
        }

        QStringList deviceNameList = DMDbusHandler::instance()->getDeviceNameList();

        if (deviceNameList.indexOf(DMDbusHandler::instance()->getCurDevicePath()) == -1) {
            m_flag = 0;
        } else {
            m_deviceNum = deviceNameList.indexOf(DMDbusHandler::instance()->getCurDevicePath());
        }

        m_addItem = 1;

        // 删除全部VG后，重置标志位
        if (m_vgIsShow && DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::VOLUMEGROUP) {
            m_flag = 0;
        }

        if (m_flag == 0) {
            for (int i = 0; i < deviceNameList.count(); i++) {
                if (countMap.value(i) != 0) {
                    m_treeView->setRefreshItem(i, 0);
                    break;
                }
            }
            //        m_treeView->setDefaultdmItem();
        } else {
            int index = m_treeView->getCurrentNum();
            if (countMap.value(m_deviceNum) == index) {
                index = m_treeView->getCurrentNum() - 1;
            }

            if (countMap.value(m_deviceNum) < index) {
                index = countMap.value(m_deviceNum) - 1;
            }

            int topNum = m_treeView->getCurrentTopNum();
            // 判断是否显示过VG并且当前选择的节点是磁盘节点
            if (m_vgIsShow && m_treeView->getCurrentGroupNum() == -1) {
                topNum = -1;
            }

            if (topNum == -1) {
                if (deviceNameList.indexOf(DMDbusHandler::instance()->getCurDevicePath()) == -1) {
                    m_treeView->setDefaultdmItem();
                } else {
                    m_treeView->setRefreshItem(topNum, deviceNameList.indexOf(DMDbusHandler::instance()->getCurDevicePath()));
                }

            } else {
                m_treeView->setRefreshItem(m_deviceNum, index);
            }
        }

        m_vgIsShow = false;

        m_flag += 1;
    }
}
