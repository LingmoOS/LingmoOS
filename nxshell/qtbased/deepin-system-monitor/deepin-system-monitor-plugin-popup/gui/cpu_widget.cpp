// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ddlog.h"
#include "cpu_widget.h"
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
#include <QTimer>
#include <QProcess>

const int cpuTxtWidth = 96;

DWIDGET_USE_NAMESPACE
using namespace DDLog;
CpuWidget::CpuWidget(QWidget *parent)
    : QWidget(parent)
{
    m_width = parent->width() - 20;
    setFixedSize(m_width, 111);
    setContentsMargins(0, 0, 0, 0);

    downloadSpeeds = new QList<double>();
    for (int i = 0; i <= pointsNumber; i++) {
        downloadSpeeds->append(0);
    }

    auto *dAppHelper = DApplicationHelper::instance();
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, &CpuWidget::changeTheme);
    changeTheme(dAppHelper->themeType());

    initConnection();
    this->installEventFilter(this);
}

CpuWidget::~CpuWidget()
{
    delete downloadSpeeds;
}

void CpuWidget::initConnection()
{
    changeFont(DApplication::font());
    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &CpuWidget::changeFont);

    connect(&DataDealSingleton::getInstance(), &DataDealSingleton::sigDataUpdate, this, &CpuWidget::updateStatus);
}

void CpuWidget::getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue)
{
    qreal offsetX = 0;
    qreal distance = (this->width() - cpuTxtWidth - 10) * 1.0 / pointsNumber;
    int dataCount = listData->size();

    if (maxVlaue < 0.000001)
        maxVlaue = 1;

    for (int i = 0; i < dataCount - 1; i++) {
        //sp 为线段的起始点，ep 为线段的终点
        //c1，c2 为贝塞尔曲线的控制点
        QPointF sp = QPointF(offsetX, 20 * listData->at(i) / maxVlaue);
        ;
        QPointF ep = QPointF(offsetX + distance, 20 * listData->at(i + 1) / maxVlaue);
        ;

        offsetX += distance;

        QPointF c1 = QPointF((sp.x() + ep.x()) / 2.0, sp.y());
        QPointF c2 = QPointF((sp.x() + ep.x()) / 2.0, ep.y());
        path.cubicTo(c1, c2, ep);

        //       qCInfo(app)<<"sp,  x:"<<offsetX<< "Y:"<< 20 * listData->at(i) / maxVlaue;
    }
}

void CpuWidget::updateStatus()
{
    if (!DataDealSingleton::getInstance().readCpuPer(m_cpuPer))
        qCInfo(app) << "false: " << m_cpuPer;

    // Init download path.
    downloadSpeeds->append(m_cpuPer);

    if (downloadSpeeds->size() > pointsNumber + 1) {
        downloadSpeeds->pop_front();
    }
    double downloadMaxHeight = *std::max_element(downloadSpeeds->begin(), downloadSpeeds->end()) * 1.1;

    QPainterPath tmpDownloadpath;
    getPainterPathByData(downloadSpeeds, tmpDownloadpath, downloadMaxHeight);
    downloadPath = tmpDownloadpath;

    update();
}

