// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "weekview.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "units.h"

#include <DWidget>
#include <DHiDPIHelper>

#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QEvent>
#include <QMessageBox>
#include <QWheelEvent>
#include <QtGlobal>
#include <QMouseEvent>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

CWeekView::CWeekView(const GetWeekNumOfYear &getWeekNumOfYear, QWidget *parent)
    : QWidget(parent)
    , m_touchGesture(this)
    , m_weekNumWidget(nullptr)
{
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setMargin(0);
    hBoxLayout->setSpacing(0);

    //上一周按钮
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    m_prevButton->setFixedSize(36, 36);
    connect(m_prevButton, &DIconButton::clicked, this, &CWeekView::signalBtnPrev);

    m_weekNumWidget = new CWeekNumWidget(getWeekNumOfYear, this);
    //下一周按钮
    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    m_nextButton->setFixedSize(36, 36);
    connect(m_nextButton, &DIconButton::clicked, this, &CWeekView::signalBtnNext);

    hBoxLayout->addWidget(m_prevButton);
    hBoxLayout->addWidget(m_weekNumWidget);
    hBoxLayout->addWidget(m_nextButton);
    //设置布局
    setLayout(hBoxLayout);

    connect(m_weekNumWidget, &CWeekNumWidget::signalsSelectDate, this, &CWeekView::signalsSelectDate);
    connect(m_weekNumWidget, &CWeekNumWidget::signalBtnPrev, this, &CWeekView::signalBtnPrev);
    connect(m_weekNumWidget, &CWeekNumWidget::signalBtnNext, this, &CWeekView::signalBtnNext);
}

CWeekView::~CWeekView()
{
}

/**
 * @brief setCurrentDate        设置选择时间，并更新
 * @param date                  时间
 */
void CWeekView::setSelectDate(const QDate date)
{
    m_weekNumWidget->setSelectDate(date);
}

/**
 * @brief CWeekView::setCurrent     设置当前时间
 * @param date
 */
void CWeekView::setCurrent(const QDateTime &dateTime)
{
    m_weekNumWidget->setCurrent(dateTime);
}

/**
 * @brief setTheMe  根据系统主题类型设置颜色
 * @param type      系统主题类型
 */
void CWeekView::setTheMe(int type)
{
    m_weekNumWidget->setTheMe(type);
}

/**
 * @brief wheelEvent 鼠标滚轮切换上一周下一周
 * @param event 鼠标滚轮事件
 */
void CWeekView::wheelEvent(QWheelEvent *event)
{
    bool isDragging = false;
    emit signalIsDragging(isDragging);
    //判断是否是拖拽状态
    if (!isDragging) {
        //左移切换上周,右移切换下周
        if (event->delta() > 0) {
            //上一周
            signalBtnPrev();
        } else {
            //下一周
            signalBtnNext();
        }
    }
}

bool CWeekView::event(QEvent *e)
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
                    //切换下周
                    signalBtnNext();
                    break;
                case touchGestureOperation::T_RIGHT:
                    //切换上周
                    signalBtnPrev();
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
        return DWidget::event(e);
    }
}

CWeekNumWidget::CWeekNumWidget(const GetWeekNumOfYear &getWeekNumOfYear, QWidget *parent)
    : QWidget(parent)
    , m_getWeekNumOfYear(getWeekNumOfYear)
    , m_isFocus(false)
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_dayNumFont.setWeight(QFont::Light);
    setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    //显示周数的widget
    for (int c = 0; c != DDEWeekCalendar::NumWeeksDisplayed; ++c) {
        QWidget *cell = new QWidget;
        //设置事件过滤器
        cell->installEventFilter(this);
        hBoxLayout->addWidget(cell, Qt::AlignTop);
        m_cellList.append(cell);
    }
    this->setLayout(hBoxLayout);
}

CWeekNumWidget::~CWeekNumWidget()
{
}

void CWeekNumWidget::setSelectDate(const QDate date)
{
    m_selectDate = date;
    updateDate();
}

void CWeekNumWidget::setCurrent(const QDateTime &dateTime)
{
    m_currentDate = dateTime;
}

void CWeekNumWidget::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_defaultTextColor = Qt::black;
        m_backgrounddefaultColor = Qt::white;
        m_currentDayTextColor = Qt::white;
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_fillColor = "#FFFFFF";
    } else if (type == 2) {
        m_defaultTextColor = "#C0C6D4";
        m_backgrounddefaultColor = "#FFFFFF";
        m_backgrounddefaultColor.setAlphaF(0.05);
        m_currentDayTextColor = "#B8D3FF";
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_fillColor = "#000000";
        m_fillColor.setAlphaF(0.05);
    }
}

