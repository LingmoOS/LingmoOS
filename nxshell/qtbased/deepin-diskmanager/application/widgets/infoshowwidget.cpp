// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "infoshowwidget.h"
#include "customcontrol/sizeinfowidget.h"
#include "partedproxy/dmdbushandler.h"
#include "customcontrol/infotopframe.h"
#include "customcontrol/dmframewidget.h"
#include "customcontrol/partitioninfowidget.h"
#include "customcontrol/vgsizeinfowidget.h"
#include "vginfoshowwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

DWIDGET_USE_NAMESPACE

InfoShowWidget::InfoShowWidget(DWidget *parent)
    : DFrame(parent)
{
    initUi();
    initConnection();
}

void InfoShowWidget::initUi()
{
    setWindowFlags(Qt::FramelessWindowHint); //无边框
    setAttribute(Qt::WA_TranslucentBackground); //背景透明

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_frame = new DFrame();
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(m_frame);
    m_frame->setLineWidth(0);

    QVBoxLayout *frameLayout = new QVBoxLayout(m_frame);
    frameLayout->setContentsMargins(10, 10, 10, 10);

    //  右侧 上部数据信息
    m_infoTopFrame = new InfoTopFrame(this);
    frameLayout->addWidget(m_infoTopFrame);
    m_infoTopFrame->setMinimumWidth(500);
    m_infoTopFrame->setLineWidth(0);

    // 右侧 中部数据信息
    m_frameMid = new DFrame;
    m_frameMid->setMinimumWidth(450);
    m_frameMid->setFixedHeight(200);
    m_frameMid->setFrameShape(QFrame::NoFrame);
    frameLayout->addWidget(m_frameMid);
    midFramSettings();

    // 右侧 底部数据信息
    m_frameBottom = new DmFrameWidget(m_diskInfoData);
    m_frameBottom->setFixedHeight(150);
    m_frameBottom->setMinimumWidth(450);
    frameLayout->addWidget(m_frameBottom);
    frameLayout->addStretch();
    bottomFramSettings();
}

void InfoShowWidget::initConnection()
{
    connect(DMDbusHandler::instance(), &DMDbusHandler::curSelectChanged, this, &InfoShowWidget::onCurSelectChanged);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &InfoShowWidget::onHandleChangeTheme);
    connect(m_partitionInfoWidget, &PartitionInfoWidget::enterWidget, this, &InfoShowWidget::onEnterWidget);
    connect(m_partitionInfoWidget, &PartitionInfoWidget::leaveWidget, this, &InfoShowWidget::onLeaveWidget);
    connect(m_vgSizeInfoWidget, &VGSizeInfoWidget::enterWidget, this, &InfoShowWidget::onEnterVGInfoWidget);
}

void InfoShowWidget::midFramSettings()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(m_frameMid);
    m_infoWidget = new SizeInfoWidget;
    m_partitionInfoWidget = new PartitionInfoWidget;
    m_vgSizeInfoWidget = new VGSizeInfoWidget;
    mainLayout->addWidget(m_infoWidget);
    mainLayout->addWidget(m_partitionInfoWidget);
    mainLayout->addWidget(m_vgSizeInfoWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_partitionInfoWidget->hide();
    m_vgSizeInfoWidget->hide();

    m_arrowRectangle = new DArrowRectangle(DArrowRectangle::ArrowBottom, this);
    QColor color("#F8F8F8");
    color.setAlphaF(0.5);
    m_arrowRectangle->setBackgroundColor(color);
    m_arrowRectangle->setArrowWidth(30);
    m_arrowRectangle->setArrowHeight(12);
    m_arrowRectangle->setRadius(15);
    m_arrowRectangle->setWidth(262);
    m_arrowRectangle->setHeight(30);

    m_pathLabel = new DLabel("/dev/sda");
    DFontSizeManager::instance()->bind(m_pathLabel, DFontSizeManager::T6, QFont::Normal);
    m_pathLabel->setAccessibleName("partitionPath");
    DPalette palette;
    palette.setColor(DPalette::WindowText, QColor("#001A2E"));
    m_pathLabel->setPalette(palette);
    m_pathLabel->setAlignment(Qt::AlignCenter);
    m_pathLabel->setFixedHeight(28);
    m_arrowRectangle->setContent(m_pathLabel);

    m_vgInfoShowWidget = new VGInfoShowWidget;
    m_vgInfoShowWidget->setObjectName("vgInfoShowWidget");
}

