// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "infotopframe.h"
#include "partedproxy/dmdbushandler.h"
#include "common.h"

#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>

InfoTopFrame::InfoTopFrame(DWidget *parent)
    : DFrame(parent)
{
    setFixedHeight(110);
    setBackgroundRole(DPalette::ItemBackground);

    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setContentsMargins(20, 0, 20, 0);
    setLayout(m_mainLayout);

    m_pictureLabel = new DLabel(this);
    if (DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
        m_pictureLabel->setPixmap(Common::getIcon("labeldisk").pixmap(85, 85));
    } else if (DMDbusHandler::DISK == DMDbusHandler::instance()->getCurLevel()) {
        m_pictureLabel->setPixmap(Common::getIcon("disk").pixmap(85, 85));
    } else if (DMDbusHandler::VOLUMEGROUP == DMDbusHandler::instance()->getCurLevel()) {
        m_pictureLabel->setPixmap(Common::getIcon("vg").pixmap(85, 85));
    } else if (DMDbusHandler::LOGICALVOLUME == DMDbusHandler::instance()->getCurLevel()) {
        m_pictureLabel->setPixmap(Common::getIcon("lv").pixmap(85, 85));
    }

    m_pictureLabel->setMinimumSize(85, 85);
    m_mainLayout->addWidget(m_pictureLabel);

    initLeftInfo();

    m_mainLayout->addStretch();

    initRightInfo();
}

void InfoTopFrame::initRightInfo()
{
    m_allNameLabel = new DLabel(tr("Capacity"));
    m_allNameLabel->setAlignment(Qt::AlignRight);
    DFontSizeManager::instance()->bind(m_allNameLabel, DFontSizeManager::T6);

    m_allMemoryLabel = new DLabel;
    m_allMemoryLabel->setAlignment(Qt::AlignRight);
    DFontSizeManager::instance()->bind(m_allMemoryLabel, DFontSizeManager::T2);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addStretch();
    layout->addWidget(m_allNameLabel);
    layout->addWidget(m_allMemoryLabel);
    layout->addStretch();

    m_mainLayout->addLayout(layout);
}

void InfoTopFrame::initLeftInfo()
{
    m_nameLabel = new DLabel;
    QFont nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T5);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);

    m_typeLabel = new DLabel;
    DFontSizeManager::instance()->bind(m_typeLabel, DFontSizeManager::T8);

    /*DPalette palette = DApplicationHelper::instance()->palette(m_typeLabel);
    palette.setBrush(DPalette::Text, palette.textTips());
    DApplicationHelper::instance()->setPalette(m_typeLabel, palette);*/

    QVBoxLayout *layout = new QVBoxLayout();
//    layout->addStretch();
    layout->addSpacing(20);
    layout->addWidget(m_nameLabel);
    layout->addSpacing(10);
    layout->addWidget(m_typeLabel);
    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addLayout(layout);
}

