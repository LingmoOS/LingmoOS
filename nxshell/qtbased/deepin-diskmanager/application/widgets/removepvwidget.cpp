// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "removepvwidget.h"
#include "messagebox.h"

#include <DWindowCloseButton>
#include <DMessageManager>

#include <QVBoxLayout>
#include <QHBoxLayout>

RemovePVWidget::RemovePVWidget(QWidget *parent) : DDBase(parent)
{
    initUi();
    initConnection();
}

void RemovePVWidget::initUi()
{
    setIcon(QIcon::fromTheme("://icons/deepin/builtin/exception-logo.svg"));
    setFixedSize(380, 190);

    DPalette palette1;
    QColor color("#000000");
    color.setAlphaF(0.9);
    palette1.setColor(DPalette::ToolTipText, color);

    DPalette palette2;
    QColor color2("#000000");
    color2.setAlphaF(0.7);
    palette2.setColor(DPalette::ToolTipText, color2);

    DPalette palette3;
    palette3.setColor(DPalette::ButtonText, QColor("#414D68"));

    DPalette palette4;
    palette4.setColor(DPalette::TextWarning, QColor("#FF5736"));

    QFont fontTitle;
    fontTitle.setWeight(QFont::Medium);
    fontTitle.setFamily("Source Han Sans");
    fontTitle.setPixelSize(14);

    QFont fontSubTitle;
    fontSubTitle.setWeight(QFont::Normal);
    fontSubTitle.setFamily("Source Han Sans");
    fontSubTitle.setPixelSize(14);

    DLabel *titleLabel = new DLabel(tr("Are you sure you want to delete the physical volume?"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(fontTitle);
    titleLabel->setPalette(palette1);

    // 删除物理卷前，请对其内的数据做好备份，以防数据丢失
    DLabel *subTitleLabel = new DLabel(tr("To prevent data loss, back up data in the physical volume before deleting it"), this);
    subTitleLabel->setAlignment(Qt::AlignCenter);
    subTitleLabel->setFont(fontSubTitle);
    subTitleLabel->setPalette(palette2);
    subTitleLabel->adjustSize();
    subTitleLabel->setWordWrap(true);

    m_cancelButton = new DPushButton(tr("Cancel", "button"), this);
    m_cancelButton->setAccessibleName("cancel");
    m_cancelButton->setFont(fontTitle);
//    m_cancelButton->setPalette(palette3);
    m_cancelButton->setFixedSize(170, 36);

    m_deleteButton = new DWarningButton(this);
    m_deleteButton->setText(tr("Delete", "button"));
    m_deleteButton->setAccessibleName("delete");
    m_deleteButton->setFont(fontTitle);
//    m_deleteButton->setPalette(palette4);
    m_deleteButton->setFixedSize(170, 36);

    DVerticalLine *line = new DVerticalLine;
    line->setObjectName("VLine");
    line->setFixedHeight(28);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addSpacing(9);
    buttonLayout->addWidget(line, 0, Qt::AlignCenter);
    buttonLayout->addSpacing(9);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *firstLayout = new QVBoxLayout;
    firstLayout->addWidget(titleLabel);
    firstLayout->addSpacing(6);
    firstLayout->addWidget(subTitleLabel);
    firstLayout->addSpacing(20);
    firstLayout->addLayout(buttonLayout);
    firstLayout->setSpacing(0);
    firstLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *firstWidget = new QWidget(this);
    firstWidget->setLayout(firstLayout);

    DLabel *delTitleLabel = new DLabel(tr("Deleting..."), this);
    delTitleLabel->setAlignment(Qt::AlignCenter);
    delTitleLabel->setFont(fontTitle);
    delTitleLabel->setPalette(palette1);

    m_waterLoadingWidget = new WaterLoadingWidget(this);
    m_waterLoadingWidget->setFixedSize(128, 128);

    QVBoxLayout *secondLayout = new QVBoxLayout;
    secondLayout->addWidget(delTitleLabel, 0, Qt::AlignCenter);
    secondLayout->addSpacing(4);
    secondLayout->addWidget(m_waterLoadingWidget, 0, Qt::AlignCenter);
    secondLayout->addStretch();
    secondLayout->setSpacing(0);
    secondLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *secondWidget = new QWidget(this);
    secondWidget->setLayout(secondLayout);

    m_stackedWidget = new DStackedWidget(this);
    m_stackedWidget->addWidget(firstWidget);
    m_stackedWidget->addWidget(secondWidget);
    m_stackedWidget->setCurrentIndex(0);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void RemovePVWidget::initConnection()
{
    connect(m_deleteButton, &DWarningButton::clicked, this, &RemovePVWidget::onButtonClicked);
    connect(m_cancelButton, &DPushButton::clicked, this, &RemovePVWidget::onCancelButtonClicked);
    connect(DMDbusHandler::instance(), &DMDbusHandler::pvDeleteMessage, this, &RemovePVWidget::onPVDeleteMessage);
}

void RemovePVWidget::onCancelButtonClicked()
{
    close();
}

void RemovePVWidget::onButtonClicked()
{
    bool bigDataMove = false;
    QStringList realDelPvLis;
    set<QString> pvStrList;
    QList<PVData> devList;
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION) {
        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        //todo pvStrList 填入分区路径
        pvStrList.insert(info.m_path);

        PVData pvData;
        pvData.m_diskPath = info.m_devicePath;
        pvData.m_startSector = info.m_sectorStart;
        pvData.m_endSector = info.m_sectorEnd;
        pvData.m_sectorSize = static_cast<int>(info.m_sectorSize);
        pvData.m_pvAct = LVMAction::LVM_ACT_DELETEPV;
        pvData.m_devicePath = info.m_path;
        pvData.m_type = DevType::DEV_PARTITION;

        devList.append(pvData);
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        //todo 此处 判断是磁盘加入还是分区加入 如果是磁盘 pvStrList 填入磁盘路径  如果是分区 依次填入分区路径
        if (info.m_disktype == "none") {
            pvStrList.insert(info.m_path);

            PVData pvData;
            pvData.m_diskPath = info.m_path;
            pvData.m_startSector = 0;
            pvData.m_endSector = info.m_length - 1;
            pvData.m_sectorSize = static_cast<int>(info.m_sectorSize);
            pvData.m_pvAct = LVMAction::LVM_ACT_DELETEPV;
            pvData.m_devicePath = info.m_path;
            pvData.m_type = DevType::DEV_DISK;

            devList.append(pvData);
        } else {
            for (int i = 0; i < info.m_partition.size(); i++) {
                PartitionInfo partInfo = info.m_partition.at(i);
                if (partInfo.m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV) {
                    pvStrList.insert(partInfo.m_path);

                    PVData pvData;
                    pvData.m_diskPath = partInfo.m_devicePath;
                    pvData.m_startSector = partInfo.m_sectorStart;
                    pvData.m_endSector = partInfo.m_sectorEnd;
                    pvData.m_sectorSize = static_cast<int>(partInfo.m_sectorSize);
                    pvData.m_pvAct = LVMAction::LVM_ACT_DELETEPV;
                    pvData.m_devicePath = partInfo.m_path;
                    pvData.m_type = DevType::DEV_PARTITION;

                    devList.append(pvData);
                }
            }
        }
    }

    if (pvStrList.size() > 0 && Utils::adjudicationPVDelete(DMDbusHandler::instance()->probLVMInfo(), pvStrList, bigDataMove, realDelPvLis)) {
        //todo 进入 说明允许删除 补充允许删除逻辑
        if (bigDataMove) {
            MessageBox warningBox(this);
            warningBox.setObjectName("messageBox");
            warningBox.setAccessibleName("removeWarningWidget");
            // XXX（磁盘名称）中存在大量数据，预计将耗费较长时间进行数据迁移，是否继续？  继续
            QString text1 = tr("A lot of data exists on %1, ").arg(realDelPvLis.join(','));
            QString text2 = tr("which may take a long time to back it up.");
            QString text3 = tr("Do you want to continue?");
            warningBox.setWarings(text1 + "\n" + text2 + "\n" + text3, "", tr("Continue"), "continue",
                                  tr("Cancel"), "messageBoxCancelButton");
            if (warningBox.exec() == DDialog::Rejected) {
                return;
            }
        }

        DMDbusHandler::instance()->onDeletePVList(devList);
    } else {
        MessageBox warningBox(this);
        warningBox.setObjectName("messageBox");
        warningBox.setAccessibleName("NotEnoughSpaceWidget");
        // 剩余空间不足，移除XXX（磁盘名称）将会造成数据丢失，请先删除逻辑卷 确定
        warningBox.setWarings(tr("Not enough space to back up data on %1, please delete the logical volume first")
                              .arg(realDelPvLis.join(',')), "", tr("OK"), "ok");
        warningBox.exec();
        return;
    }

    setFixedSize(380, 222);
    m_stackedWidget->setCurrentIndex(1);
    DWindowCloseButton *button = findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    if (button != nullptr) {
        button->setDisabled(true);
        button->hide();
    }
    m_waterLoadingWidget->setStartTime(1000);
}

void RemovePVWidget::onPVDeleteMessage(const QString &pvMessage)
{
    QStringList infoList = pvMessage.split(":");

    if (infoList.count() <= 1) {
        return;
    }

    if (infoList.at(0) == "0") {
        // 删除物理卷失败
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget()->parentWidget()->parentWidget(),
                                                 QIcon::fromTheme("://icons/deepin/builtin/warning.svg"),
                                                 tr("Failed to delete the physical volume"));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget()->parentWidget()->parentWidget(),
                                                       QMargins(0, 0, 0, 20));
        close();
    } else {
        close();
    }
}

