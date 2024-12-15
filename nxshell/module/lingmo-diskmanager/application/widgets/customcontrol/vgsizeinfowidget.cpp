// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "vgsizeinfowidget.h"
#include "partedproxy/dmdbushandler.h"

#include <DFontSizeManager>

#include <QMouseEvent>
#include <QApplication>

VGSizeInfoWidget::VGSizeInfoWidget(QWidget *parent) : QWidget(parent)
{
    QColor color("#70BEFF");    //浅蓝色
    QColor color1("#4293FF");   //蓝色
    QColor color2("#6751E4");   //紫色
    QColor color3("#FA7104");   //橙色
    QColor color4("#FFD027");   //黄色
    QColor color5("#2CCBBE");   //绿色
    QColor color6("#2C58CB");   //深蓝色
    QColor color7("#D468F0");   //浅紫色
    QColor color8("#FA7BA4");   //粉色
    QColor color9("#BFDC58");   //浅绿色
    m_colorInfo = QVector<QColor> {color, color1, color2, color3, color4, color5, color6, color7, color8, color9};

    setMouseTracking(true);
}

void VGSizeInfoWidget::setData(const DeviceInfo &info)
{
    m_sizeInfo.clear();
    m_pathInfo.clear();
    m_pathList.clear();
    m_pathSizeInfo.clear();
    m_totalSize = 0.00;

    for (int i = 0; i < info.m_partition.size(); i++) {
        PartitionInfo partitionInfo = info.m_partition.at(i);
        double partitionSize = Utils::sectorToUnit(partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1, partitionInfo.m_sectorSize, SIZE_UNIT::UNIT_GIB);

        m_sizeInfo.append(partitionSize);
        m_pathInfo.append(partitionInfo.m_path);
        if (partitionInfo.m_path == "unallocated") {
            m_pathList.insert(0, partitionInfo.m_path);
            m_pathSizeInfo.insert(0, Utils::formatSize(partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1, partitionInfo.m_sectorSize));
        } else {
            m_pathList.append(partitionInfo.m_path);
            m_pathSizeInfo.append(Utils::formatSize(partitionInfo.m_sectorEnd - partitionInfo.m_sectorStart + 1, partitionInfo.m_sectorSize));
        }

        if (FSType::FS_EXTENDED == static_cast<FSType>(partitionInfo.m_fileSystemType)) {
            m_totalSize = partitionSize;
        }
    }

    m_totalSize += Utils::sectorToUnit(info.m_length, info.m_sectorSize, SIZE_UNIT::UNIT_GIB);

    update();
}

void VGSizeInfoWidget::setData(const VGInfo &info)
{
    m_sizeInfo.clear();
    m_pathInfo.clear();
    m_pathList.clear();
    m_pathSizeInfo.clear();
    m_totalSize = 0.00;
    Sector vgSize = static_cast<Sector>(info.m_peCount * info.m_PESize);
    m_totalSize = Utils::LVMSizeToUnit(vgSize, SIZE_UNIT::UNIT_GIB);

    for (int i = 0; i < info.m_lvlist.count(); i++) {
        LVInfo lvInfo = info.m_lvlist.at(i);
        Sector lvByteSize = static_cast<Sector>(lvInfo.m_lvLECount * lvInfo.m_LESize);
        double lvSize = Utils::LVMSizeToUnit(lvByteSize, SIZE_UNIT::UNIT_GIB);

        QString lvSizeInfo = lvInfo.m_lvSize;
        if (lvSizeInfo.contains("1024")) {
            lvSizeInfo = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize + lvInfo.m_LESize);
        }

        if (lvInfo.m_lvName.isEmpty() && lvInfo.m_lvUuid.isEmpty()) {
            m_sizeInfo.append(lvSize);
            m_pathInfo.append("unallocated");
            m_pathList.insert(0, "unallocated");
            m_pathSizeInfo.insert(0, lvSizeInfo);
        } else {
            m_sizeInfo.append(lvSize);
            m_pathInfo.append(lvInfo.m_lvName);
            m_pathList.append(lvInfo.m_lvName);
            m_pathSizeInfo.append(lvSizeInfo);
        }
    }

    update();
}