void InfoShowWidget::bottomFramSettings()
{
    QVBoxLayout *leftInfoLayout = new QVBoxLayout();
    QVBoxLayout *rightInfolayout = new QVBoxLayout();

    m_mountpointLabel = new DLabel(tr("Mount point:"));
    m_mountpointLabel->setAlignment(Qt::AlignLeft);
//    m_mountpointLabel->setWordWrap(true);
//    m_mountpointLabel->adjustSize();
    m_mountpointLabel->setFixedHeight(30);
    DFontSizeManager::instance()->bind(m_mountpointLabel, DFontSizeManager::T6, QFont::Medium);
    m_mountpointLabel->setAccessibleName("Mount point");

    m_freeLabel = new DLabel(tr("Free:"));
//    m_freeLabel->setWordWrap(true);
//    m_freeLabel->adjustSize();
    m_freeLabel->setAlignment(Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_freeLabel, DFontSizeManager::T6, QFont::Medium);
    m_freeLabel->setAccessibleName("Free");

    m_usedLabel = new DLabel(tr("Used:"));
//    m_usedLabel->setWordWrap(true);
//    m_usedLabel->adjustSize();
    m_usedLabel->setAlignment(Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_usedLabel, DFontSizeManager::T6, QFont::Medium);
    m_usedLabel->setAccessibleName("Used");

    m_typeLabel = new DLabel(tr("Type:"));
//    m_typeLabel->setWordWrap(true);
//    m_typeLabel->adjustSize();
    m_typeLabel->setFixedHeight(30);
    m_typeLabel->setAlignment(Qt::AlignTop);
    DFontSizeManager::instance()->bind(m_typeLabel, DFontSizeManager::T6, QFont::Medium);
    m_typeLabel->setAccessibleName("Type");

    m_capacityLabel = new DLabel(tr("Capacity:"));
//    m_capacityLabel->setWordWrap(true);
//    m_capacityLabel->adjustSize();
    DFontSizeManager::instance()->bind(m_capacityLabel, DFontSizeManager::T6, QFont::Medium);
    m_capacityLabel->setAccessibleName("Capacity");

    m_volumeLabel = new DLabel(tr("Volume label:"));
//    m_volumeLabel->setWordWrap(true);
//    m_volumeLabel->adjustSize();
    DFontSizeManager::instance()->bind(m_volumeLabel, DFontSizeManager::T6, QFont::Medium);
    m_volumeLabel->setAccessibleName("Volume label");

    /*connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ] {
        DPalette pa;
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        if (themeType == DGuiApplicationHelper::LightType) {
            pa = palette();
            pa.setColor(DPalette::Text, pa.color(DPalette::Text));

            m_mountpointLabel->setPalette(pa);
            m_freeLabel->setPalette(pa);
            m_usedLabel->setPalette(pa);
            m_typeLabel->setPalette(pa);
            m_capacityLabel->setPalette(pa);
            m_volumeLabel->setPalette(pa);
        } else if (themeType == DGuiApplicationHelper::DarkType) {
            pa = palette();
            pa.setColor(DPalette::Text, QColor("#c0c6d4"));

            m_mountpointLabel->setPalette(pa);
            m_freeLabel->setPalette(pa);
            m_usedLabel->setPalette(pa);
            m_typeLabel->setPalette(pa);
            m_capacityLabel->setPalette(pa);
            m_volumeLabel->setPalette(pa);
        }
    });

    DPalette palette = DApplicationHelper::instance()->palette(m_mountpointLabel);
    palette.setBrush(DPalette::Text, palette.color(DPalette::Text));
    DApplicationHelper::instance()->setPalette(m_mountpointLabel, palette);
    DApplicationHelper::instance()->setPalette(m_freeLabel, palette);
    DApplicationHelper::instance()->setPalette(m_usedLabel, palette);
    DApplicationHelper::instance()->setPalette(m_typeLabel, palette);
    DApplicationHelper::instance()->setPalette(m_capacityLabel, palette);
    DApplicationHelper::instance()->setPalette(m_volumeLabel, palette);*/

    leftInfoLayout->addSpacing(1);
    leftInfoLayout->addWidget(m_mountpointLabel, 0, Qt::AlignLeft);
    leftInfoLayout->addSpacing(18);
    leftInfoLayout->addWidget(m_freeLabel, 0, Qt::AlignLeft);
    leftInfoLayout->addSpacing(22);
    leftInfoLayout->addWidget(m_usedLabel, 0, Qt::AlignLeft);
    leftInfoLayout->addStretch();

    rightInfolayout->addSpacing(1);
    rightInfolayout->addWidget(m_typeLabel, 0, Qt::AlignLeft);
    rightInfolayout->addSpacing(18);
    rightInfolayout->addWidget(m_capacityLabel, 0, Qt::AlignLeft);
    rightInfolayout->addSpacing(22);
    rightInfolayout->addWidget(m_volumeLabel, 0, Qt::AlignLeft);
    rightInfolayout->addStretch();

    QHBoxLayout *mainlayout = new QHBoxLayout(m_frameBottom);
    mainlayout->addLayout(leftInfoLayout);
    mainlayout->addSpacing(10);
    mainlayout->addLayout(rightInfolayout);
}

