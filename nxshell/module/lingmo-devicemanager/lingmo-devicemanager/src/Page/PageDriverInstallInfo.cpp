// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QBoxLayout>

#include <DScrollArea>

#include "PageDriverInstallInfo.h"
#include "DetectedStatusWidget.h"
#include "PageDriverTableView.h"
#include "DeviceManager.h"

PageDriverInstallInfo::PageDriverInstallInfo(QWidget *parent)
    : DFrame(parent)
    , mp_HeadWidget(new DetectedStatusWidget(this))
    , mp_ViewNotInstall(new PageDriverTableView(this))
    , mp_ViewCanUpdate(new PageDriverTableView(this))
    , mp_AllDriverIsNew(new PageDriverTableView(this))
    , mp_InstallLabel(new DLabel(this))
    , mp_UpdateLabel(new DLabel(this))
    , mp_LabelIsNew(new DLabel(this))
    , mp_InstallWidget(new DWidget(this))
    , mp_UpdateWidget(new DWidget(this))
{
    initUI();

    connect(mp_ViewNotInstall, &PageDriverTableView::operatorClicked, this, &PageDriverInstallInfo::operatorClicked);
    connect(mp_ViewCanUpdate, &PageDriverTableView::operatorClicked, this, &PageDriverInstallInfo::operatorClicked);
    connect(mp_ViewNotInstall, &PageDriverTableView::itemChecked, this, &PageDriverInstallInfo::itemChecked);
    connect(mp_ViewCanUpdate, &PageDriverTableView::itemChecked, this, &PageDriverInstallInfo::itemChecked);
    connect(mp_HeadWidget, &DetectedStatusWidget::installAll, this, &PageDriverInstallInfo::installAll);
    connect(mp_HeadWidget, &DetectedStatusWidget::installAll, this, [=](){
        // 安装过程中，所有已经选中的勾选框置灰
        mp_ViewNotInstall->setCheckedCBDisnable();
        mp_ViewCanUpdate->setCheckedCBDisnable();
    });
    connect(mp_HeadWidget, &DetectedStatusWidget::cancelClicked, this, &PageDriverInstallInfo::undoInstall);
    connect(mp_HeadWidget, &DetectedStatusWidget::redetected, this, &PageDriverInstallInfo::redetected);
}

void PageDriverInstallInfo::initUI()
{
    this->setLineWidth(0);
    initTable();

    QVBoxLayout *vLaout = new QVBoxLayout(this);
    vLaout->setContentsMargins(20, 20, 20, 0);
    vLaout->setSpacing(0);

    // 上方的表头
    QHBoxLayout *headerLayout = new QHBoxLayout(this);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(mp_HeadWidget);

    // 下方的可滑动区域
    DScrollArea *area = new DScrollArea(this);
    area->setMinimumHeight(10);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    DWidget *frame = new DWidget(this);
    frame->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *frameLayout = new QVBoxLayout();
    frameLayout->setContentsMargins(0, 0, 0, 0);

    // 将Label，tabel spacing都放在widget中作为一个整体进行展示
    QVBoxLayout *installLayout = new QVBoxLayout();
    installLayout->setContentsMargins(0, 0, 0, 0);
    installLayout->addWidget(mp_InstallLabel);
    installLayout->addWidget(mp_ViewNotInstall);
    installLayout->addSpacing(7);
    mp_InstallWidget->setLayout(installLayout);
    frameLayout->addWidget(mp_InstallWidget);

    QVBoxLayout *updateLayout = new QVBoxLayout();
    updateLayout->setContentsMargins(0, 0, 0, 0);
    updateLayout->addWidget(mp_UpdateLabel);
    updateLayout->addWidget(mp_ViewCanUpdate);
    updateLayout->addSpacing(7);
    mp_UpdateWidget->setLayout(updateLayout);
    frameLayout->addWidget(mp_UpdateWidget);

    frameLayout->addWidget(mp_LabelIsNew);
    frameLayout->addWidget(mp_AllDriverIsNew);

    frameLayout->addSpacing(17);
    frameLayout->addStretch();

    frame->setLayout(frameLayout);
    area->setWidget(frame);

    vLaout->addLayout(headerLayout);
    vLaout->addSpacing(16);
    vLaout->addWidget(area);
    this->setLayout(vLaout);
}

