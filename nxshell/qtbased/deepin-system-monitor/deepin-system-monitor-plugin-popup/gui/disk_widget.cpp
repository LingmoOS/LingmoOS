// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "disk_widget.h"
#include "../common/utils.h"
#include "common/datacommon.h"
#include "datadealsingleton.h"
#include "dbus/dbuscallmaininterface.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QBrush>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QProcess>

DWIDGET_USE_NAMESPACE

using namespace Utils;
const int pointsNumber = 30;

DiskWidget::DiskWidget(QWidget *parent)
    : QWidget(parent)
{
    m_width = parent->width() - 20;
    setFixedSize(m_width, 154);
    setContentsMargins(0, 0, 0, 0);

    readSpeeds = new QList<qreal>();
    for (int i = 0; i <= pointsNumber; i++) {
        readSpeeds->append(0);
    }

    writeSpeeds = new QList<qreal>();
    for (int i = 0; i <= pointsNumber; i++) {
        writeSpeeds->append(0);
    }

    auto *dAppHelper = DApplicationHelper::instance();
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, &DiskWidget::changeTheme);
    changeTheme(dAppHelper->themeType());

    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &DiskWidget::changeFont);

    connect(&DataDealSingleton::getInstance(), &DataDealSingleton::sigDataUpdate, this, &DiskWidget::updateStatus);
    this->installEventFilter(this);
}

DiskWidget::~DiskWidget() {
    delete readSpeeds;
    delete writeSpeeds;
}

void DiskWidget::updateStatus()
{
    QString diskRead, diskTotalSize, diskWrite, diskAvailable;
    if (!DataDealSingleton::getInstance().readDiskInfo(diskRead, diskTotalSize, diskWrite, diskAvailable))
        return;

    QStringList strs = diskRead.split(" ");
    if (strs.size() == 2)
    {
        m_diskRead = strs.at(0);
        m_diskReadUnit = strs.at(1);
    }
    else
        return;

    // Init download path.
    readSpeeds->append(m_diskRead.toDouble());

    if (readSpeeds->size() > pointsNumber + 1) {
        readSpeeds->pop_front();
    }
    double downloadMaxHeight = *std::max_element(readSpeeds->begin(), readSpeeds->end()) * 1.1;

    strs = diskWrite.split(" ");
    if (strs.size() == 2)
    {
        m_diskWrite = strs.at(0);
        m_diskWriteUnit = strs.at(1);
    }
    else
        return;

    // Init upload path.
    writeSpeeds->append(m_diskWrite.toDouble());

    if (writeSpeeds->size() > pointsNumber + 1) {
        writeSpeeds->pop_front();
    }
    double uploadMaxHeight = *std::max_element(writeSpeeds->begin(), writeSpeeds->end()) * 1.1;

    double maxHeight = qMax(downloadMaxHeight, uploadMaxHeight);

    QPainterPath tmpDownloadpath;
    getPainterPathByData(readSpeeds, tmpDownloadpath, maxHeight);
    readDiskPath = tmpDownloadpath;

    QPainterPath tmpUploadpath;
    getPainterPathByData(writeSpeeds, tmpUploadpath, maxHeight);
    writeDiskPath = tmpUploadpath;


    //zong
    strs = diskTotalSize.split(" ");
    if (strs.size() == 2)
    {
        m_diskTotal = strs.at(0);
        m_diskTotalUnit = strs.at(1);
    }
    else
        return;

    strs = diskAvailable.split(" ");
    if (strs.size() == 2)
    {
        m_diskAvailable = strs.at(0);
        m_diskAvailableUnit = strs.at(1);
    }
//    else
//        return;

    update();
}

void DiskWidget::changeTheme(DApplicationHelper::ColorType themeType)
{
    switch (themeType) {
    case DApplicationHelper::LightType:
        m_titleTrans = Globals::TitleTransLight;
        m_contentTrans = Globals::contentTransLight;
        m_hoverTrans = Globals::hoverTransLight;
        m_icon = QIcon(QString(":/icons/deepin/builtin/light/icon_disk.png"));
        break;
    case DApplicationHelper::DarkType:
        m_titleTrans = Globals::TitleTransDark;
        m_contentTrans = Globals::contentTransDark;
        m_hoverTrans = Globals::hoverTransDark;
        m_icon = QIcon(QString(":/icons/deepin/builtin/dark/icon_disk.png"));
        break;
    default:
        break;
    }

    // init colors
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
#ifndef THEME_FALLBACK_COLOR
    ltextColor = palette.color(DPalette::TextTitle);
#else
    ltextColor = palette.color(DPalette::Text);
#endif

    textColor = palette.color(DPalette::Text);

    textColor = palette.color(DPalette::Text);
    summaryColor = palette.color(DPalette::TextTips);
}