void InfoShowWidget::onCurSelectChanged()
{
    qDebug() << __FUNCTION__ << "-0--0-";
    if (DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
        m_frameBottom->setFrameData();
        m_mountpointLabel->setText(tr("Mount point:"));
        m_typeLabel->setText(tr("Type:"));
        m_volumeLabel->setText(tr("Volume label:"));

        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        m_noused = Utils::sectorToUnit(info.m_sectorsUnused, info.m_sectorSize, SIZE_UNIT::UNIT_GIB);
        m_used = Utils::sectorToUnit(info.m_sectorsUsed, info.m_sectorSize, SIZE_UNIT::UNIT_GIB);

        QString mountpoints;
        for (QString point : info.m_mountPoints) {
            mountpoints.append(point + " ");
        }

        QString free = Utils::formatSize(info.m_sectorsUnused, info.m_sectorSize);
        QString used = Utils::formatSize(info.m_sectorsUsed, info.m_sectorSize);
        QString fsTypeName = Utils::fileSystemTypeToString(static_cast<FSType>(info.m_fileSystemType));
        QString capacity = Utils::formatSize(info.m_sectorEnd - info.m_sectorStart + 1, info.m_sectorSize);

        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            if (luksInfo.isDecrypt) {
                free = Utils::LVMFormatSize(luksInfo.m_mapper.m_fsUnused);
                used = Utils::LVMFormatSize(luksInfo.m_mapper.m_fsUsed);
                mountpoints = "";
                for (QString point : luksInfo.m_mapper.m_mountPoints) {
                    mountpoints.append(point + " ");
                }

                fsTypeName = DMDbusHandler::instance()->getEncryptionFsType(luksInfo);
                m_noused = Utils::LVMSizeToUnit(luksInfo.m_mapper.m_fsUnused, SIZE_UNIT::UNIT_GIB);
                m_used = Utils::LVMSizeToUnit(luksInfo.m_mapper.m_fsUsed, SIZE_UNIT::UNIT_GIB);
            }
        }

        if (free.contains("-")){
            free = "-";
        }

        if (used.contains("-")){
            used = "-";
        }

        m_mountpointLabel->setObjectName(QString("@==@%1").arg(mountpoints));
        m_freeLabel->setObjectName(QString("@==@%1").arg(free));
        m_usedLabel->setObjectName(QString("@==@%1").arg(used));
        m_typeLabel->setObjectName(QString("@==@%1").arg(fsTypeName));
        m_capacityLabel->setObjectName(QString("@==@%1").arg(capacity));
        m_volumeLabel->setObjectName(QString("@==@%1").arg(info.m_fileSystemLabel));
        m_infoTopFrame->updateDiskInfo();

        if (info.m_vgFlag == LVMFlag::LVM_FLAG_NOT_PV) {
            m_partitionInfoWidget->hide();
            m_vgSizeInfoWidget->hide();
            m_infoWidget->show();

            DPalette palette;
            //    typeLabel->setText(typeLabel->objectName());
            DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
            if (themeType == DGuiApplicationHelper::LightType) {
                fillcolor = QColor("#0091ff");
                fillcolor1 = palette.color(DPalette::Normal, DPalette::ToolTipText);
                fillcolor1.setAlphaF(0.1);
            } else if (themeType == DGuiApplicationHelper::DarkType) {
                fillcolor = QColor("#0059D2");
                fillcolor1 = palette.color(DPalette::Normal, DPalette::BrightText);
                fillcolor1.setAlphaF(0.2);
            }

            QVector<QColor> color {fillcolor, fillcolor1};
            QVector<double> size {m_used, m_noused};
            m_infoWidget->setData(info, color, size, 1);
        } else {
            m_partitionInfoWidget->hide();
            m_vgSizeInfoWidget->show();
            m_infoWidget->hide();

            m_vgSizeInfoWidget->setData(info.m_vgData);
        }
    } else if (DMDbusHandler::DISK == DMDbusHandler::instance()->getCurLevel()) {
        m_mountpointLabel->setText(tr("Path:"));
        m_typeLabel->setText(tr("Disk type:"));
        m_volumeLabel->setText(tr("Interface:"));

        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        if (info.m_mediaType == "SSD") {
            info.m_mediaType = tr("SSD");
        } else if (info.m_mediaType == "HDD") {
            info.m_mediaType = tr("HDD");
        }
        Sector usedSector = 0;
        Sector unusedSector = 0;
        Sector sumUsed = 0;
        Sector sumUnused = 0;
        for (int i = 0; i < info.m_partition.size(); i++) {
            PartitionInfo partitionInfo = info.m_partition.at(i);
            if (partitionInfo.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
                LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(partitionInfo.m_path);
                if (luksInfo.isDecrypt) {
                    if (luksInfo.m_mapper.m_fsUsed > 0) {
                        usedSector += luksInfo.m_mapper.m_fsUsed;
                    }

                    if (luksInfo.m_mapper.m_fsUnused > 0) {
                        unusedSector += luksInfo.m_mapper.m_fsUnused;
                    }

                    sumUsed += luksInfo.m_mapper.m_fsUsed;
                    sumUnused += luksInfo.m_mapper.m_fsUnused;
                } else {
                    usedSector += (partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1) * partitionInfo.m_sectorSize;
                    sumUsed += (partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1) * partitionInfo.m_sectorSize;
                    sumUnused += luksInfo.m_mapper.m_fsUnused;
                }

                continue;
            }

            if (partitionInfo.m_path != "unallocated") {
                if (partitionInfo.m_sectorsUsed > 0) {
                    usedSector += partitionInfo.m_sectorsUsed * partitionInfo.m_sectorSize;
                }

                if (partitionInfo.m_sectorsUnused > 0) {
                    unusedSector += partitionInfo.m_sectorsUnused * partitionInfo.m_sectorSize;
                }

                if (partitionInfo.m_sectorsUsed < 0 && partitionInfo.m_sectorsUnused < 0) {
                    usedSector += (partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1) * partitionInfo.m_sectorSize;
                }

                sumUsed += partitionInfo.m_sectorsUsed * partitionInfo.m_sectorSize;
                sumUnused += partitionInfo.m_sectorsUnused * partitionInfo.m_sectorSize;
            } else {
                unusedSector += (partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1) * partitionInfo.m_sectorSize;
                sumUsed += partitionInfo.m_sectorsUsed * partitionInfo.m_sectorSize;
                sumUnused += (partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1) * partitionInfo.m_sectorSize;
            }
        }

        QString mountpoints = info.m_path;
        QString diskType = info.m_mediaType;
        QString volumeLabel = info.m_interface;

        // 当前选择为磁盘时，是否是整盘加密的磁盘
        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS && info.m_partition.size() == 0) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            m_mountpointLabel->setText(tr("Mount point:"));
            m_typeLabel->setText(tr("Type:"));
            m_volumeLabel->setText(tr("Volume label:"));
            if (luksInfo.isDecrypt) {
                if (luksInfo.m_mapper.m_fsUsed > 0) {
                    usedSector += luksInfo.m_mapper.m_fsUsed;
                }

                if (luksInfo.m_mapper.m_fsUnused > 0) {
                    unusedSector += luksInfo.m_mapper.m_fsUnused;
                }

                sumUsed += luksInfo.m_mapper.m_fsUsed;
                sumUnused += luksInfo.m_mapper.m_fsUnused;

                mountpoints = "";
                for (QString point : luksInfo.m_mapper.m_mountPoints) {
                    mountpoints.append(point + " ");
                }

                diskType = DMDbusHandler::instance()->getEncryptionFsType(luksInfo);
            } else {
                usedSector = -1 * info.m_sectorSize;
                unusedSector = -1 * info.m_sectorSize;
                sumUsed = -1 * info.m_sectorSize;
                sumUnused = -1 * info.m_sectorSize;

                mountpoints = "";
                diskType = Utils::fileSystemTypeToString(FSType::FS_LUKS);
            }

            volumeLabel = luksInfo.m_fileSystemLabel;
        }

        QString diskSize = Utils::formatSize(info.m_length, info.m_sectorSize);
        QString used = Utils::LVMFormatSize(usedSector);
        QString unused = Utils::LVMFormatSize(unusedSector);
        QString sumUsedSize = Utils::LVMFormatSize(sumUsed);
        QString sumUnusedSiz = Utils::LVMFormatSize(sumUnused);

        if (sumUsedSize.contains("-")) {
            used = "-";
        }

        if (sumUnusedSiz.contains("-")) {
            unused = "-";
        }


        m_mountpointLabel->setObjectName(QString("@==@%1").arg(mountpoints));
        m_capacityLabel->setObjectName(QString("@==@%1").arg(diskSize));
        m_freeLabel->setObjectName(QString("@==@%1").arg(unused));
        m_usedLabel->setObjectName(QString("@==@%1").arg(used));
        m_typeLabel->setObjectName(QString("@==@%1").arg(diskType));
        m_volumeLabel->setObjectName(QString("@==@%1").arg(volumeLabel));

        m_frameBottom->setDiskFrameData(mountpoints, diskType, used, unused, diskSize, volumeLabel);
        m_infoTopFrame->updateDiskInfo();