void PageDriverInstallInfo::initTable()
{
    // 设置列宽
    mp_ViewNotInstall->initHeaderView(QStringList() << ""
                                      << tr("Device Name")
                                      << tr("Version Available")
                                      << tr("Size")
                                      << tr("Status")
                                      << tr("Action"), true);
    mp_ViewNotInstall->setColumnWidth(0, 41);
    mp_ViewNotInstall->setColumnWidth(1, 278);
    mp_ViewNotInstall->setColumnWidth(2, 150);
    mp_ViewNotInstall->setColumnWidth(3, 100);
    mp_ViewNotInstall->setColumnWidth(4, 140);
    mp_ViewNotInstall->setColumnWidth(5, 60);

    mp_ViewCanUpdate->initHeaderView(QStringList() << ""
                                     << tr("Device Name")
                                     << tr("New Version")
                                     << tr("Size")
                                     << tr("Status")
                                     << tr("Action"), true);
    mp_ViewCanUpdate->setHeaderCbStatus(false);
    mp_ViewCanUpdate->setColumnWidth(0, 41);
    mp_ViewCanUpdate->setColumnWidth(1, 278);
    mp_ViewCanUpdate->setColumnWidth(2, 150);
    mp_ViewCanUpdate->setColumnWidth(3, 100);
    mp_ViewCanUpdate->setColumnWidth(4, 140);
    mp_ViewCanUpdate->setColumnWidth(5, 60);

    mp_AllDriverIsNew->initHeaderView(QStringList() << tr("Device Name") << tr("Current Version"));
    mp_AllDriverIsNew->setColumnWidth(0, 418);
}

void PageDriverInstallInfo::addDriverInfoToTableView(DriverInfo *info, int index)
{
    PageDriverTableView *view = nullptr;
    if (ST_NOT_INSTALL == info->status()) {
        view = mp_ViewNotInstall;
        view->appendRowItems(6);
    } else if (ST_CAN_UPDATE == info->status()) {
        view = mp_ViewCanUpdate;
        view->appendRowItems(6);
    } else if (ST_DRIVER_IS_NEW == info->status()) {
        view = mp_AllDriverIsNew;
        view->appendRowItems(2);
    } else {
        return;
    }

    int row = view->model()->rowCount() - 1;

    if (view != mp_AllDriverIsNew) {

        // 设置CheckBtn
        DriverCheckItem *cbItem = new DriverCheckItem(this);
        connect(cbItem, &DriverCheckItem::sigChecked, view, [index, view](bool checked) {
            Q_UNUSED(index)
            view->setHeaderCbStatus(checked);
        });
        cbItem->setChecked(ST_NOT_INSTALL == info->status());
        view->setWidget(row, 0, cbItem);

        // 设置设备信息
        DriverNameItem *nameItem = new DriverNameItem(this, info->type());
        nameItem->setName(info->name());
        nameItem->setIndex(index);
        view->setWidget(row, 1, nameItem);

        // 设置版本
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->debVersion());
        view->setWidget(row, 2, versionItem);

        // 设置大小
        DriverLabelItem *sizeItem = new DriverLabelItem(this, info->size());
        view->setWidget(row, 3, sizeItem);

        // 设置状态
        DriverStatusItem *statusItem = new DriverStatusItem(this, info->status());
        view->setWidget(row, 4, statusItem);

        // 添加操作按钮
        DriverOperationItem *operateItem = new DriverOperationItem(this, ST_NOT_INSTALL == info->status() ? DriverOperationItem::INSTALL : DriverOperationItem::UPDATE);
        view->setWidget(row, 5, operateItem);
    } else {
        // 设置设备信息
        DriverNameItem *nameItem = new DriverNameItem(this, info->type());
        nameItem->setName(info->name());
        nameItem->setIndex(index);
        view->setWidget(row, 0, nameItem);

        // 设置版本
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
        view->setWidget(row, 1, versionItem);
    }
}

