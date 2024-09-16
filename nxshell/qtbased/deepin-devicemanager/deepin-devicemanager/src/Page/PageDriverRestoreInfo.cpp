// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QBoxLayout>

#include <DScrollArea>
#include <DStackedWidget>

#include "PageDriverRestoreInfo.h"
#include "DetectedStatusWidget.h"
#include "PageDriverTableView.h"
#include "driveritem.h"

PageDriverRestoreInfo::PageDriverRestoreInfo(QWidget *parent)
    : DFrame(parent)
    , mp_StackWidget(new DStackedWidget(this))
    , mp_HeadWidget(new DetectedStatusWidget(this))
    , mp_NotBackupWidget(new DWidget(this))
    , mp_ViewBackable(new PageDriverTableView(this))
    , mp_BackableDriverLabel(new DLabel(this))
    , mp_NoRestoreDriverFrame(new DFrame(this))
    , mp_GotoBackupSgButton(new DSuggestButton(this))
{
    initUI();

    connect(mp_ViewBackable, &PageDriverTableView::operatorClicked, this, &PageDriverRestoreInfo::operatorClicked);
    connect(mp_ViewBackable, &PageDriverTableView::operatorClicked, this, &PageDriverRestoreInfo::slotOperatorClicked);
    connect(mp_GotoBackupSgButton, &DSuggestButton::clicked, this, &PageDriverRestoreInfo::gotoBackup);
    connect(mp_HeadWidget, &DetectedStatusWidget::redetected, this, &PageDriverRestoreInfo::redetected);
}
void PageDriverRestoreInfo::initUI()
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
    frame->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *frameLayout = new QVBoxLayout();
    frameLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(mp_BackableDriverLabel);
    vLayout->addWidget(mp_ViewBackable);
    vLayout->addSpacing(7);
    vLayout->addStretch();
    mp_NotBackupWidget->setLayout(vLayout);

    frameLayout->addWidget(mp_NotBackupWidget);
    frameLayout->addSpacing(17);
    frameLayout->addStretch();
    frame->setLayout(frameLayout);
    area->setWidget(frame);

    QVBoxLayout *restoreVLayout = new QVBoxLayout(this);
    restoreVLayout->setSpacing(0);
    restoreVLayout->addLayout(headerLayout);
    restoreVLayout->addSpacing(16);
    restoreVLayout->addWidget(area);

    DWidget *restoreWidget = new DWidget(this);
    restoreWidget->setLayout(restoreVLayout);


    // 没有备份文件页面
    QVBoxLayout *noRestoreMainLayout = new QVBoxLayout(this);
    noRestoreMainLayout->setAlignment(Qt::AlignHCenter);

    DLabel *picLabel = new DLabel(this);
    picLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/restore_128.svg"));
    QPixmap pix = icon.pixmap(128, 128);
    picLabel->setPixmap(pix);
    //picLabel->setFrameShape(QFrame::Box);

    DLabel *tipLabel = new DLabel(this);
    tipLabel->setText(tr("You do not have any drivers to restore, please backup first"));
    //tipLabel->setFrameShape(QFrame::Box);

    mp_GotoBackupSgButton->setText(tr("Go to Backup Driver"));
    mp_GotoBackupSgButton->setFixedWidth(310);
    mp_GotoBackupSgButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mp_GotoBackupSgButton->setFocusPolicy(Qt::NoFocus);

    noRestoreMainLayout->addStretch();
    noRestoreMainLayout->addWidget(picLabel);
    noRestoreMainLayout->addSpacing(10);
    noRestoreMainLayout->addWidget(tipLabel, 0, Qt::AlignHCenter);
    noRestoreMainLayout->addSpacing(35);
    noRestoreMainLayout->addWidget(mp_GotoBackupSgButton, 0, Qt::AlignHCenter);
    noRestoreMainLayout->addSpacing(25);
    noRestoreMainLayout->addStretch();
    mp_NoRestoreDriverFrame->setLayout(noRestoreMainLayout);
    mp_NoRestoreDriverFrame->setLineWidth(0);

    mp_StackWidget->addWidget(mp_NoRestoreDriverFrame);
    mp_StackWidget->addWidget(restoreWidget);


    mainLayout->addWidget(mp_StackWidget);
    this->setLayout(mainLayout);
}

void PageDriverRestoreInfo::initTable()
{
    mp_ViewBackable->initHeaderView(QStringList()<< tr("Name")
                                    << tr("Current Version")
                                    << tr("Backup Version")
                                    << tr("Action"), false);
    mp_ViewBackable->setHeaderCbStatus(false);
    mp_ViewBackable->setColumnWidth(0, 324);
    mp_ViewBackable->setColumnWidth(1, 158);
    mp_ViewBackable->setColumnWidth(2, 158);
}

void PageDriverRestoreInfo::addDriverInfoToTableView(DriverInfo *info, int index)
{
    PageDriverTableView *view = nullptr;
    if (!info->debBackupVersion().isEmpty()) {
        view = mp_ViewBackable;
        view->appendRowItems(4);
    } else {
        return;
    }

    int row = view->model()->rowCount() - 1;

    if (view == mp_ViewBackable) {
        // 设置名称
        DriverNameItem *nameItem = new DriverNameItem(this, info->type());
        nameItem->setName(info->name());
        nameItem->setIndex(index);
        view->setWidget(row, 0, nameItem);

        // 设置当前版本
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
        view->setWidget(row, 1, versionItem);

        // 设置备份版本
        DriverLabelItem *sizeItem = new DriverLabelItem(this, info->debBackupVersion());
        view->setWidget(row, 2, sizeItem);

        // 添加操作按钮
        DriverOperationItem *operateItem = new DriverOperationItem(this, DriverOperationItem::RESTORE);
        view->setWidget(row, 3, operateItem);
    }
}

void PageDriverRestoreInfo::showTables(int backedLength)
{
    mp_BackableDriverLabel->setText(tr("Restorable Drivers"));
    mp_HeadWidget->setRestoreDriverUI(backedLength);

    if (backedLength == 0) {
        mp_StackWidget->setCurrentIndex(0);
    } else {
        mp_StackWidget->setCurrentIndex(1);
    }
}

void PageDriverRestoreInfo::clearAllData()
{
    mp_ViewBackable->clear();

    initTable();
}

void PageDriverRestoreInfo::setItemOperationEnable(int index, bool enable)
{
    mp_ViewBackable->setItemOperationEnable(index, enable);
}

void PageDriverRestoreInfo::slotOperatorClicked(int index, int itemIndex, DriverOperationItem::Mode mode)
{
    PageDriverTableView *view = mp_ViewBackable;
    // 设置状态
    DriverStatusItem *statusItem = new DriverStatusItem(this, ST_DRIVER_RESTORING);
//    view->removeItemAndWidget(itemIndex, 3);
    view->setWidget(itemIndex, 3, statusItem);
}