void VGSizeInfoWidget::setData(const QVector<VGData> &vglist)
{
    if (vglist.count() == 0 ) {
        return;
    }

    m_sizeInfo.clear();
    m_pathInfo.clear();
    m_pathList.clear();
    m_pathSizeInfo.clear();
    m_totalSize = 0.00;

    // 判断磁盘下是否存在多个VG，若存在就显示VG的分布情况，否则就显示VG下LV的分布情况
    if (vglist.count() > 1) {
        QStringList vgNameList;
        bool isUnallocated = false;
        Sector totalVGSize = 0;

        for (int i = 0; i < vglist.count(); i++) {
            VGData vgData = vglist.at(i);
            totalVGSize += vgData.m_vgByteSize;

            if (vgData.m_vgName.isEmpty() && vgData.m_vgUuid.isEmpty() && !isUnallocated) {
                isUnallocated = true;
            } else if (-1 == vgNameList.indexOf(vgData.m_vgName)) {
                vgNameList.append(vgData.m_vgName);
            }
        }

        qSort(vgNameList.begin(), vgNameList.end());

        if (isUnallocated) {
            vgNameList.append("unallocated");
        }

        m_totalSize = Utils::LVMSizeToUnit(totalVGSize, SIZE_UNIT::UNIT_GIB);
        if (vgNameList.count() > 1) {
            for (int i = 0; i < vgNameList.count(); i++) {
                if ("unallocated" == vgNameList.at(i)) {
                    double vgSize = 0.00;
                    Sector sumSize = 0;
                    for (int j = 0; j < vglist.count(); j++) {
                        VGData vgData = vglist.at(j);
                        if (vgData.m_vgName.isEmpty() && vgData.m_vgUuid.isEmpty()) {
                            vgSize += Utils::LVMSizeToUnit(vgData.m_vgByteSize, SIZE_UNIT::UNIT_GIB);
                            sumSize += vgData.m_vgByteSize;
                        }
                    }

                    m_sizeInfo.append(vgSize);
                    m_pathInfo.append("unallocated");
                    m_pathList.insert(0, "unallocated");
                    m_pathSizeInfo.insert(0, Utils::LVMFormatSize(sumSize));
                } else {
                    double vgSize = 0.00;
                    Sector sumSize = 0;
                    for (int j = 0; j < vglist.count(); j++) {
                        VGData vgData = vglist.at(j);
                        if (vgData.m_vgName == vgNameList.at(i)) {
                            vgSize += Utils::LVMSizeToUnit(vgData.m_vgByteSize, SIZE_UNIT::UNIT_GIB);
                            sumSize += vgData.m_vgByteSize;
                        }
                    }

                    m_sizeInfo.append(vgSize);
                    m_pathInfo.append(vgNameList.at(i));
                    m_pathList.append(vgNameList.at(i));
                    m_pathSizeInfo.append(Utils::LVMFormatSize(sumSize));
                }
            }
        } else {
            QStringList lvNameList;
            Sector totalVGSize = 0;
            for (int i = 0; i < vglist.count(); i++) {
                VGData vgData = vglist.at(i);
                totalVGSize += vgData.m_vgByteSize;
                QVector<LVData> lvList = vgData.m_lvList;

                for (int j = 0; j < lvList.count(); j++) {
                    LVData lvData = lvList.at(j);

                    if (-1 == lvNameList.indexOf(lvData.m_lvName) && !lvData.m_lvName.isEmpty() && "unallocated" != lvData.m_lvName) {
                        lvNameList.append(lvData.m_lvName);
                    }
                }
            }

            m_totalSize = Utils::LVMSizeToUnit(totalVGSize, SIZE_UNIT::UNIT_GIB);
            qSort(lvNameList.begin(), lvNameList.end());

            if (lvNameList.count() > 0) {
                Sector lvSizeSum = 0;
                for (int i = 0; i < lvNameList.count(); i++) {
                    Sector lvSize = 0;
                    for (int j = 0; j < vglist.count(); j++) {
                        VGData vgData = vglist.at(j);
                        QVector<LVData> lvList = vgData.m_lvList;

                        for (int k = 0; k < lvList.count(); k++) {
                            LVData lvData = lvList.at(k);

                            if (lvData.m_lvName == lvNameList.at(i)) {
                                lvSize += lvData.m_lvByteSize;
                            }
                        }
                    }

                    m_sizeInfo.append(Utils::LVMSizeToUnit(lvSize, SIZE_UNIT::UNIT_GIB));
                    m_pathInfo.append(QString("%1-%2").arg(vgNameList.at(0)).arg(lvNameList.at(i)));
                    m_pathList.append(QString("%1-%2").arg(vgNameList.at(0)).arg(lvNameList.at(i)));
                    m_pathSizeInfo.append(Utils::LVMFormatSize(lvSize));

                    lvSizeSum += lvSize;
                }

                Sector difference = totalVGSize - lvSizeSum;
                if (difference > 0) {
                    m_sizeInfo.append(Utils::LVMSizeToUnit(difference, SIZE_UNIT::UNIT_GIB));
                    m_pathInfo.append("unallocated");
                    m_pathList.insert(0, "unallocated");
                    m_pathSizeInfo.insert(0, Utils::LVMFormatSize(difference));
                }
            } else {
                double vgSize = Utils::LVMSizeToUnit(totalVGSize, SIZE_UNIT::UNIT_GIB);
                m_sizeInfo.append(vgSize);
                m_pathInfo.append(vgNameList.at(0));
                m_pathList.append(vgNameList.at(0));
                m_pathSizeInfo.append(Utils::LVMFormatSize(totalVGSize));
            }
        }
    } else {
        VGData vgData = vglist.at(0);
        QVector<LVData> lvList = vgData.m_lvList;
        m_totalSize = Utils::LVMSizeToUnit(vgData.m_vgByteSize, SIZE_UNIT::UNIT_GIB);
        LVMInfo lvmInfo = DMDbusHandler::instance()->probLVMInfo();

        // 判断VG下是否存在LV，若存在就显示LV的分布情况，否则就显示VG的分布情况
        if (lvList.count() > 0) {
            Sector lvSizeSum = 0;
            for (int i = 0; i < lvList.count(); i++) {
                LVData lvData = lvList.at(i);
                double lvSize = Utils::LVMSizeToUnit(lvData.m_lvByteSize, SIZE_UNIT::UNIT_GIB);

                m_sizeInfo.append(lvSize);
                m_pathInfo.append(QString("%1-%2").arg(vgData.m_vgName).arg(lvData.m_lvName));
                m_pathList.append(QString("%1-%2").arg(vgData.m_vgName).arg(lvData.m_lvName));

                QString lvSizeInfo = lvData.m_lvSize;
                if (lvSizeInfo.contains("1024") && lvmInfo.lvInfoExists(vgData.m_vgName, lvData.m_lvName)) {
                    LVInfo lvInfo = lvmInfo.getLVInfo(vgData.m_vgName, lvData.m_lvName);
                    lvSizeInfo = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize + lvInfo.m_LESize);
                }
                m_pathSizeInfo.append(lvSizeInfo);

                lvSizeSum += lvData.m_lvByteSize;
            }

            Sector difference = vgData.m_vgByteSize - lvSizeSum;
            if (difference > 0) {
                m_sizeInfo.append(Utils::LVMSizeToUnit(difference, SIZE_UNIT::UNIT_GIB));
                m_pathInfo.append("unallocated");
                m_pathList.insert(0, "unallocated");
                m_pathSizeInfo.insert(0, Utils::LVMFormatSize(difference));
            }
        } else {
            double vgSize = Utils::LVMSizeToUnit(vgData.m_vgByteSize, SIZE_UNIT::UNIT_GIB);
            m_sizeInfo.append(vgSize);
            m_pathInfo.append(vgData.m_vgName);
            m_pathList.append(vgData.m_vgName);

            QString vgSizeInfo = vgData.m_vgSize;
            if (vgSizeInfo.contains("1024") && lvmInfo.vgExists(vgData.m_vgName)) {
                VGInfo vgInfo = lvmInfo.getVG(vgData.m_vgName);
                vgSizeInfo = Utils::LVMFormatSize(vgInfo.m_peCount * vgInfo.m_PESize + vgInfo.m_PESize);
            }
            m_pathSizeInfo.append(vgSizeInfo);
        }
    }

    update();
}