void InfoTopFrame::updateDiskInfo()
{
    if (DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
        auto info = DMDbusHandler::instance()->getCurPartititonInfo();

        if (info.m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV) {
            m_pictureLabel->setPixmap(Common::getIcon("lv").pixmap(85, 85));
        } else if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            m_pictureLabel->setPixmap(Common::getIcon("partitionlock").pixmap(85, 85));
        } else {
            m_pictureLabel->setPixmap(Common::getIcon("labeldisk").pixmap(85, 85));
        }

        QFontMetrics fmDevpath = m_nameLabel->fontMetrics();
        QString textDevpath = info.m_path;
        int devpathWidth = fmDevpath.width(textDevpath);
        if (devpathWidth > width() / 2) {
            devpathWidth = width() / 2;
            textDevpath = m_nameLabel->fontMetrics().elidedText(textDevpath, Qt::ElideMiddle, devpathWidth);
        }
        m_nameLabel->setText(textDevpath);
    //    if ("unallocated" == info.m_path) {
    //        m_nameLabel->setText("ocated");
    //    }

        QString diskSize = Utils::formatSize(info.m_sectorEnd - info.m_sectorStart + 1,
                                                info.m_sectorSize);
        m_allMemoryLabel->setText(diskSize);

        QString diskType = Utils::fileSystemTypeToString(static_cast<FSType>(info.m_fileSystemType));
        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            if (luksInfo.isDecrypt) {
                diskType = DMDbusHandler::instance()->getEncryptionFsType(luksInfo);
            }
        }
        m_typeLabel->setText(tr("File system") + ": " + diskType);
    } else if (DMDbusHandler::DISK == DMDbusHandler::instance()->getCurLevel()) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();

        if (DMDbusHandler::instance()->getIsJoinAllVG().value(info.m_path) == "true") {
            m_pictureLabel->setPixmap(Common::getIcon("vg").pixmap(85, 85));
        } else if (DMDbusHandler::instance()->getIsAllEncryption().value(info.m_path) == "true") {
            m_pictureLabel->setPixmap(Common::getIcon("disklock").pixmap(85, 85));
        } else {
            m_pictureLabel->setPixmap(Common::getIcon("disk").pixmap(85, 85));
        }

        QString diskSize = Utils::formatSize(info.m_length, info.m_sectorSize);
        m_allMemoryLabel->setText(diskSize);

        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS && info.m_partition.size() == 0) {
            // 处理磁盘是整盘加密页面显示
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            m_nameLabel->setText(info.m_path);
            if (luksInfo.isDecrypt) {
                m_typeLabel->setText(tr("File system") + ": " + DMDbusHandler::instance()->getEncryptionFsType(luksInfo));
            } else {
                m_typeLabel->setText(tr("File system") + ": " + Utils::fileSystemTypeToString(FSType::FS_LUKS));
            }
        } else {
            m_nameLabel->setText(info.m_model);
            QString partitionTable;
            if (info.m_disktype == "gpt") {
                partitionTable = "GPT";
            } else if (info.m_disktype == "msdos") {
                partitionTable = "MBR";
            } else {
                partitionTable = info.m_disktype;
            }
            m_typeLabel->setText(tr("%1 partition table").arg(partitionTable));
        }

    } else if (DMDbusHandler::VOLUMEGROUP == DMDbusHandler::instance()->getCurLevel()) {
        VGInfo vgInfo = DMDbusHandler::instance()->getCurVGInfo();

        if (DMDbusHandler::instance()->getIsAllEncryption().value(vgInfo.m_vgName) == "true") {
            m_pictureLabel->setPixmap(Common::getIcon("vglock").pixmap(85, 85));
        } else {
            m_pictureLabel->setPixmap(Common::getIcon("vg").pixmap(85, 85));
        }

        m_nameLabel->setText(vgInfo.m_vgName);
        QString vgSize = vgInfo.m_vgSize;
        if (vgSize.contains("1024")) {
            vgSize = Utils::LVMFormatSize(vgInfo.m_peCount * vgInfo.m_PESize + vgInfo.m_PESize);
        }
        m_allMemoryLabel->setText(vgSize);

        m_typeLabel->setText(tr("Volume group"));
    } else if (DMDbusHandler::LOGICALVOLUME == DMDbusHandler::instance()->getCurLevel()) {
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();

        if (lvInfo.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            m_pictureLabel->setPixmap(Common::getIcon("lvlock").pixmap(85, 85));
        } else {
            m_pictureLabel->setPixmap(Common::getIcon("lv").pixmap(85, 85));
        }

        m_nameLabel->setText(lvInfo.m_lvPath);
        QString lvSize = lvInfo.m_lvSize;
        if (lvSize.contains("1024")) {
            lvSize = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize + lvInfo.m_LESize);
        }
        m_allMemoryLabel->setText(lvSize);

        QString fstypeName = Utils::fileSystemTypeToString(static_cast<FSType>(lvInfo.m_lvFsType));
        if (lvInfo.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(lvInfo.m_lvPath);
            if (luksInfo.isDecrypt) {
                fstypeName = DMDbusHandler::instance()->getEncryptionFsType(luksInfo);
            }
        }
        m_typeLabel->setText(tr("File system") + ": " + fstypeName);
    }
}

void InfoTopFrame::resizeEvent(QResizeEvent *event)
{
    if (DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
        auto info = DMDbusHandler::instance()->getCurPartititonInfo();

        QFontMetrics fmDevpath = m_nameLabel->fontMetrics();
        QString textDevpath = info.m_path;
        int devpathWidth = fmDevpath.width(textDevpath);
        if (devpathWidth > width() / 2) {
            devpathWidth = width() / 2;
            textDevpath = m_nameLabel->fontMetrics().elidedText(textDevpath, Qt::ElideMiddle, devpathWidth);
        }
        m_nameLabel->setText(textDevpath);
    }

    DFrame::resizeEvent(event);
}