//        m_partitionInfoWidget->hide();
//        m_infoWidget->hide();
//        m_vgSizeInfoWidget->show();
//        m_vgSizeInfoWidget->setData(info);

        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS && info.m_partition.size() == 0) {
            // 当前磁盘是整盘加密时的逻辑处理
            m_partitionInfoWidget->hide();
            m_vgSizeInfoWidget->hide();
            m_infoWidget->show();

            m_noused = Utils::LVMSizeToUnit(unusedSector, SIZE_UNIT::UNIT_GIB);
            m_used = Utils::LVMSizeToUnit(usedSector, SIZE_UNIT::UNIT_GIB);

            DPalette palette;
            DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
            if (themeType == DGuiApplicationHelper::LightType) {
                fillcolor = QColor("#0091ff");
                fillcolor1 = palette.color(DPalette::Normal, DPalette::ToolTipText);
                fillcolor1.setAlphaF(0.1);
            } else if (themeType == DGuiApplicationHelper::DarkType) {
                fillcolor = QColor("#0059D2");
                fillcolor1 = palette.color(DPalette::Normal, DPalette::BrightText);
                fillcolor1.setAlphaF(0.2);
            }

            QVector<QColor> color {fillcolor, fillcolor1};
            QVector<double> size {m_used, m_noused};
            m_infoWidget->setData(info, color, size, 1);
        } else if (DMDbusHandler::instance()->getIsJoinAllVG().value(info.m_path) == "false") {
            // 判断当前磁盘是否全部加入VG，是则显示磁盘下VG的分布情况，否则依然显示磁盘分区的分布情况
            m_partitionInfoWidget->show();
            m_vgSizeInfoWidget->hide();
            m_infoWidget->hide();

            m_partitionInfoWidget->setData(info);
        } else {
            m_partitionInfoWidget->hide();
            m_infoWidget->hide();
            m_vgSizeInfoWidget->show();

            QVector<VGData> vglist = info.m_vglist;
            m_vgSizeInfoWidget->setData(vglist);
        }
    } else if (DMDbusHandler::VOLUMEGROUP == DMDbusHandler::instance()->getCurLevel()) {
        m_partitionInfoWidget->hide();
        m_infoWidget->hide();
        m_vgSizeInfoWidget->show();

        VGInfo vgInfo = DMDbusHandler::instance()->getCurVGInfo();
        QString vgSize = vgInfo.m_vgSize;
        if (vgSize.contains("1024")) {
            vgSize = Utils::LVMFormatSize(vgInfo.m_peCount * vgInfo.m_PESize + vgInfo.m_PESize);
        }

        QString unused = vgInfo.m_vgUnused;
        if (unused.contains("1024")) {
            unused = Utils::LVMFormatSize(vgInfo.m_peCount * vgInfo.m_PESize + vgInfo.m_PESize);
        }

        m_mountpointLabel->setText(tr("LV count:"));
        m_typeLabel->setText(tr("Type:"));
        m_volumeLabel->setText(tr("VG name:"));
        m_mountpointLabel->setObjectName(QString("@==@%1").arg(vgInfo.m_curLV));
        m_capacityLabel->setObjectName(QString("@==@%1").arg(vgSize));
        m_freeLabel->setObjectName(QString("@==@%1").arg(unused));
        m_usedLabel->setObjectName(QString("@==@%1").arg(vgInfo.m_vgUsed));
        m_typeLabel->setObjectName(QString("@==@%1").arg(tr("Volume group")));
        m_volumeLabel->setObjectName(QString("@==@%1").arg(vgInfo.m_vgName));

        m_frameBottom->setDiskFrameData(QString("%1").arg(vgInfo.m_curLV), tr("Volume group"), vgInfo.m_vgUsed,
                                        unused, vgSize, vgInfo.m_vgName);
        m_infoTopFrame->updateDiskInfo();
        m_vgSizeInfoWidget->setData(vgInfo);
    } else if (DMDbusHandler::LOGICALVOLUME == DMDbusHandler::instance()->getCurLevel()) {
        m_partitionInfoWidget->hide();
        m_infoWidget->show();
        m_vgSizeInfoWidget->hide();
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();

        QString mountPoint;
        for (int i = 0; i < lvInfo.m_mountPoints.size(); i++) {
            mountPoint += lvInfo.m_mountPoints[i];
        }

        QString used = Utils::LVMFormatSize(lvInfo.m_fsUsed);
        QString unused = Utils::LVMFormatSize(lvInfo.m_fsUnused);
        double usedSize = Utils::LVMSizeToUnit(lvInfo.m_fsUsed, SIZE_UNIT::UNIT_GIB);
        double unusedSize = Utils::LVMSizeToUnit(lvInfo.m_fsUnused, SIZE_UNIT::UNIT_GIB);
        QString lvName = lvInfo.m_lvName;
        QString lvSize = lvInfo.m_lvSize;

        if (lvInfo.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(lvInfo.m_lvPath);
            if (luksInfo.isDecrypt) {
                unused = Utils::LVMFormatSize(luksInfo.m_mapper.m_fsUnused);
                used = Utils::LVMFormatSize(luksInfo.m_mapper.m_fsUsed);
                mountPoint = "";
                for (int i = 0; i < luksInfo.m_mapper.m_mountPoints.size(); ++i) {
                    mountPoint += luksInfo.m_mapper.m_mountPoints[i];
                }

                unusedSize = Utils::LVMSizeToUnit(luksInfo.m_mapper.m_fsUnused, SIZE_UNIT::UNIT_GIB);
                usedSize = Utils::LVMSizeToUnit(luksInfo.m_mapper.m_fsUsed, SIZE_UNIT::UNIT_GIB);
            } else {
                unused = "-";
                used = "-";
                usedSize = 0.00;
                unusedSize = Utils::LVMSizeToUnit(lvInfo.m_lvLECount * lvInfo.m_LESize, SIZE_UNIT::UNIT_GIB);
            }
        }

        if (lvInfo.m_lvName.isEmpty() && lvInfo.m_lvUuid.isEmpty()) {
            unused = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize);
            unusedSize = Utils::LVMSizeToUnit(lvInfo.m_lvLECount * lvInfo.m_LESize, SIZE_UNIT::UNIT_GIB);
            lvName = "unallocated";
        }

        if (usedSize == 0.00 && unusedSize == 0.00) {
            unused = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize);
        }

        if (used.contains("-")) {
            used = "-";
        }

        if (unused.contains("-")) {
            unused = "-";
        }

        if (lvSize.contains("1024")) {
            lvSize = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize + lvInfo.m_LESize);
        }

        if (unused.contains("1024")) {
            unused = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize + lvInfo.m_LESize);
        }

        m_mountpointLabel->setText(tr("Mount point:"));
        m_typeLabel->setText(tr("Type:"));
        m_volumeLabel->setText(tr("Volume name:"));
        m_mountpointLabel->setObjectName(QString("@==@%1").arg(mountPoint));
        m_capacityLabel->setObjectName(QString("@==@%1").arg(lvSize));
        m_freeLabel->setObjectName(QString("@==@%1").arg(unused));
        m_usedLabel->setObjectName(QString("@==@%1").arg(used));
        m_typeLabel->setObjectName(QString("@==@%1").arg(tr("Logical volume")));
        m_volumeLabel->setObjectName(QString("@==@%1").arg(lvName));

        m_frameBottom->setDiskFrameData(mountPoint, tr("Logical volume"), used, unused, lvSize, lvName);
        m_infoTopFrame->updateDiskInfo();

        DPalette palette;
    //    typeLabel->setText(typeLabel->objectName());
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        if (themeType == DGuiApplicationHelper::LightType) {
            fillcolor = QColor("#0091ff");
            fillcolor1 = palette.color(DPalette::Normal, DPalette::ToolTipText);
            fillcolor1.setAlphaF(0.1);
        } else if (themeType == DGuiApplicationHelper::DarkType) {
            fillcolor = QColor("#0059D2");
            fillcolor1 = palette.color(DPalette::Normal, DPalette::BrightText);
            fillcolor1.setAlphaF(0.2);
        }

        QVector<QColor> color {fillcolor, fillcolor1};
        QVector<double> size {usedSize, unusedSize};
        m_infoWidget->setData(lvInfo, color, size, 1);
    }
}