void VGSizeInfoWidget::setData(const VGData &vgData)
{
    m_sizeInfo.clear();
    m_pathInfo.clear();
    m_pathList.clear();
    m_pathSizeInfo.clear();
    m_totalSize = 0.00;
    QVector<LVData> lvList = vgData.m_lvList;
    LVMInfo lvmInfo = DMDbusHandler::instance()->probLVMInfo();

    // 判断VG下是否存在LV，若存在就显示LV的分布情况，否则就显示VG的分布情况
    if (lvList.count() > 0) {
        Sector lvSizeSum = 0;
        Sector vgSize = vgData.m_vgByteSize;
        m_totalSize = Utils::LVMSizeToUnit(vgSize, SIZE_UNIT::UNIT_GIB);

        for (int i = 0; i < lvList.count(); i++) {
            LVData lvData = lvList.at(i);
            double lvSize = Utils::LVMSizeToUnit(lvData.m_lvByteSize, SIZE_UNIT::UNIT_GIB);

            m_sizeInfo.append(lvSize);
            m_pathInfo.append(QString("%1-%2").arg(vgData.m_vgName).arg(lvData.m_lvName));
            m_pathList.append(QString("%1-%2").arg(vgData.m_vgName).arg(lvData.m_lvName));

            QString lvSizeInfo = lvData.m_lvSize;
            if (lvSizeInfo.contains("1024") && lvmInfo.lvInfoExists(vgData.m_vgName, lvData.m_lvName)) {
                LVInfo lvInfo = lvmInfo.getLVInfo(vgData.m_vgName, lvData.m_lvName);
                lvSizeInfo = Utils::LVMFormatSize(lvInfo.m_lvLECount * lvInfo.m_LESize + lvInfo.m_LESize);
            }
            m_pathSizeInfo.append(lvSizeInfo);

            lvSizeSum += lvData.m_lvByteSize;
        }

        Sector difference = vgSize - lvSizeSum;
        if (difference > 0) {
            m_sizeInfo.append(Utils::LVMSizeToUnit(difference, SIZE_UNIT::UNIT_GIB));
            m_pathInfo.append("unallocated");
            m_pathList.insert(0, "unallocated");
            m_pathSizeInfo.insert(0, Utils::LVMFormatSize(difference));
        }
    } else {
        m_totalSize = Utils::LVMSizeToUnit(vgData.m_vgByteSize, SIZE_UNIT::UNIT_GIB);
        double vgSize = Utils::LVMSizeToUnit(vgData.m_vgByteSize, SIZE_UNIT::UNIT_GIB);
        m_sizeInfo.append(vgSize);
        m_pathInfo.append(vgData.m_vgName);
        m_pathList.append(vgData.m_vgName);

        QString vgSizeInfo = vgData.m_vgSize;
        if (vgSizeInfo.contains("1024") && lvmInfo.vgExists(vgData.m_vgName)) {
            VGInfo vgInfo = lvmInfo.getVG(vgData.m_vgName);
            vgSizeInfo = Utils::LVMFormatSize(vgInfo.m_peCount * vgInfo.m_PESize + vgInfo.m_PESize);
        }
        m_pathSizeInfo.append(vgSizeInfo);
    }

    update();
}

