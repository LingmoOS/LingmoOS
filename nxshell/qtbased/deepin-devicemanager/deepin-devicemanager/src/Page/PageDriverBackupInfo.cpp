// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QBoxLayout>

#include <DScrollArea>

#include "PageDriverBackupInfo.h"
#include "DetectedStatusWidget.h"
#include "PageDriverTableView.h"
#include "driveritem.h"

PageDriverBackupInfo::PageDriverBackupInfo(QWidget *parent)
    : DFrame(parent)
    , mp_HeadWidget(new DetectedStatusWidget(this))
    , mp_NotBackupWidget(new DWidget(this))
    , mp_ViewBackable(new PageDriverTableView(this))
    , mp_BackableDriverLabel(new DLabel(this))
    , mp_BackedUpWidget(new DWidget(this))
    , mp_ViewBackedUp(new PageDriverTableView(this))
    , mp_BackedUpDriverLabel(new DLabel(this))
{
    initUI();

    connect(mp_ViewBackable, &PageDriverTableView::operatorClicked, this, &PageDriverBackupInfo::operatorClicked);
    connect(mp_ViewBackable, &PageDriverTableView::itemChecked, this, &PageDriverBackupInfo::itemChecked);
    connect(mp_HeadWidget, &DetectedStatusWidget::backupAll, this, &PageDriverBackupInfo::backupAll);
    connect(mp_HeadWidget, &DetectedStatusWidget::backupAll, this, [=](){
        mp_ViewBackable->setCheckedCBDisnable();
    });
    connect(mp_HeadWidget, &DetectedStatusWidget::cancelClicked, this, &PageDriverBackupInfo::undoBackup);
    connect(mp_HeadWidget, &DetectedStatusWidget::redetected, this, &PageDriverBackupInfo::redetected);
}

void PageDriverBackupInfo::initUI()
{
    this->setLineWidth(0);
    initTable();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 0);
    mainLayout->setSpacing(0);

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
    QVBoxLayout *frameLayout = new QVBoxLayout();
    frame->setContentsMargins(0, 0, 0, 0);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *backableLayout = new QVBoxLayout(this);
    backableLayout->setContentsMargins(0, 0, 0, 0);
    backableLayout->addWidget(mp_BackableDriverLabel);
    backableLayout->addWidget(mp_ViewBackable);
    backableLayout->addSpacing(7);
    backableLayout->addStretch();
    mp_NotBackupWidget->setLayout(backableLayout);

    QVBoxLayout *updateLayout = new QVBoxLayout(this);
    updateLayout->setContentsMargins(0, 0, 0, 0);
    updateLayout->addWidget(mp_BackedUpDriverLabel);
    updateLayout->addWidget(mp_ViewBackedUp);
    updateLayout->addSpacing(7);
    updateLayout->addStretch();
    mp_BackedUpWidget->setLayout(updateLayout);

    frameLayout->addWidget(mp_NotBackupWidget);
    frameLayout->addWidget(mp_BackedUpWidget);
    frameLayout->addSpacing(17);
    frameLayout->addStretch();
    frame->setLayout(frameLayout);
    area->setWidget(frame);

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(16);
    mainLayout->addWidget(area);
    this->setLayout(mainLayout);
}

void PageDriverBackupInfo::initTable()
{
    mp_ViewBackable->initHeaderView(QStringList() << "" << tr("Name")
                                    << tr("Current Version") << tr("Driver Platform Version")
                                    << tr("Status") << tr("Action"), true);
    mp_ViewBackable->setHeaderCbStatus(false);
    mp_ViewBackable->setColumnWidth(0, 41);
    mp_ViewBackable->setColumnWidth(1, 290);
    mp_ViewBackable->setColumnWidth(2, 120);
    mp_ViewBackable->setColumnWidth(3, 120);
//    mp_ViewBackable->setColumnWidth(4, 80);
    mp_ViewBackable->setColumnWidth(4, 120);
    mp_ViewBackable->setColumnWidth(5, 80);

    mp_ViewBackedUp->initHeaderView(QStringList() << tr("Name") << tr("Current Version") << tr("Driver Platform Version"));
    mp_ViewBackedUp->setColumnWidth(0, 418);
    mp_ViewBackedUp->setColumnWidth(1, 185);
}

void PageDriverBackupInfo::addDriverInfoToTableView(DriverInfo *info, int index)
{
    PageDriverTableView *view = nullptr;
    if (info->debBackupVersion() != info->debVersion()) {
        view = mp_ViewBackable;
        view->appendRowItems(6);

        int row = view->model()->rowCount() - 1;

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
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
        view->setWidget(row, 2, versionItem);

        // 设置版本
        DriverLabelItem *debVersionItem = new DriverLabelItem(this, info->debVersion());
        view->setWidget(row, 3, debVersionItem);

//        // 设置大小
//        DriverLabelItem *sizeItem = new DriverLabelItem(this, info->size());
//        view->setWidget(row, 4, sizeItem);

        // 设置状态
        DriverStatusItem *statusItem = new DriverStatusItem(this, Status::ST_DRIVER_NOT_BACKUP);
        view->setWidget(row, 4, statusItem);

        // 添加操作按钮
        DriverOperationItem *operateItem = new DriverOperationItem(this,  DriverOperationItem::BACKUP);
        view->setWidget(row, 5, operateItem);
    }

    if (!info->debBackupVersion().isEmpty()) {
        view = mp_ViewBackedUp;
        view->appendRowItems(3);

        int row = view->model()->rowCount() - 1;

        // 设置设备信息
        DriverNameItem *nameItem = new DriverNameItem(this, info->type());
        nameItem->setName(info->name());
        nameItem->setIndex(index);
        view->setWidget(row, 0, nameItem);

        // 设置版本
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
        view->setWidget(row, 1, versionItem);

        // 设置版本
        DriverLabelItem *debVersionItem = new DriverLabelItem(this, info->debVersion());
        view->setWidget(row, 2, debVersionItem);
    }
}

void PageDriverBackupInfo::showTables(int backableLength, int backedupLength)
{
    // Label显示
    mp_BackableDriverLabel->setText(tr("Backupable Drivers"));
    mp_BackedUpDriverLabel->setText(tr("Backed up Drivers"));

    mp_BackedUpDriverLabel->setVisible(backedupLength != 0);
    mp_BackedUpWidget->setVisible(backedupLength != 0);

    mp_BackableDriverLabel->setVisible(backableLength != 0);
    mp_NotBackupWidget->setVisible(backableLength != 0);

    // 显示表头显示的内容
    if (backableLength == 0) {
        mp_HeadWidget->setNoBackupDriverUI(backableLength, backedupLength);
    } else {
        mp_HeadWidget->setBackableDriverUI(backableLength, backedupLength);
    }
}

void PageDriverBackupInfo::getCheckedDriverIndex(QList<int> &lstIndex)
{
    mp_ViewBackable->getCheckedDriverIndex(lstIndex);
}

void PageDriverBackupInfo::clearAllData()
{
    mp_ViewBackable->clear();
    mp_ViewBackedUp->clear();

    initTable();
}

void PageDriverBackupInfo::updateItemStatus(int index, Status status)
{
    mp_ViewBackable->setItemStatus(index, status);
    if (status == ST_DRIVER_BACKUP_FAILED)
        mp_ViewBackable->setErrorMsg(index, QString("Backup Failed"));
}

void PageDriverBackupInfo::setCheckedCBDisnable()
{
    mp_ViewBackable->setCheckedCBDisnable();
}

void PageDriverBackupInfo::setHeaderCbEnable(bool enable)
{
    mp_ViewBackable->setHeaderCbEnable(enable);
}