void DiskWidget::getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue)
{
    qreal offsetX = 0;
    qreal distance = (260) * 1.0 / pointsNumber;
    int dataCount = listData->size();

    for (int i = 0;  i < dataCount - 1; i++) {
        QPointF sp = QPointF(offsetX, renderMaxHeight * listData->at(i) / maxVlaue);;
        QPointF ep = QPointF(offsetX + distance, renderMaxHeight * listData->at(i + 1) / maxVlaue);;

        offsetX += distance;

        QPointF c1 = QPointF((sp.x() + ep.x()) / 2.0, sp.y());
        QPointF c2 = QPointF((sp.x() + ep.x()) / 2.0, ep.y());
        path.cubicTo(c1, c2, ep);
    }
}

void DiskWidget::paintEvent(QPaintEvent *e)
{
    setFixedWidth(m_width);
    QPainter painter;
    painter.begin(this);
    painter.setPen(textColor);
    painter.setRenderHint(QPainter::Antialiasing);

    //裁剪绘制区域
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    painter.setClipPath(path);
    if (m_isHover) {
        painter.fillRect(rect(), QBrush(QColor(255, 255, 255, m_hoverTrans)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(255, 255, 255, 0)));
    }

    //标题栏背景
    QRect titleRect(rect().x(), rect().y(), m_width, 36);
    painter.fillRect(titleRect, QBrush(QColor(255, 255, 255, m_titleTrans)));

    QRect contentRect(rect().x(), rect().y()+36, m_width, 118);
    painter.fillRect(contentRect, QBrush(QColor(255, 255, 255,m_contentTrans)));

    //标题
    QString disksTitle = DApplication::translate("Disk.Widget", "Disk");
    painter.setFont(m_sectionFont);
    QFontMetrics fmTitle = painter.fontMetrics();
    int widthTitleTxt = fmTitle.width(disksTitle);
    int heightTitleTxt = fmTitle.descent()+fmTitle.ascent();
    QRect netTitleRect(titleRect.x(), titleRect.y(), widthTitleTxt, heightTitleTxt);
    painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, disksTitle);

    //图标
    int iconSize = 20;
    QRect iconRect(titleRect.x()+(titleRect.width()-widthTitleTxt)/2-iconSize, titleRect.y() + qCeil((titleRect.height() - iconSize) / 2.) + 2,iconSize, iconSize);
    m_icon.paint(&painter, iconRect);

    //分隔符
    int sepHeight = 38;
    painter.setFont(m_contentFont);
    QRect separatorRect1(contentRect.x()+10, contentRect.y()+10, 1, sepHeight);
    painter.fillRect(separatorRect1, QBrush(QColor(0, 0, 0,20)));
    QRect separatorRect2(contentRect.x()+10+87*1, contentRect.y()+10, 1, sepHeight);
    painter.fillRect(separatorRect2, QBrush(QColor(0, 0, 0,20)));
    QRect separatorRect3(contentRect.x()+10+87*2, contentRect.y()+10, 1, sepHeight);
    painter.fillRect(separatorRect3, QBrush(QColor(0, 0, 0,20)));
    QRect separatorRect4(contentRect.x()+10+87*3, contentRect.y()+10, 1, sepHeight);
    painter.fillRect(separatorRect4, QBrush(QColor(0, 0, 0,20)));

    int bulletSize = 6;
    int padleft = 36;
    auto spacing = 20;
    auto margin = 10;

    QString rtagTitle = DApplication::translate("Disk.Widget", "Read");
    QString wtagTitle = DApplication::translate("Disk.Widget", "Write");

    QFontMetrics fmContent(m_contentFont);
    QFontMetrics fmContentUnit(m_contentUnitFont);
    QFontMetrics fmSubContent(m_subContentFont);
    QRect contentDiskRect(padleft, separatorRect1.y() + separatorRect1.height()+margin,
                      rect().x() + rect().width() - padleft, 1);

    //磁盘读取
    QRect readRect(contentDiskRect.x(), contentDiskRect.y(),
                          fmContent.size(Qt::TextSingleLine, m_diskRead).width(), fmContent.height());
    //磁盘读取单位
    QRect readUnitRect(readRect.x()+readRect.width(), contentDiskRect.y()+8,
                              fmContentUnit.size(Qt::TextSingleLine, m_diskReadUnit).width(), fmContent.height());

    //磁盘读取标题
    QRect readTitleRect(contentDiskRect.x(), readRect.y() + readRect.height(),
                        fmContent.size(Qt::TextSingleLine, rtagTitle).width(), fmSubContent.height());
//    //"总计接受"标题
//    QRect totalReceiveTitleRect(totalReceiveRect.x(), recvTitleRect.y(), totalReceiveRect.width(), recvTitleRect.height());


    //"磁盘写入"的值
    QRect writeRect(qCeil((contentDiskRect.width() + spacing) / 2) + spacing, contentDiskRect.y() ,
                          fmContent.size(Qt::TextSingleLine, m_diskWrite).width(), fmContent.height());
    QRect writeUnitRect(writeRect.x()+writeRect.width(), writeRect.y() + 8,
                              fmContentUnit.size(Qt::TextSingleLine, m_diskWriteUnit).width(), fmContent.height());