void VGSizeInfoWidget::paintEvent(QPaintEvent *event)
{
    if (m_sizeInfo.isEmpty()) {
        return;
    }

    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    QRect rect;
    QVector<QPainterPath> path;
    for (int i = 0; i < m_sizeInfo.size(); i++) {
        QPainterPath painterPath;
        path.append(painterPath);
    }

    DPalette palette;
    QColor color;
    QColor nameColor;
    QColor capacityColor;
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        color = palette.color(DPalette::Normal, DPalette::ToolTipText);
        color.setAlphaF(0.1);

        nameColor = QColor("#414D68");
        capacityColor = QColor("#526A7F");
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        color = palette.color(DPalette::Normal, DPalette::BrightText);
        color.setAlphaF(0.2);

        nameColor = QColor("#C0C6D4");
        capacityColor = QColor("#6D7C88");
    }

    m_pathColorInfo.clear();
    if (m_pathInfo.indexOf("unallocated") != -1) {
        m_pathColorInfo << color;
    }

    rect.setX(x());
    rect.setY(y());
    rect.setWidth(width());
    rect.setHeight(height());
    QRect paintRect = QRect(0, 60, rect.width() + 1, rect.height() - 170);

    // 解决分区占比很小时，分区不显示的问题
    double smallSum = 0.0;
    int bigCount = 0;
    int smallCount = 0;
    for (int i = 0; i < m_sizeInfo.size(); i++) {
        if (m_sizeInfo[i] / m_totalSize < 0.01 || (m_sizeInfo[i] / m_totalSize) * (paintRect.width() - 8) < 8) {
            smallCount++;
            smallSum += (m_sizeInfo[i] / m_totalSize) * (paintRect.width() - 8);
        } else {
            bigCount++;
        }
    }
    double space = (smallCount * 8 - smallSum) / bigCount;

    m_reectInfo.clear();
    int radius = 8;
    if (1 == m_sizeInfo.size()) {
        path[0].moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path[0].lineTo(paintRect.topRight() + QPoint(0, radius));
        path[0].arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                            QSize(radius * 2, radius * 2)), 0, 90);
        path[0].lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path[0].arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path[0].lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path[0].arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                            QSize(radius * 2, radius * 2)), 180, 90);
        path[0].lineTo(paintRect.bottomRight() - QPoint(radius, 0));
        path[0].arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                            QSize(radius * 2, radius * 2)), 270, 90);

        if (m_pathInfo.at(0) == "unallocated") {
            painter.setBrush(QBrush(color));
            painter.setPen(QPen(color, 3));
            painter.fillPath(path[0], color);
        } else {
            painter.setBrush(QBrush(m_colorInfo[0]));
            painter.setPen(QPen(QColor(m_colorInfo[0]), 3));
            painter.fillPath(path[0], m_colorInfo[0]);
            m_pathColorInfo.append(m_colorInfo[0]);
        }

        m_reectInfo.append(path[0].controlPointRect());
    } else {
        //根据color和size数据遍历绘制矩形
        for (int i = 0; i < m_sizeInfo.size(); i++) {
            double widths = (m_sizeInfo[i] / m_totalSize) * (paintRect.width() - radius);
            double width1 = 0.00;
            widths = widths - space;
            if (m_sizeInfo[i] / m_totalSize < 0.01 || widths < radius) {
                widths = 8;
            }
            if (i == 0) {
                path[0].moveTo(paintRect.topLeft() + QPoint(radius, 0));
                path[0].arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
                path[0].lineTo(paintRect.bottomLeft() - QPoint(0, radius));
                path[0].arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                                    QSize(radius * 2, radius * 2)), 180, 90);
                path[0].lineTo(paintRect.bottomLeft() + QPoint(static_cast<int>(widths + radius), 0));
                path[0].lineTo(paintRect.topLeft() + QPoint(static_cast<int>(widths + radius), 0));
                path[0].lineTo(paintRect.topLeft() + QPoint(radius, 0));
                if (m_pathInfo.at(0) == "unallocated") {
                    painter.setBrush(QBrush(color));
                    painter.setPen(QPen(color, 3));
                    painter.fillPath(path[0], color);
                } else {
                    painter.setBrush(QBrush(m_colorInfo[0]));
                    painter.setPen(QPen(QColor(m_colorInfo[0]), 3));
                    painter.fillPath(path[0], m_colorInfo[0]);
                    m_pathColorInfo.append(m_colorInfo[0]);
                }

                m_reectInfo.append(path[0].controlPointRect());
            } else if (i > 0 && i < m_sizeInfo.size() - 1) {
                width1 = (m_sizeInfo[i - 1] / m_totalSize) * (paintRect.width() - radius);
                width1 = width1 - space;
                if (width1 < 8 || m_sizeInfo[i - 1] / m_totalSize < 0.01) {
                    width1 = 8;
                }

                path[i].moveTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1), 0));
                path[i].lineTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1 + widths), 0));
                path[i].lineTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1 + widths), paintRect.height() - 1));
                path[i].lineTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1), paintRect.height() - 1));
                path[i].lineTo(path[i - 1].currentPosition() + QPoint(static_cast<int>(width1), 0));

                if (m_pathInfo.at(i) == "unallocated") {
                    painter.setBrush(QBrush(color));
                    painter.setPen(QPen(color, 3));
                    painter.fillPath(path[i], color);
                } else {
                    painter.setBrush(QBrush(m_colorInfo[i % (m_colorInfo.size())]));
                    painter.setPen(QPen(QColor(m_colorInfo[i % (m_colorInfo.size())]), 3));
                    painter.fillPath(path[i], m_colorInfo[i % (m_colorInfo.size())]);
                    m_pathColorInfo.append(m_colorInfo[i % (m_colorInfo.size())]);
                }

                m_reectInfo.append(path[i].controlPointRect());
            } else if (i == m_sizeInfo.size() - 1) {
                double width = ((m_sizeInfo[m_sizeInfo.size() - 2] / m_totalSize)) * (paintRect.width() - radius) - space;
                if (m_sizeInfo[m_sizeInfo.size() - 2] / m_totalSize < 0.01) {
                    width = 8;
                }

                path[m_sizeInfo.size() - 1].moveTo(paintRect.bottomRight() - QPoint(0, radius));
                path[m_sizeInfo.size() - 1].lineTo(paintRect.topRight() + QPoint(0, radius));
                path[m_sizeInfo.size() - 1].arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                                                        QSize(radius * 2, radius * 2)), 0, 90);
                path[m_sizeInfo.size() - 1].lineTo(path[m_sizeInfo.size() - 2].currentPosition() + QPoint(static_cast<int>(width), 0));
                path[m_sizeInfo.size() - 1].lineTo(path[m_sizeInfo.size() - 2].currentPosition() + QPoint(static_cast<int>(width), paintRect.height() - 1));
                path[m_sizeInfo.size() - 1].lineTo(paintRect.bottomRight() - QPoint(radius, 0));
                path[m_sizeInfo.size() - 1].arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                                                        QSize(radius * 2, radius * 2)), 270, 90);
                if (m_pathInfo.at(m_sizeInfo.size() - 1) == "unallocated") {
                    painter.setBrush(QBrush(color));
                    painter.setPen(QPen(color, 3));
                    painter.fillPath(path[m_sizeInfo.size() - 1], color);
                } else {
                    painter.setBrush(QBrush(m_colorInfo[(m_sizeInfo.size() - 1) % (m_colorInfo.size())]));
                    painter.setPen(QPen(QColor(m_colorInfo[(m_sizeInfo.size() - 1) % (m_colorInfo.size())]), 3));
                    painter.fillPath(path[m_sizeInfo.size() - 1], m_colorInfo[(m_sizeInfo.size() - 1) % (m_colorInfo.size())]);
                    m_pathColorInfo.append(m_colorInfo[(m_sizeInfo.size() - 1) % (m_colorInfo.size())]);
                }

                m_reectInfo.append(path[m_sizeInfo.size() - 1].controlPointRect());
            }
        }
    }

    int sumWidth = 0;
    QVector<int> lstWidth;
    QVector<int> lstPathWidth;
    for (int i = 0; i < m_pathList.count(); i++) {
        painter.setFont(DFontSizeManager::instance()->get(DFontSizeManager::T6, QFont::Medium));
        QFontMetrics fmPath = painter.fontMetrics();
        QString textPath = QString(m_pathList.at(i));
        if (m_pathList.at(i) == "unallocated") {
            textPath = QString(tr("Unallocated"));
        } else if (textPath.contains("/dev")) {
            textPath.remove(0, 5);
        }
        int width = fmPath.width(textPath);

        painter.setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8, QFont::Normal));
        QFontMetrics fmSize = painter.fontMetrics();
        QString textSize = QString(m_pathSizeInfo.at(i));
        int widthSize = fmSize.width(textSize);
        int number = 0;

        if (width >= widthSize) {
            number = width + 25;
        } else {
            number = widthSize + 25;
        }

        if (sumWidth + number > rect.width()) {
            sumWidth -= lstPathWidth.last();
            lstWidth.removeLast();
            lstPathWidth.removeLast();
            break;
        } else {
            lstWidth << sumWidth;
            lstPathWidth << number;
            sumWidth += number;
        }
    }

    int width = (rect.width() - sumWidth) / lstWidth.count();
    if (m_pathList.count() > lstWidth.count()) {
        painter.setFont(DFontSizeManager::instance()->get(DFontSizeManager::T6, QFont::Medium));
        QFontMetrics fmEllipsis = painter.fontMetrics();
        QString textEllipsis = QString("...");
        int widthEllipsis = fmEllipsis.width(textEllipsis);

        if (sumWidth + widthEllipsis > rect.width()) {
            sumWidth -= lstPathWidth.last();
            lstWidth.removeLast();
        }

        sumWidth += widthEllipsis;
        width = (rect.width() - sumWidth) / (lstWidth.count() + 1);
    }

    for (int i = 0; i < lstWidth.count(); i++) {
        if (m_pathList.at(i) == "unallocated") {
            int height = 90 - static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            QRect roundRect = QRect(rect.bottomLeft().x() + 2 + lstWidth.at(i) + width * i, rect.bottomLeft().y() - height, 15, 15);
            QPainterPath painterPath;
            painterPath.addRoundedRect(roundRect, 3, 3);
            m_parentPb = DApplicationHelper::instance()->palette(this);
            QBrush brush = DApplicationHelper::instance()->palette(this).dark();
            QColor colorRect = m_parentPb.color(DPalette::Normal, DPalette::Dark);
            painter.setBrush(brush);
            painter.setPen(colorRect);
            painter.drawPath(painterPath);
            painter.fillPath(painterPath, brush);

            QFont font;
            font = DFontSizeManager::instance()->get(DFontSizeManager::T6, QFont::Medium);
            m_parentPb.setColor(DPalette::Text, nameColor);
            QColor textColor = m_parentPb.color(DPalette::Normal, DPalette::Text);
            painter.setFont(font);
            painter.setPen(textColor);
            QTextOption option;
            option.setAlignment(Qt::AlignTop);
            QRect rectText = QRect(paintRect.bottomLeft().x() + 25 + lstWidth.at(i) + width * i, paintRect.bottomLeft().y() + 17, paintRect.width(), 70);
            painter.drawText(rectText, QString(tr("Unallocated")), option);

            m_parentPb.setColor(DPalette::Text, capacityColor);
            QColor text1color = m_parentPb.color(DPalette::Normal, DPalette::Text);
            painter.setPen(text1color);
            font = DFontSizeManager::instance()->get(DFontSizeManager::T8, QFont::Normal);
            painter.setFont(font);
            height = 37 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            QRect textRect1 = QRect(paintRect.bottomLeft().x() + 25 + lstWidth.at(i) + width * i, paintRect.bottomLeft().y() + height, paintRect.width(), 70);
            painter.drawText(textRect1, m_pathSizeInfo.at(i));
        } else {
            int height = 90 - static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            QRect roundRect = QRect(rect.bottomLeft().x() + 2 + lstWidth.at(i) + width * i, rect.bottomLeft().y() - height, 15, 15);
            QPainterPath painterPath;
            painterPath.addRoundedRect(roundRect, 3, 3);
            painter.setBrush(QBrush(m_pathColorInfo.at(i)));
            painter.setPen(m_pathColorInfo.at(i));
            painter.drawPath(painterPath);
            painter.fillPath(painterPath, QBrush(m_pathColorInfo.at(i)));

            QFont font;
            font = DFontSizeManager::instance()->get(DFontSizeManager::T6, QFont::Medium);
            m_parentPb.setColor(DPalette::Text, nameColor);
            QColor textColor = m_parentPb.color(DPalette::Normal, DPalette::Text);
            painter.setFont(font);
            painter.setPen(textColor);
            QTextOption option;
            option.setAlignment(Qt::AlignTop);
            QRect rectText = QRect(paintRect.bottomLeft().x() + 25 + lstWidth.at(i) + width * i, paintRect.bottomLeft().y() + 17, paintRect.width(), 70);
            QString devPath = m_pathList.at(i);
            if (devPath.contains("/dev")) {
                devPath.remove(0, 5);
            }
            painter.drawText(rectText, devPath, option);

            m_parentPb.setColor(DPalette::Text, capacityColor);
            QColor text1color = m_parentPb.color(DPalette::Normal, DPalette::Text);
            painter.setPen(text1color);
            font = DFontSizeManager::instance()->get(DFontSizeManager::T8, QFont::Normal);
            painter.setFont(font);
            height = 37 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            QRect textRect1 = QRect(paintRect.bottomLeft().x() + 25 + lstWidth.at(i) + width * i, paintRect.bottomLeft().y() + height, paintRect.width(), 70);
            painter.drawText(textRect1, m_pathSizeInfo.at(i));
        }
    }

    m_rectEllipsis = rect;
    m_isEllipsis = false;
    m_showNumber = 0;
    if (m_pathList.count() > lstWidth.count()) {
        QFont font;
        font = DFontSizeManager::instance()->get(DFontSizeManager::T6, QFont::Medium);
        m_parentPb.setColor(DPalette::Text, nameColor);
        QColor textColor = m_parentPb.color(DPalette::Normal, DPalette::Text);
        painter.setFont(font);
        painter.setPen(textColor);
        QTextOption option;
        option.setAlignment(Qt::AlignTop);

        QFontMetrics fmEllipsis = painter.fontMetrics();
        QString textEllipsis = QString("...");
        int widthEllipsis = fmEllipsis.width(textEllipsis);

        m_rectEllipsis = QRect(paintRect.bottomLeft().x() + (rect.width() - widthEllipsis), paintRect.bottomLeft().y() + 17, widthEllipsis, 25);
        painter.drawText(m_rectEllipsis, "...", option);

        m_showNumber = lstWidth.count();
        m_isEllipsis = true;
    }

    painter.restore();
}