void InfoShowWidget::onHandleChangeTheme()
{
    DPalette palette;
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        fillcolor = QColor("#0091ff");
        fillcolor1 = palette.color(DPalette::Normal, DPalette::ToolTipText);
        fillcolor1.setAlphaF(0.1);

    } else if (themeType == DGuiApplicationHelper::DarkType) {
        fillcolor = QColor("#0059D2");
        fillcolor1 = palette.color(DPalette::Normal, DPalette::BrightText);
        fillcolor1.setAlphaF(0.2);
    }

    m_noused = Utils::sectorToUnit(DMDbusHandler::instance()->getCurPartititonInfo().m_sectorsUnused, DMDbusHandler::instance()->getCurPartititonInfo().m_sectorSize, SIZE_UNIT::UNIT_GIB);
    m_used = Utils::sectorToUnit(DMDbusHandler::instance()->getCurPartititonInfo().m_sectorsUsed, DMDbusHandler::instance()->getCurPartititonInfo().m_sectorSize, SIZE_UNIT::UNIT_GIB);

    QVector<QColor> color {fillcolor, fillcolor1};
    QVector<double> size {m_used, m_noused};
    m_infoWidget->setData(DMDbusHandler::instance()->getCurPartititonInfo(), color, size, 1);
}

void InfoShowWidget::onEnterWidget(QRectF rect, QString path)
{
    if ("unallocated" != path) {
        path = path.remove(0, 5);
    }

    m_pathLabel->setText(path);
    QFontMetrics fmLabel = m_pathLabel->fontMetrics();
    int labelWidth = fmLabel.width(path);
    m_pathLabel->setFixedWidth(labelWidth + 20);

    m_arrowRectangle->show(m_partitionInfoWidget->mapToGlobal(m_partitionInfoWidget->pos()).x() + (rect.x() + rect.width() / 2),
                           m_partitionInfoWidget->mapToGlobal(m_partitionInfoWidget->pos()).y() + rect.y());
}

void InfoShowWidget::onLeaveWidget()
{
    m_arrowRectangle->hide();
}

void InfoShowWidget::onEnterVGInfoWidget(QRect rect, const QList<QMap<QString, QVariant> > &lstInfo)
{
    if (!m_vgInfoShowWidget->isHidden()) {
        return;
    }

    m_vgInfoShowWidget->setData(lstInfo);
    m_vgInfoShowWidget->setGeometry(m_vgSizeInfoWidget->mapToGlobal(m_vgSizeInfoWidget->pos()).x() + (rect.x()/* + rect.width() / 2*/),
                                    m_vgSizeInfoWidget->mapToGlobal(m_vgSizeInfoWidget->pos()).y() + rect.y() + rect.height(), 100, 100);
    m_vgInfoShowWidget->show();
}