void CWeekNumWidget::resizeEvent(QResizeEvent *event)
{
    //获取当前所有cell的宽度
    const int _allCellWidth = width();
    //获取当前cell的宽度
    int w = _allCellWidth / DDEWeekCalendar::NumWeeksDisplayed;
    //最小显示的宽度
    const int _minWidget = 36;
    //默认都显示
    QVector<bool> vIndex(10, true);
    //cell的宽度小于最小宽度则隐藏部分显示
    if (w < _minWidget) {
        //计算前后需要隐藏的个数
        int t_num = qRound((_minWidget * DDEWeekCalendar::NumWeeksDisplayed - _allCellWidth) / _minWidget / 2.0);
        for (int i = 0; i < t_num; i++) {
            vIndex[i] = false;
            vIndex[9 - i] = false;
        }
    }
    //设置是否显示
    for (int i = 0; i < DDEWeekCalendar::NumWeeksDisplayed; i++) {
        m_cellList[i]->setVisible(vIndex[i]);
    }
    QWidget::resizeEvent(event);
    //更新显示
    update();
}

void CWeekNumWidget::focusInEvent(QFocusEvent *event)
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

void CWeekNumWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    m_isFocus = false;
    update();
}

void CWeekNumWidget::paintCell(QWidget *cell)
{
    const QRect rect(0, 0, cell->width(), cell->height());
    const int pos = m_cellList.indexOf(cell);
    //计算当前日期周数
    const int _showWeekNum = m_getWeekNumOfYear(m_days[pos]);
    const int _currentWeekNum = m_getWeekNumOfYear(m_currentDate.date());

    const bool isCurrentDay = _showWeekNum == _currentWeekNum && m_days[pos].year() == m_currentDate.date().year();
    const bool isSelectDay = m_days[pos].weekNumber() == m_selectDate.weekNumber();

    QPainter painter(cell);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.save();
    painter.setBrush(QBrush(m_fillColor));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect); //画矩形
    painter.restore();
    painter.setPen(Qt::SolidLine);

    const QString dayNum = QString::number(_showWeekNum);

    if (isSelectDay) {
        QRect fillRect((cell->width() - 24) / 2, (cell->height() - 32) / 2 + 4, 24, 24);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(fillRect);

        if (m_isFocus) {
            //绘制焦点获取效果
            QPen pen;
            pen.setWidth(2);
            pen.setColor(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor());
            painter.setPen(pen);
            //在原有的选中效果外面再绘制一圈
            QRectF focusRect(fillRect.x() - 2, fillRect.y() - 2, fillRect.width() + 4, fillRect.height() + 4);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(focusRect);
        }
        painter.restore();
        painter.setPen(m_currentDayTextColor);
        painter.setFont(m_dayNumFont);
        painter.drawText(QRect(0, 0, cell->width(), cell->height()), Qt::AlignCenter, dayNum);

    } else {
        if (isCurrentDay) {
            painter.setPen(m_backgroundcurrentDayColor);
        } else {
            painter.setPen(m_defaultTextColor);
        }
        painter.setFont(m_dayNumFont);
        painter.drawText(QRect(0, 0, cell->width(), cell->height()), Qt::AlignCenter, dayNum);
    }
    painter.end();
}

bool CWeekNumWidget::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);
    if (cell && m_cellList.contains(cell)) {
        const int pos = m_cellList.indexOf(cell);
        //获取每个cell的时间,如果小于1900年则过滤显示和点击操作
        if (!withinTimeFrame( m_days[pos]))
            return false;
        if (e->type() == QEvent::Paint) {
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
            if (mouseEvent->button() == Qt::LeftButton) {
                cellClicked(cell);
            }
        }
    }
    return false;
}

void CWeekNumWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    m_isFocus = false;
}

/**
 * @brief setSelectedCell   设置被选择的周数
 * @param index             周数所在的索引
 */
void CWeekNumWidget::setSelectedCell(int index)
{
    if (m_selectedCell == index)
        return;

    const int prevPos = m_selectedCell;
    m_selectedCell = index;

    m_cellList.at(prevPos)->update();
    m_cellList.at(index)->update();
    m_selectDate = m_days[index];
    const QString dayNum = QString::number(m_getWeekNumOfYear(m_selectDate));
    if (m_days[index].year() < DDECalendar::QueryEarliestYear && dayNum != "1")
        return;
    emit signalsSelectDate(m_days[index]);
}

void CWeekNumWidget::updateDate()
{
    for (int i = 0 ; i < DDEWeekCalendar::NumWeeksDisplayed; ++i) {
        m_days[i]  = m_selectDate.addDays((i - 4) * DDEWeekCalendar::AFewDaysofWeek);
        if (m_days[i] == m_selectDate)
            m_selectedCell = i;
    }
    update();
}

bool CWeekNumWidget::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(e);
        if (focusWidget() == this) {
            //如果焦点在该widget上,可以左右键切换时间
            if (keyEvent->key() == Qt::Key_Left) {
                emit signalBtnPrev();
            }

            if (keyEvent->key() == Qt::Key_Right) {
                emit signalBtnNext();
            }
        }
    }
    return DWidget::event(e);
}

void CWeekNumWidget::cellClicked(QWidget *cell)
{
    const int pos = m_cellList.indexOf(cell);

    if (pos == -1)
        return;

    setSelectedCell(pos);
    update();
}