void CpuWidget::changeTheme(DApplicationHelper::ColorType themeType)
{
    switch (themeType) {
    case DApplicationHelper::LightType:
        m_titleTrans = Globals::TitleTransLight;
        m_contentTrans = Globals::contentTransLight;
        m_hoverTrans = Globals::hoverTransLight;
        m_icon = QIcon(QString(":/icons/deepin/builtin/light/icon_cpu.png"));
        break;
    case DApplicationHelper::DarkType:
        m_titleTrans = Globals::TitleTransDark;
        m_contentTrans = Globals::contentTransDark;
        m_hoverTrans = Globals::hoverTransDark;
        m_icon = QIcon(QString(":/icons/deepin/builtin/dark/icon_cpu.png"));
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
    summaryColor = palette.color(DPalette::TextTips);
}

void CpuWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter;
    painter.begin(this);
    painter.setPen(textColor);
    painter.setRenderHint(QPainter::Antialiasing);
    QFontMetrics fmTitleContent(m_TitleFont);
    QFontMetrics fmContent(m_contentFont);
    QFontMetrics fmContentUnit(m_contentUnitFont);
    QFontMetrics fmSubContent(m_subContentFont);

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
    QRect contentRect(rect().x(), rect().y() + 36, m_width, 75);
    painter.fillRect(contentRect, QBrush(QColor(255, 255, 255, m_contentTrans)));

    //标题
    QString cpuTitle = DApplication::translate("Cpu.Widget", "CPU");
    painter.setFont(m_TitleFont);
    //    QFontMetrics fmTitle = painter.fontMetrics();
    int widthTitle = fmTitleContent.width(cpuTitle);
    int heightTitle = fmTitleContent.descent() + fmTitleContent.ascent();
    QRect cpuTitleRect(titleRect.x(), titleRect.y(), widthTitle, heightTitle);
    painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, cpuTitle);

    //图标
    int iconSize = 20;
    QRect iconRect(titleRect.x() + (titleRect.width() - widthTitle) / 2 - iconSize, titleRect.y() + qCeil((titleRect.height() - iconSize) / 2.) + 2, iconSize, iconSize);
    m_icon.paint(&painter, iconRect);

    //cpu使用率长宽
    painter.setFont(m_contentFont);
    QString cpuStatText = QString::number(m_cpuPer, 'f', 1);
    int widthCpu = fmContent.size(Qt::TextSingleLine, cpuStatText).width();
    int heightCpu = fmContent.descent() + fmContent.ascent();
    //cpu使用率百分号长宽
    int widthCpuper = fmContentUnit.size(Qt::TextSingleLine, "%").width();
    int heightCpuper = fmContentUnit.descent() + fmContentUnit.ascent();

    //写cpu使用率和百分号
    QRect cpuUsageRect(contentRect.x() + (cpuTxtWidth - widthCpu - widthCpuper) / 2, contentRect.y() + 10, widthCpu, heightCpu);
    painter.drawText(cpuUsageRect, Qt::AlignLeft | Qt::AlignBottom, cpuStatText);

    painter.setFont(m_contentUnitFont);
    QRect cpuperUsageRect(cpuUsageRect.x() + cpuUsageRect.width(), cpuUsageRect.y() + heightCpu / 4, widthCpuper, heightCpuper);
    painter.drawText(cpuperUsageRect, Qt::AlignLeft | Qt::AlignBottom, "%");

    //总使用率 文本
    painter.setOpacity(0.6);
    painter.setFont(m_subContentFont);
    int widthcpuText = fmSubContent.size(Qt::TextSingleLine, tr("Utilization")).width();
    QRect cpuTextRect(contentRect.x() + (cpuTxtWidth - widthcpuText) / 2, cpuUsageRect.y() + cpuUsageRect.height(),
                      fmSubContent.size(Qt::TextSingleLine, tr("Utilization")).width(), fmSubContent.height());
    painter.drawText(cpuTextRect, Qt::AlignLeft | Qt::AlignTop, tr("Utilization"));
    painter.setOpacity(1);

    //分隔符
    int sepheight = 50;
    int sepwidth = (contentRect.width() - 92 - 10) / 2;
    int rightMax = contentRect.x() + contentRect.width() - 10;
    painter.setFont(m_contentFont);
    QRect separatorRect1(contentRect.x() + cpuTxtWidth, contentRect.y() + 10, 1, sepheight);
    painter.fillRect(separatorRect1, QBrush(QColor(0, 0, 0, 20)));
    QRect separatorRect2(contentRect.x() + cpuTxtWidth + sepwidth, contentRect.y() + 10, 1, sepheight);
    painter.fillRect(separatorRect2, QBrush(QColor(0, 0, 0, 20)));
    QRect separatorRect3(rightMax, contentRect.y() + 10, 1, sepheight);
    painter.fillRect(separatorRect3, QBrush(QColor(0, 0, 0, 20)));

    //走势图
    QPainterPath framePath;
    QRect chartRect(separatorRect1.x(), separatorRect1.y(), contentRect.width() - cpuTxtWidth - 10, sepheight * 2);
    //    framePath.addRect(chartRect);

    QColor Color { "#004EEF" };
    QBrush recvBrush(Color);
    qreal networkCurveWidth = 1.2;

    QPainterPath clip;
    clip.addRect(chartRect);
    painter.setClipPath(clip);
    painter.setRenderHint(QPainter::Antialiasing, true);   //反锯齿

    painter.translate(chartRect.x() + 2, chartRect.y() + chartRect.height() / 2 - 2);
    painter.scale(1, -1);
    painter.setPen(QPen(recvBrush, networkCurveWidth));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPath(downloadPath);
}

bool CpuWidget::eventFilter(QObject *target, QEvent *event)
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

void CpuWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        DataDealSingleton::getInstance().sendJumpWidgetMessage("MSG_CPU");
        return;
    }
    return QWidget::mouseDoubleClickEvent(event);
}

void CpuWidget::changeFont(const QFont &font)
{
    m_TitleFont = font;
    m_TitleFont.setWeight(QFont::DemiBold);
    m_TitleFont.setPointSizeF(m_TitleFont.pointSizeF());

    m_contentFont = font;
    m_contentFont.setWeight(QFont::Normal);
    m_contentFont.setPointSizeF(Globals::ContentFont + 3);

    m_contentUnitFont = font;
    m_contentUnitFont.setWeight(QFont::Normal);
    m_contentUnitFont.setPointSizeF(Globals::ContentUnitFont + 3);

    m_subContentFont = font;
    m_subContentFont.setWeight(QFont::ExtraLight);
    m_subContentFont.setPointSizeF(Globals::subContentFont);
}
