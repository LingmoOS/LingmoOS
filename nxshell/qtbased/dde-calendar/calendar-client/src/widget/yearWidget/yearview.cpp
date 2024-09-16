// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "yearview.h"
#include "constants.h"
#include "configsettings.h"

#include <DHiDPIHelper>
#include <DPalette>

#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QLocale>
#include <QMouseEvent>
#include <QPainterPath>
#include <QtMath>
#include <QStyleOptionFocusRect>

DGUI_USE_NAMESPACE

CYearView::CYearView(QWidget *parent)
    : CustomFrame(parent)
{
    //设置焦点类型
    setFocusPolicy(Qt::FocusPolicy::TabFocus);
    setMouseTracking(true);
    //add separator line
    m_currentMouth = new CustomFrame();
    m_currentMouth->setObjectName("currentMouth");
    m_currentMouth->setFixedHeight(24);
    m_currentMouth->setContentsMargins(0, 0, 0, 0);
    m_currentMouth->setTextAlign(Qt::AlignLeft);

    m_momthFont.setWeight(QFont::Medium);
    m_momthFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_currentMouth->setTextFont(m_momthFont);

    QHBoxLayout *separatorLineLayout = new QHBoxLayout;
    separatorLineLayout->setMargin(0);
    separatorLineLayout->setSpacing(0);
    separatorLineLayout->setContentsMargins(0, 0, 0, 0);
    separatorLineLayout->addWidget(m_currentMouth);
    separatorLineLayout->addStretch();

    m_currentMouth->show();
    m_currentMouth->installEventFilter(this);

    m_weekWidget = new CWeekWidget(this);
//    m_weekWidget->setMinimumHeight(20);

    m_monthView = new MonthBrefWidget(this);
    connect(m_monthView,
            &MonthBrefWidget::signalPressDate,
            this,
            &CYearView::slotPressClickDate);
    connect(m_monthView,
            &MonthBrefWidget::signalDoubleClickDate,
            this,
            &CYearView::slotDoubleClickDate);

    m_hhLayout = new QVBoxLayout;
    m_hhLayout->addLayout(separatorLineLayout);
    m_hhLayout->addWidget(m_weekWidget, 1);
    m_hhLayout->addWidget(m_monthView, 6);
    m_hhLayout->setMargin(0);
    m_hhLayout->setSpacing(0);
    m_hhLayout->setContentsMargins(13, 10, 10, 10);
    setLayout(m_hhLayout);
}

/**
 * @brief slotDoubleClickDate   隐藏日程浮框，并发出鼠标双击的信号
 * @param date                  传递时间参数
 */
void CYearView::slotDoubleClickDate(const QDate &date)
{
    emit signalMousePress(date, 1);
}

/**
 * @brief slotPressClickDate    鼠标单击日期，显示日程浮框
 * @param date                  传递日程参数
 */
void CYearView::slotPressClickDate(const QDate &date)
{
    emit signalMousePress(date, 0);
}

/**
 * @brief setTheMe      根据系统主题类型，设置颜色
 * @param type          系统主题类型
 */
void CYearView::setTheMe(int type)
{
    m_themetype = type;

    if (type == 0 || type == 1) {
        m_bnormalColor = Qt::white;
        m_currentMouth->setTextColor(QColor("#CF0059"));

    } else if (type == 2) {
        m_bnormalColor = "#FFFFFF";
        m_bnormalColor.setAlphaF(0.05);
        m_currentMouth->setTextColor(QColor("#BF1D63"));
    }
    QColor monthcolor = Qt::white;
    monthcolor.setAlphaF(0);
    m_currentMouth->setBColor(monthcolor);
}

void CYearView::setShowMonthDate(const QDate &showMonth)
{
    m_showMonthDate = QDate(showMonth.year(), showMonth.month(), 1);
    m_currentMouth->setTextStr(QLocale::system().monthName(m_showMonthDate.month(), QLocale::ShortFormat));
    m_monthView->setShowMonthDate(m_showMonthDate);
}

//获取显示的月
QDate CYearView::getShowMonthDate()
{
    return m_showMonthDate;
}

/**
 * @brief CYearView::setHasScheduleSet
 * @param hasScheduleSet
 * 设置含有日程的集合
 */
void CYearView::setHasScheduleSet(const QSet<QDate> &hasScheduleSet)
{
    m_monthView->setHasScheduleDateSet(hasScheduleSet);
}

void CYearView::setHasSearchScheduleSet(const QSet<QDate> &hasScheduleSet)
{
    m_monthView->setHasSearchScheduleSet(hasScheduleSet);
}

/**
 * @brief CYearView::getStartAndStopDate    获取这个月开始结束时间
 * @param startDate
 * @param stopDate
 * @return
 */
bool CYearView::getStartAndStopDate(QDate &startDate, QDate &stopDate)
{
    startDate = m_showMonthDate;
    stopDate = m_showMonthDate.addMonths(1);
    stopDate = stopDate.addDays(-1);
    return true;
}

/**
 * @brief eventFilter 过滤器，双击年视图下的月份跳转到月视图。
 * @param o 事件对象
 * @param e 事件类型
 * @return false
 */
bool CYearView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);

    if (cell == m_currentMouth) {
        if (e->type() == QEvent::MouseButtonDblClick) {
            emit signalMousePress(m_showMonthDate, 2);
        }
    }
    return false;
}

/**
 * @brief resizeEvent 更新月份框的高度
 * @param event 窗口大小调整事件
 */
void CYearView::resizeEvent(QResizeEvent *event)
{
    int leftmagin = qFloor(width() * 0.06435 + 0.5);
    int rightmagin = leftmagin;
    int topmagin = qFloor(height() * 0.02955 + 0.5);
    int buttonmagin = qFloor(height() * 0.044 + 0.5);

    m_hhLayout->setContentsMargins(leftmagin, topmagin, rightmagin, buttonmagin);
    m_momthFont.setPixelSize(qFloor(16 + (height() - 159) / 16.75));
    m_currentMouth->setTextFont(m_momthFont);
    m_currentMouth->setFixedHeight(24 + (height() - 159) / 12);
    m_currentMouth->update();
    QFrame::resizeEvent(event);
}

/**
 * @brief paintEvent 绘制每个月的背景
 * @param e 绘图事件
 */
void CYearView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width() - 2 * m_borderframew;
    int labelheight = height() - 2 * m_borderframew;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(QBrush(m_bnormalColor));
    painter.setPen(Qt::NoPen);
    QPainterPath painterPath;
    painterPath.moveTo(m_radius, m_borderframew);
    painterPath.arcTo(QRect(m_borderframew, m_borderframew, m_radius * 2, m_radius * 2), 90, 90);

    painterPath.lineTo(0, labelheight - m_radius);
    painterPath.arcTo(QRect(m_borderframew, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);

    painterPath.lineTo(labelwidth - m_radius, labelheight);
    painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);

    painterPath.lineTo(labelwidth, m_radius);

    painterPath.arcTo(QRect(labelwidth - m_radius * 2, m_borderframew, m_radius * 2, m_radius * 2), 0, 90);
    painterPath.lineTo(m_radius, m_borderframew);
    painterPath.closeSubpath();
    painter.drawPath(painterPath);

    if (hasFocus()) {
        //有焦点，绘制焦点
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Background);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}
