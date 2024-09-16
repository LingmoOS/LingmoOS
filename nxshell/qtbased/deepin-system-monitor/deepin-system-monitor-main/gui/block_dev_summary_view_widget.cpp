// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "block_dev_summary_view_widget.h"
#include "system/device_db.h"
#include "common/common.h"
#include "system/block_device_info_db.h"
#include "common/thread_manager.h"
#include "system/block_device.h"
#include "common/thread_manager.h"
#include "system/system_monitor_thread.h"
#include "system/block_device_info_db.h"
#include "system/device_db.h"
#include "base/base_detail_item_delegate.h"

#include <QHeaderView>
#include <DApplicationHelper>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <DApplication>
#include <DStyle>
#include <QMap>
#include <QScroller>

using namespace core::system;
using namespace common::format;

#define SUMMARY_CHART_LINE_ALPH 0.13

class DeailTableModelBlock : public QAbstractTableModel
{
public:
    explicit DeailTableModelBlock(QObject *parent = nullptr);
    virtual ~DeailTableModelBlock();
protected:
    int rowCount(const QModelIndex &) const
    {
        return 7;
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const
    {

        if (!index.isValid() || currDeciveName.isEmpty())
            return QVariant();
        int row = index.row();
        int column = index.column();
        if (role == Qt::DisplayRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Model");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Read speed");
                break;
            case 1:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Write speed");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Capacity");
                break;
            case 2:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors read");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Reads/s");
                break;
            case 3:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors read/s");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Reads merged/s");
                break;
            case 4:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors written");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Writes completed ");
                break;
            case 5:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Writes merged");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Writes/s");
                break;
            case 6:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors written/s");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Writes merged/s");
                break;

            }
        } else if (role == Qt::UserRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return m_blockInfo.model();
                else if (column == 1)
                    return formatUnit_memory_disk(m_blockInfo.readSpeed(), B, 1);
                break;
            case 1:
                if (column == 0)
                    return formatUnit_memory_disk(m_blockInfo.writeSpeed(), B, 1);
                else if (column == 1)
                    return formatUnit_memory_disk(m_blockInfo.capacity(), B, 1);
                break;
            case  2:
                if (column == 0)
                    return m_blockInfo.blocksRead();
                else if (column == 1)
                    return m_blockInfo.readRequestIssuedPerSecond();
                break;
            case  3:
                if (column == 0)
                    return m_blockInfo.sectorsReadPerSecond();
                else if (column == 1)
                    return m_blockInfo.readRequestMergedPerSecond();
                break;
            case  4:
                if (column == 0)
                    return m_blockInfo.blocksWritten();
                else if (column == 1)
                    return m_blockInfo.writeComplete();
                break;
            case  5:
                if (column == 0)
                    return m_blockInfo.writeMerged();
                else if (column == 1)
                    return m_blockInfo.writeRequestIssuedPerSecond();
                break;
            case  6:
                if (column == 0)
                    return m_blockInfo.sectorsWrittenPerSecond();
                else if (column == 1)
                    return m_blockInfo.writeRequestMergedPerSecond();
                break;
            }
        } else if (role == Qt::TextColorRole) {
            const auto &palette = DApplicationHelper::instance()->applicationPalette();
            return palette.color(DPalette::Text);
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }

public slots:
    void updateModel();

public:
    void setCurrentName(const QString &str)
    {
        currDeciveName = str;
        if (!str.isEmpty())
            m_blockInfo = m_mapInfo.find(currDeciveName).value();
        updateModel();
    }

private:
    QString currDeciveName;
    BlockDevice m_blockInfo;
    QMap<QString, BlockDevice> m_mapInfo;
};

DeailTableModelBlock::DeailTableModelBlock(QObject *parent): QAbstractTableModel(parent)
{
    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &DeailTableModelBlock::updateModel);
    updateModel();
}

void DeailTableModelBlock::updateModel()
{
    beginResetModel();
    const QList<BlockDevice> &infoDB = DeviceDB::instance()->blockDeviceInfoDB()->deviceList();

    for (int i = 0; i < infoDB.size(); ++i) {
        m_mapInfo.insert(infoDB[i].deviceName(), infoDB[i]);
    }

    if (currDeciveName == "" && !infoDB.isEmpty()) {
        currDeciveName = infoDB[0].deviceName();
        m_blockInfo = infoDB[0];
    } else if (!infoDB.isEmpty()) {
        m_blockInfo = m_mapInfo.find(currDeciveName).value();
    }

    endResetModel();
}

DeailTableModelBlock::~DeailTableModelBlock()
{

}

BlockDevSummaryViewWidget::BlockDevSummaryViewWidget(QWidget *parent)
    : DTableView(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);

    this->horizontalHeader()->setStretchLastSection(true);

    this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->setGridStyle(Qt::NoPen);
    this->setFrameShape(QFrame::NoFrame);

    m_model = new DeailTableModelBlock(this);
    this->setModel(m_model);
    this->setItemDelegate(new BaseDetailItemDelegate(this));

    auto *scroller = QScroller::scroller(viewport());
    auto prop = scroller->scrollerProperties();
    // turn off overshoot to fix performance issue
    prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    prop.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 1);
    // lock scroll direction to fix performance issue
    prop.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1);
    scroller->setScrollerProperties(prop);
    // enable touch gesture
    QScroller::grabGesture(viewport(), QScroller::TouchGesture);

    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
                this, &BlockDevSummaryViewWidget::fontChanged);
}

void BlockDevSummaryViewWidget::chageSummaryInfo(const QString &deviceName)
{
    m_model->setCurrentName(deviceName);
}

void BlockDevSummaryViewWidget::fontChanged(const QFont &font)
{
    m_font = font;
    this->setFont(m_font);
    setFixedHeight(260);
}
void BlockDevSummaryViewWidget::paintEvent(QPaintEvent *event)
{
    DTableView::paintEvent(event);
    QPainter painter(this->viewport());
    painter.setRenderHints(QPainter::Antialiasing);
    const auto &palette = DApplicationHelper::instance()->applicationPalette();
    QColor frameColor = palette.color(DPalette::FrameBorder);
    frameColor.setAlphaF(SUMMARY_CHART_LINE_ALPH);
    painter.setPen(QPen(frameColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(this->horizontalHeader()->sectionSize(0) - 1, 2, this->horizontalHeader()->sectionSize(0) - 1, this->viewport()->height() - 2);
    painter.drawRoundedRect(this->rect().adjusted(1, 1, -1, -1), 6, 6);
}