//    //"总计发送"的值
//    QRect crect32(totalReceiveRect.x(), Sent1Rect.y(), totalReceiveRect.width(), recv1Rect.height());
//    QRect SentTotal1Rect(crect32.x(), crect32.y(),
//                          fmContent.size(Qt::TextSingleLine, m_diskAvailable).width(), fmContent.height());
//    QRect SentTotal2Rect(SentTotal1Rect.x()+SentTotal1Rect.width(), SentTotal1Rect.y()+8,
//                              fmContentUnit.size(Qt::TextSingleLine, m_diskAvailableUnit).width(), fmContent.height());


    //磁盘写入标题writeRect.x(), writeRect.y() + writeRect.height() + 8,
    QRect writeTitleRect(writeRect.x(), writeRect.y() + writeRect.height(),
                  fmContent.size(Qt::TextSingleLine, rtagTitle).width(), fmSubContent.height());
//    QRect sentTotalTitleRect(totalReceiveRect.x(), sentTitleRect.y(), totalReceiveRect.width(), recvTitleRect.height());
    QRectF r1Ind(contentDiskRect.x() - margin, readTitleRect.y() + qCeil((readTitleRect.height() - bulletSize) / 2.), bulletSize,
                 bulletSize);
    QRectF r2Ind(writeTitleRect.x() - margin, writeTitleRect.y() + qCeil((writeTitleRect.height() - bulletSize) / 2.), bulletSize,
                 bulletSize);

    //磁盘读取
//    painter.setPen(ltextColor);
    painter.setFont(m_contentFont);
    painter.drawText(readRect, Qt::AlignLeft | Qt::AlignHCenter,m_diskRead);
    //磁盘读取的单位
//    painter.setPen(ltextColor);
    painter.setFont(m_contentUnitFont);
    painter.drawText(readUnitRect, Qt::AlignLeft | Qt::AlignHCenter,m_diskReadUnit);

    //磁盘写入
//    painter.setPen(ltextColor);
    painter.setFont(m_contentFont);
    painter.drawText(writeRect, Qt::AlignLeft | Qt::AlignVCenter,m_diskWrite);
    //磁盘写入的单位
//    painter.setPen(ltextColor);
    painter.setFont(m_contentUnitFont);
    painter.drawText(writeUnitRect, Qt::AlignLeft | Qt::AlignHCenter, m_diskWriteUnit);

    //标题
    painter.setOpacity(0.6);
    painter.setFont(m_subContentFont);
    painter.drawText(readTitleRect, Qt::AlignLeft | Qt::AlignVCenter,rtagTitle);
    painter.drawText(writeTitleRect, Qt::AlignLeft | Qt::AlignVCenter, wtagTitle);
    painter.setOpacity(1);

    QPainterPath path1, path2;
    path1.addEllipse(r1Ind);
    path2.addEllipse(r2Ind);

    painter.fillPath(path1, m_diskReadColor);
    painter.fillPath(path2, m_diskWriteColor);

    //走势图
    QPainterPath framePath;
    QRect chartRect(separatorRect1.x(), separatorRect1.y(), contentRect.width()-20, 38);
//    framePath.addRect(chartRect);


    QBrush readBrush(m_diskReadColor);
    QBrush writeBrush(m_diskWriteColor);
    qreal networkCurveWidth = 1.2;

    //反锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath clip;
    clip.addRect(chartRect);
    painter.setClipPath(clip);
    painter.translate(chartRect.x() + 2, chartRect.y() + chartRect.height() / 2 - 2);
    painter.scale(1, -1);
    painter.setPen(QPen(readBrush, networkCurveWidth));
    painter.drawPath(readDiskPath);

    painter.translate(0, -5);
    painter.scale(1, -1);
    painter.setPen(QPen(writeBrush, networkCurveWidth));
    painter.drawPath(writeDiskPath);
}

bool DiskWidget::eventFilter(QObject *target, QEvent *event)
{
    if (target == this) {
        if (event->type() == QEvent::Enter) {
            m_isHover = true;
            update();
            return true;
        }
        if (event->type() == QEvent::Leave) {
            m_isHover = false;
            update();
            return true;
        }
    }
    return QWidget::eventFilter(target, event);
}

void DiskWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        DataDealSingleton::getInstance().sendJumpWidgetMessage("MSG_DISK");
        return;
    }
    return QWidget::mouseDoubleClickEvent(event);
}

void DiskWidget::changeFont(const QFont &font)
{
    m_sectionFont = font;
    m_sectionFont.setWeight(QFont::DemiBold);
    m_sectionFont.setPointSizeF(m_sectionFont.pointSizeF());

    m_contentFont = font;
    m_contentFont.setWeight(QFont::Normal);
    m_contentFont.setPointSizeF(Globals::ContentFont);

    m_contentUnitFont = font;
    m_contentUnitFont.setWeight(QFont::Normal);
    m_contentUnitFont.setPointSizeF(Globals::ContentUnitFont);

    m_subContentFont = font;
    m_subContentFont.setWeight(QFont::ExtraLight);
    m_subContentFont.setPointSizeF(Globals::subContentFont);

}