void VGSizeInfoWidget::onHandleChangeTheme()
{
    m_parentPb = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
}

void VGSizeInfoWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    qreal rectX = m_rectEllipsis.x();
    qreal rectY = m_rectEllipsis.y();
    qreal rectWidth = m_rectEllipsis.width();
    qreal rectHeight = m_rectEllipsis.height();

    bool isInside = false;
    if (m_isEllipsis) {
        if (x > rectX && x < (rectX + rectWidth) && y > rectY && y < (rectY + rectHeight + 10)) {
            isInside = true;
        }
    }

    if (isInside) {
        QList< QMap<QString, QVariant> > lstInfo;
        for (int i = 0; i < m_pathList.count(); i++) {
            if (i >= m_showNumber) {
                QString devPath = m_pathList.at(i);
                if (devPath.contains("/dev")) {
                    devPath.remove(0, 5);
                }

                QMap<QString, QVariant> mapInfo;
                mapInfo["name"] = devPath;
                mapInfo["size"] = m_pathSizeInfo.at(i);
                mapInfo["color"] = m_pathColorInfo.at(i);

                lstInfo << mapInfo;
            }
        }
        emit enterWidget(m_rectEllipsis, lstInfo);
    }

}

void VGSizeInfoWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
//    emit leaveWidget();
}



