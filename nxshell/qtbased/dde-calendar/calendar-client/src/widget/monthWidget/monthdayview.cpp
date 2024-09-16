// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthdayview.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "units.h"

#include <DPalette>
#include <DHiDPIHelper>

#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QKeyEvent>

DGUI_USE_NAMESPACE
CMonthDayView::CMonthDayView(QWidget *parent)
    : DFrame(parent)
    , m_touchGesture(this)
{
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setMargin(0);
    hBoxLayout->setSpacing(0);
    hBoxLayout->setContentsMargins(10, 0, 10, 0);
    m_monthWidget = new CMonthWidget(this);
    hBoxLayout->addWidget(m_monthWidget);
    setLayout(hBoxLayout);
    setFrameRounded(true);
    setLineWidth(0);
    setWindowFlags(Qt::FramelessWindowHint);
    connect(m_monthWidget, &CMonthWidget::signalsSelectDate, this, &CMonthDayView::signalsSelectDate);
}

CMonthDayView::~CMonthDayView()
{
}

/**
 * @brief CMonthDayView::setSelectDate  设置选择时间
 * @param date
 */
void CMonthDayView::setSelectDate(const QDate &date)
{
    m_selectDate = date;
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_days[i] = m_selectDate.addMonths(i - 5);
    }
    m_monthWidget->setDate(m_days);
    update();
}

/**
 * @brief CMonthDayView::setTheMe   设置主题颜色
 * @param type
 */
void CMonthDayView::setTheMe(int type)
{
    QColor frameColor;
    if (type == 0 || type == 1) {
        frameColor = "#FFFFFF";
    } else if (type == 2) {
        frameColor = "#FFFFFF";
        frameColor.setAlphaF(0.05);
    }
    DPalette aniPa = palette();
    aniPa.setColor(DPalette::Background, frameColor);
    setPalette(aniPa);
    setBackgroundRole(DPalette::Background);
    CMonthRect::setTheMe(type);
}

void CMonthDayView::setSearchflag(bool flag)
{
    m_searchFlag = flag;
}

void CMonthDayView::wheelEvent(QWheelEvent *e)
{
    //如果滚动为左右则触发信号
    if (e->orientation() == Qt::Orientation::Horizontal) {
        emit signalAngleDelta(e->angleDelta().x());
    } else {
        emit signalAngleDelta(e->angleDelta().y());
    }
}

bool CMonthDayView::event(QEvent *e)
{
    if (m_touchGesture.event(e)) {
        //获取触摸状态
        switch (m_touchGesture.getTouchState()) {
        case touchGestureOperation::T_SLIDE: {
            //在滑动状态如果可以更新数据则切换月份
            if (m_touchGesture.isUpdate()) {
                m_touchGesture.setUpdate(false);
                switch (m_touchGesture.getMovingDir()) {
                case touchGestureOperation::T_LEFT:
                    emit signalAngleDelta(-1);
                    break;
                case touchGestureOperation::T_RIGHT:
                    emit signalAngleDelta(1);
                    break;
                default:
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
        return true;
    } else {
        return DFrame::event(e);
    }
}
/**
 * @brief CMonthWidget 构造函数
 * @param parent 父类
 */
CMonthWidget::CMonthWidget(QWidget *parent)
    : QWidget(parent)
    , m_isFocus(false)
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        CMonthRect *monthrect = new CMonthRect(this);
        m_MonthItem.append(monthrect);
    }
    //获取Tab焦点
    setFocusPolicy(Qt::StrongFocus);
}

CMonthWidget::~CMonthWidget()
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        CMonthRect *monthrect = m_MonthItem.at(i);
        delete  monthrect;
    }

    m_MonthItem.clear();
}

void CMonthWidget::setDate(const QDate date[12])
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_MonthItem.at(i)->setDate(date[i]);
    }
    CMonthRect::setSelectRect(m_MonthItem.at(5));
    update();
}

void CMonthWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateSize();
}

void CMonthWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸转换则设置触摸状态和触摸开始坐标
        m_touchState = 1;
        m_touchBeginPoint = event->pos();
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::RightButton)
        return;
    m_isFocus = false;
    mousePress(event->pos());
}

void CMonthWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    for (int i = 0; i < m_MonthItem.size(); ++i) {
        m_MonthItem.at(i)->paintItem(&painter, m_MonthItem.at(i)->rect(), m_isFocus);
    }
}

void CMonthWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        if (m_touchState == 1) {
            //如果为触摸且状态为点击则为触摸点击
            mousePress(event->pos());
            m_touchState = 0;
        }
        QWidget::mouseReleaseEvent(event);
    }
}

void CMonthWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        QPoint currentPoint = event->pos();
        //如果移动距离大与5则为触摸移动状态
        if (QLineF(m_touchBeginPoint, currentPoint).length() > 5) {
            m_touchState = 2;
        }
        QWidget::mouseMoveEvent(event);
    }
}

void CMonthWidget::keyPressEvent(QKeyEvent *event)
{
    //获取当前选择时间
    QDate selectDate = CMonthRect::getSelectRect()->getDate();
    //初始化需要设置的时间
    QDate setdate = selectDate;
    switch (event->key()) {
    case Qt::Key_Left: {
        setdate = selectDate.addMonths(-1);
    } break;
    case Qt::Key_Right: {
        setdate = selectDate.addMonths(1);
    } break;
    default:
        QWidget::keyPressEvent(event);
    }
    if (selectDate != setdate) {
        //更新时间
        updateShowDate(setdate);
        //设置更新后的时间并更新界面
        setDate(m_days);
        emit signalsSelectDate(setdate);
    }
}

void CMonthWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    switch (event->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
    case Qt::ActiveWindowFocusReason:
        m_isFocus = true;
        break;
    default:
        m_isFocus = false;
        break;
    };
    update();
}

void CMonthWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    m_isFocus = false;
    update();
}

void CMonthWidget::mousePress(const QPoint &point)
{
    int itemindex = getMousePosItem(point);
    if (!(itemindex < 0)) {
        if (!withinTimeFrame(m_MonthItem.at(itemindex)->getDate())) {
            return;
        }
        CMonthRect::setSelectRect(m_MonthItem.at(itemindex));
        emit signalsSelectDate(m_MonthItem.at(itemindex)->getDate());
    }
    update();
}
/**
 * @brief CMonthWidget::updateSize  更新item大小
 */
void CMonthWidget::updateSize()
{
    qreal w = this->width() / m_MonthItem.size();
    for (int i = 0; i < m_MonthItem.size(); ++i) {
        m_MonthItem.at(i)->setRect(i * w, 0, w, this->height());
    }
    update();
}

/**
 * @brief CMonthWidget::getMousePosItem     获取鼠标点击的区域编号
 * @param pos
 * @return
 */
int CMonthWidget::getMousePosItem(const QPointF &pos)
{
    int res = -1;

    for (int i = 0 ; i < m_MonthItem.size(); ++i) {
        if (m_MonthItem.at(i)->rect().contains(pos)) {
            res = i;
            break;
        }
    }

    return res;
}

/**
 * @brief CMonthWidget::updateShowDate  根据选择时间更新显示的月份
 * @param selectDate
 */
void CMonthWidget::updateShowDate(const QDate &selectDate)
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumOfYear; ++i) {
        m_days[i] = selectDate.addMonths(i - 5);
    }
}

int         CMonthRect::m_themetype ;
qreal       CMonthRect::m_DevicePixelRatio;

QColor      CMonthRect::m_defaultTextColor;
QColor      CMonthRect::m_backgrounddefaultColor ;
QColor      CMonthRect::m_currentDayTextColor;
QColor      CMonthRect::m_backgroundcurrentDayColor;
QColor      CMonthRect::m_fillColor;
QFont       CMonthRect::m_dayNumFont;
CMonthRect         *CMonthRect::m_SelectRect = nullptr;

CMonthRect::CMonthRect(QWidget *parent)
    : m_parentWidget(parent)
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_dayNumFont.setWeight(QFont::Light);
}

/**
 * @brief CMonthRect::setDate   设置时间
 * @param date
 */