void PageDriverInstallInfo::addCurDriverInfo(DriverInfo *info)
{
    mp_AllDriverIsNew->appendRowItems(2);

    DriverNameItem *nameItem = new DriverNameItem(this, info->type());
    int row = mp_AllDriverIsNew->model()->rowCount() - 1;

    nameItem->setName(info->name());
    mp_AllDriverIsNew->setWidget(row, 0, nameItem);

    DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
    mp_AllDriverIsNew->setWidget(row, 1, versionItem);
}

void PageDriverInstallInfo::showTables(int installLength, int updateLength, int newLength)
{
    // Label显示
    mp_InstallLabel->setText(tr("Missing drivers (%1)").arg(installLength));
    mp_UpdateLabel->setText(tr("Outdated drivers (%1)").arg(updateLength));
    mp_LabelIsNew->setText(tr("Up-to-date drivers (%1)").arg(newLength));

    // 无需更新驱动列表是否显示
    mp_LabelIsNew->setVisible(newLength != 0);
    mp_AllDriverIsNew->setVisible(newLength != 0);

    // 可安装和可更新表格分别与Label作为整体进行隐藏
    mp_InstallWidget->setVisible(installLength != 0);
    mp_UpdateWidget->setVisible(updateLength != 0);

    // 显示表头显示的内容
    const QMap<QString, QString> &overviewMap = DeviceManager::instance()->getDeviceOverview();
    if (installLength == 0 && updateLength == 0) {
        mp_HeadWidget->setNoUpdateDriverUI(overviewMap["Overview"]);
    } else {
        mp_HeadWidget->setDetectFinishUI(QString::number(installLength + updateLength), overviewMap["Overview"], installLength != 0);
    }
}

void PageDriverInstallInfo::getCheckedDriverIndex(QList<int> &lstIndex)
{
    mp_ViewNotInstall->getCheckedDriverIndex(lstIndex);
    mp_ViewCanUpdate->getCheckedDriverIndex(lstIndex);
}

void PageDriverInstallInfo::clearAllData()
{
    mp_ViewCanUpdate->clear();
    mp_ViewNotInstall->clear();
    mp_AllDriverIsNew->clear();

    initTable();
}

void PageDriverInstallInfo::updateItemStatus(int index, Status status, QString errS)
{
    mp_ViewCanUpdate->setItemStatus(index, status);
    mp_ViewNotInstall->setItemStatus(index, status);

    mp_ViewCanUpdate->setErrorMsg(index, errS);
    mp_ViewNotInstall->setErrorMsg(index, errS);
}

void PageDriverInstallInfo::setCheckedCBDisnable()
{
    mp_ViewCanUpdate->setCheckedCBDisnable();
    mp_ViewCanUpdate->setCheckedCBDisnable();
}

void PageDriverInstallInfo::setHeaderCbEnable(bool enable)
{
    mp_ViewNotInstall->setHeaderCbEnable(enable);
    mp_ViewCanUpdate->setHeaderCbEnable(enable);
}
void PageDriverInstallInfo::slotDownloadProgressChanged(DriverType type, QString size, QStringList msg)
{
    // 将下载过程时时更新到表格上方的状态里面 qCInfo(appLog) << "Download ********** " << msg[0] << " , " << msg[1] << " , " << msg[2];
    mp_HeadWidget->setDownloadUI(type, msg[2], msg[1], size, msg[0].toInt());
}

void PageDriverInstallInfo::slotDownloadFinished(int index, Status status)
{
    mp_ViewCanUpdate->setItemStatus(index, status);
    mp_ViewNotInstall->setItemStatus(index, status);
}