void CMonthRect::setDate(const QDate &date)
{
    m_Date = date;
}

/**
 * @brief CMonthRect::getDate   获取时间
 * @return
 */
QDate CMonthRect::getDate() const
{
    return  m_Date;
}

/**
 * @brief CMonthRect::rect      获取矩阵大小
 * @return
 */
QRectF CMonthRect::rect() const
{
    return  m_rect;
}

/**
 * @brief CMonthRect::setRect     设置矩阵大小
 * @param rect
 */
void CMonthRect::setRect(const QRectF &rect)
{
    m_rect = rect;
}

/**
 * @brief CMonthRect::setRect       设置矩阵大小
 * @param x
 * @param y
 * @param w
 * @param h
 */
void CMonthRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rect.setRect(x, y, w, h);
}

/**
 * @brief CMonthRect::paintItem     绘制
 * @param painter
 * @param rect
 */
void CMonthRect::paintItem(QPainter *painter, const QRectF &rect, bool drawFocus)
{
    m_selectColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();

    if ( !withinTimeFrame(m_Date))
        return;
    const bool isCurrentDay = (m_Date.month() == QDate::currentDate().month()
                               && m_Date.year() == QDate::currentDate().year());

    painter->setPen(Qt::SolidLine);

    const QString dayNum = QString::number(m_Date.month());

    if (m_SelectRect == this) {
        QRectF fillRect((rect.width() - 36) / 2 + rect.x() + 6,
                        (rect.height() - 36) / 2 + 7 + rect.y(),
                        24,
                        24);
        painter->setBrush(QBrush(m_selectColor));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(fillRect);
        //如果有焦点，绘制tab选中效果
        if (drawFocus) {
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_selectColor);
            painter->setPen(pen);
            //在原有的选中效果外面再绘制一圈
            QRectF focusRect(fillRect.x() - 2, fillRect.y() - 2, fillRect.width() + 4, fillRect.height() + 4);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(focusRect);
        }
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        painter->setPen(m_currentDayTextColor);
        painter->setFont(m_dayNumFont);
        painter->drawText(rect, Qt::AlignCenter, dayNum);
    } else {
        if (isCurrentDay) {
            painter->setPen(m_backgroundcurrentDayColor);
        } else {
            painter->setPen(m_defaultTextColor);
        }
        painter->setFont(m_dayNumFont);
        painter->drawText(rect, Qt::AlignCenter, dayNum);
    }
}

/**
 * @brief CMonthRect::setDevicePixelRatio       设置设备缩放比例
 * @param pixel
 */
void CMonthRect::setDevicePixelRatio(const qreal pixel)
{
    m_DevicePixelRatio = pixel;
}

/**
 * @brief CMonthRect::setTheMe  设置主题
 * @param type
 */
void CMonthRect::setTheMe(int type)
{
    m_themetype = type;
    QColor frameColor;

    if (type == 0 || type == 1) {
        m_defaultTextColor = Qt::black;
        m_backgrounddefaultColor = Qt::white;
        m_currentDayTextColor = Qt::white;
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_fillColor = "#FFFFFF";
        frameColor = m_fillColor;
        m_fillColor.setAlphaF(0);
    } else if (type == 2) {
        m_defaultTextColor = "#C0C6D4";
        QColor framecolor = Qt::black;
        framecolor.setAlphaF(0.5);
        m_backgrounddefaultColor = framecolor;
        m_currentDayTextColor = "#C0C6D4";
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_fillColor = "#FFFFFF";
        m_fillColor.setAlphaF(0.05);
        frameColor = m_fillColor;
        m_fillColor.setAlphaF(0);
    }
}

/**
 * @brief CMonthRect::setSelectRect  设置选择的矩阵
 * @param selectRect
 */
void CMonthRect::setSelectRect(CMonthRect *selectRect)
{
    m_SelectRect = selectRect;
}

/**
 * @brief CMonthRect::getSelectRect 获取选择的矩阵
 * @return
 */
CMonthRect *CMonthRect::getSelectRect()
{
    return m_SelectRect;
}
