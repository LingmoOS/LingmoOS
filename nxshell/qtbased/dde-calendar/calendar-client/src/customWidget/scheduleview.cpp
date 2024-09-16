// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleview.h"
#include "alldayeventview.h"
#include "graphicsview.h"
#include "scheduledlg.h"
#include "schedulecoormanage.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "calendarmanage.h"
#include "calendarglobalenv.h"

#include <DPalette>
#include <DHorizontalLine>
#include <QPainterPath>

#include <QGridLayout>
#include <QShortcut>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

DGUI_USE_NAMESPACE

static int hourTextWidth = 50;
static int hourTextHeight = 20;
CScheduleView::CScheduleView(QWidget *parent, ScheduleViewPos viewType)
    : DFrame(parent)
    , m_viewPos(viewType)
    , m_touchGesture(this)
    , m_timeFormat(CalendarManager::getInstance()->getTimeFormat())
{
    initUI();
    initConnection();
    setLineWidth(0);
    setFocusPolicy(Qt::TabFocus);
}

CScheduleView::~CScheduleView()
{
}

void CScheduleView::setviewMargin(int left, int top, int right, int bottom)
{
    m_leftMargin = left;
    m_topMargin = top;
    m_rightMargin = right;
    m_layout->setContentsMargins(left, 0, 0, 0);
    m_graphicsView->setMargins(0, 0, right, bottom);
    m_alldaylist->setMargins(0, 0, 0, 0);
}

void CScheduleView::setRange(int w, int h, QDate begin, QDate end)
{
    Q_UNUSED(h);

    if (!(w > 0)) {
        return;
    }
    m_beginDate = begin;
    m_endDate = end;
    m_TotalDay = begin.daysTo(end) + 1;
    m_graphicsView->setRange(w, scheduleViewHeight(), begin, end, m_rightMargin);
    m_alldaylist->setRange(w, 22, m_beginDate, m_endDate, m_rightMargin);

    if (m_viewPos == ScheduleViewPos::DayPos)
        m_currteDate = begin;
    update();
}

void CScheduleView::setRange(QDate begin, QDate end)
{
    m_TotalDay = begin.daysTo(end) + 1;
    m_graphicsView->setRange(begin, end);
    m_alldaylist->setRange(begin, end);
    m_beginDate = begin;
    m_endDate = end;
    updateSchedule();
}

void CScheduleView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_linecolor = "#000000";
        m_linecolor.setAlphaF(0.1);
        m_ALLDayColor = "#303030";
        m_timeColor = "#7D7D7D";
    } else if (type == 2) {
        m_linecolor = "#000000";
        m_linecolor.setAlphaF(0.1);
        m_ALLDayColor = "#7D7D7D";
        m_timeColor = "#7D7D7D";
    }
    DPalette _painte;
    //获取外框背景色
    m_outerBorderColor = _painte.color(QPalette::Active, QPalette::Window);
    m_graphicsView->setTheMe(type);
    m_alldaylist->setTheMe(type);
    update();
}

void CScheduleView::setLunarVisible(bool state)
{
    Q_UNUSED(state);
}

void CScheduleView::setTime(QTime time)
{
    m_graphicsView->setTime(time);
}

void CScheduleView::setSelectSchedule(const DSchedule::Ptr &scheduleInfo)
{
    if (scheduleInfo->allDay()) {
        m_alldaylist->setSelectSearchSchedule(scheduleInfo);
    } else {
        m_graphicsView->setSelectSearchSchedule(scheduleInfo);
    }
}

void CScheduleView::updateHeight()
{
    m_graphicsView->updateHeight();
    m_alldaylist->updateHeight();
}

bool CScheduleView::IsDragging()
{
    return (m_graphicsView->getDragStatus() != 4) || (m_alldaylist->getDragStatus() != 4);
}

void CScheduleView::setCurrentDate(const QDateTime &currentDate)
{
    m_graphicsView->setCurrentDate(currentDate);
}

/**
 * @brief CScheduleView::setShowScheduleInfo        设置显示日程
 * @param scheduleInfo
 */
void CScheduleView::setShowScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo)
{
    m_showSchedule = scheduleInfo;
    updateSchedule();
}

/**
 * @brief CScheduleView::setTimeFormat 设置日期显示格式
 * @param timeformat 日期格式
 */
void CScheduleView::setTimeFormat(QString timeformat)
{
    m_timeFormat = timeformat;
    m_ScheduleRemindWidget->setTimeFormat(timeformat);
}

void CScheduleView::setDate(QDate date)
{
    m_currteDate = date;
    updateAllday();
}

void CScheduleView::slotupdateSchedule()
{
    updateSchedule();
}

void CScheduleView::slotPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType)
{
    m_vHours = vHours;
    m_vPos = vPos;
    m_currentTimeType = currentTimeType;
    update();
}

void CScheduleView::paintEvent(QPaintEvent *event)
{
    DFrame::paintEvent(event);
    QPainter painter(this);
    font.setWeight(QFont::Normal);
    font.setPixelSize(DDECalendar::FontSizeEleven);
    if (m_vPos.isEmpty())
        return;
    QLocale locale;

    if (locale.language() == QLocale::Chinese) {
        QRect tinrect((m_leftMargin - hourTextWidth) / 2 - 5,
                      m_topMargin - 8 + m_vPos[m_vPos.count() - 1], hourTextWidth, hourTextHeight);

        if (m_currentTimeType == 0) {
            painter.save();
            painter.setFont(font);
            painter.setPen(m_timeColor);
            for (int i = 0; i < m_vPos.size(); i++) {
                if (m_vHours[i] == 0)
                    continue;
                if (m_vHours[i] == 24)
                    continue;
                if (m_topMargin - 8 + m_vPos[i] < m_topMargin)
                    continue;
                painter.drawText(
                    QRect((m_leftMargin - hourTextWidth) / 2 - 5, m_topMargin - 8 + m_vPos[i],
                          hourTextWidth, hourTextHeight),
                    Qt::AlignCenter, QTime(m_vHours[i], 0).toString(m_timeFormat.contains("AP") ? "AP h 时" : m_timeFormat));
            }
            painter.restore();
        } else {
            painter.save();
            painter.setFont(font);
            painter.setPen(m_timeColor);

            for (int i = 0; i < m_vPos.size() - 1; i++) {
                if (m_vHours[i] == 0)
                    continue;
                if (m_vHours[i] == 24)
                    continue;
                if (m_topMargin - 8 + m_vPos[i] < m_topMargin)
                    continue;
                QRect rr((m_leftMargin - hourTextWidth) / 2 - 5, m_topMargin - 8 + m_vPos[i],
                         hourTextWidth, hourTextHeight);
                if (rr.intersects(tinrect) && m_viewPos == ScheduleViewPos::DayPos && m_beginDate == QDate::currentDate()) {
                    continue;
                }
                painter.drawText(
                    QRect((m_leftMargin - hourTextWidth) / 2 - 5, m_topMargin - 8 + m_vPos[i],
                          hourTextWidth, hourTextHeight),
                    Qt::AlignCenter, QTime(m_vHours[i], 0).toString(m_timeFormat.contains("AP") ? "AP h 时" : m_timeFormat));
            }
            painter.restore();

            if (m_viewPos == ScheduleViewPos::DayPos && m_beginDate == QDate::currentDate()) {
                painter.save();
                painter.setFont(font);
                painter.setPen(m_currenttimecolor);
                QString str = QTime::currentTime().toString(m_timeFormat);
                painter.drawText(QRect((m_leftMargin - hourTextWidth) / 2 - 5,
                                       m_topMargin - 8 + m_vPos[m_vPos.count() - 1], hourTextWidth + 1,
                                       hourTextHeight),
                                 Qt::AlignCenter, str);
                painter.restore();
            }
        }
    } else {
        if (m_currentTimeType == 0) {
            painter.save();
            painter.setFont(font);
            painter.setPen(m_timeColor);

            for (int i = 0; i < m_vPos.size(); i++) {
                if (m_vHours[i] == 0)
                    continue;
                if (m_vHours[i] == 24)
                    continue;
                if (m_topMargin - 8 + m_vPos[i] < m_topMargin)
                    continue;
                painter.drawText(
                    QRect((m_leftMargin - hourTextWidth) / 2 - 5, m_topMargin - 8 + m_vPos[i],
                          hourTextWidth + 2, hourTextHeight),
                    Qt::AlignCenter, QTime(m_vHours[i], 0).toString(m_timeFormat));
            }
            painter.restore();
        } else {
            painter.save();
            painter.setFont(font);
            painter.setPen(m_timeColor);
            QRect tinrect((m_leftMargin - hourTextWidth) / 2 - 5,
                          m_topMargin - 8 + m_vPos[m_vPos.count() - 1], hourTextWidth + 2,
                          hourTextHeight);

            for (int i = 0; i < m_vPos.size() - 1; i++) {
                if (m_vHours[i] == 0)
                    continue;
                if (m_vHours[i] == 24)
                    continue;
                if (m_topMargin - 8 + m_vPos[i] < m_topMargin)
                    continue;
                QRect rr((m_leftMargin - hourTextWidth) / 2 - 5, m_topMargin - 8 + m_vPos[i],
                         hourTextWidth + 2, hourTextHeight);

                if (rr.intersects(tinrect) && m_viewPos == ScheduleViewPos::DayPos && m_beginDate == QDate::currentDate())
                    continue;

                painter.drawText(
                    QRect((m_leftMargin - hourTextWidth) / 2 - 5, m_topMargin - 8 + m_vPos[i],
                          hourTextWidth + 2, hourTextHeight),
                    Qt::AlignCenter, QTime(m_vHours[i], 0).toString(m_timeFormat));
            }
            painter.restore();

            if (m_viewPos == ScheduleViewPos::DayPos && m_beginDate == QDate::currentDate()) {
                painter.save();
                painter.setFont(font);
                painter.setPen(m_currenttimecolor);
                QString str = QTime::currentTime().toString(m_timeFormat);

                if (m_topMargin - 8 + m_vPos[m_vPos.count() - 1] >= m_topMargin)
                    painter.drawText(QRect((m_leftMargin - hourTextWidth) / 2 - 5,
                                           m_topMargin - 8 + m_vPos[m_vPos.count() - 1],
                                           hourTextWidth + 2, hourTextHeight),
                                     Qt::AlignCenter, str);
                painter.restore();
            }
        }
    }

    painter.save();
    QFont alldayfont;
    alldayfont.setWeight(QFont::Medium);
    alldayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    painter.setFont(alldayfont);
    painter.setPen(m_ALLDayColor);
    painter.drawText(QRect(0, 0, m_leftMargin - 2, m_topMargin - 2), Qt::AlignCenter, tr("ALL DAY"));
    painter.restore();

    //绘制全天与非全天之间的直线
    painter.save();
    painter.setPen(Qt::NoPen);
    //分割线y坐标点
    const int point_y = m_alldaylist->height() + m_alldaylist->y();
    //设置间隔线颜色
    painter.setBrush(m_linecolor);
    //绘制间隔线矩阵
    painter.drawRect(QRectF(0, point_y, this->width() - m_rightMargin - 2, 1));
    painter.restore();
    if (m_viewPos == ScheduleViewPos::WeekPos) {
        //如果为周视图绘制右侧背景色（否则会有一个竖线的白色背景，不协调）
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_outerBorderColor);
        painter.drawRect(QRectF(this->width() - 1, 0, this->width(), this->height()));
        //绘制个三角遮住左上角圆角
        QPainterPath path;
        path.moveTo(0,0);
        path.lineTo(m_radius, 0);
        path.lineTo(0, m_radius);
        path.lineTo(0, 0);
        painter.fillPath(path, palette().color(backgroundRole()));
    }
}

void CScheduleView::resizeEvent(QResizeEvent *event)
{
    if (m_viewPos == ScheduleViewPos::WeekPos) {
        m_sMaxNum = ((width() - m_leftMargin) / 7) / 27;
        if (m_sMaxNum < 4) {
            m_sMaxNum = 4;
        }
    }
    m_graphicsView->setMaxNum(m_sMaxNum);
    m_graphicsView->setRange(width() - m_leftMargin,
                             scheduleViewHeight(), m_beginDate, m_endDate, m_rightMargin);
    m_alldaylist->setRange(width() - m_leftMargin, 22, m_beginDate, m_endDate, m_rightMargin);
    update();
    QFrame::resizeEvent(event);
    updateAllday();
    m_graphicsView->updateInfo();
    m_graphicsView->keepCenterOnScene();
}

void CScheduleView::wheelEvent(QWheelEvent *e)
{
    if (e->orientation() == Qt::Orientation::Horizontal) {
        emit signalAngleDelta(e->angleDelta().x());
    }
}

bool CScheduleView::event(QEvent *e)
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

void CScheduleView::initUI()
{
    m_layout = new QVBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_alldaylist = new CAllDayEventWeekView(this, m_viewPos);
    m_layout->addWidget(m_alldaylist);
    m_layout->addSpacing(1);
    m_graphicsView = new CGraphicsView(this, m_viewPos);
    const int miniHeight = m_viewPos == ScheduleViewPos::WeekPos ? 300 : 380;
    m_graphicsView->setMinimumHeight(miniHeight);
    connect(m_graphicsView, SIGNAL(signalsPosHours(QVector<int>, QVector<int>, int)), this,
            SLOT(slotPosHours(QVector<int>, QVector<int>, int)));
    m_layout->addWidget(m_graphicsView);
    setLayout(m_layout);
    m_graphicsView->scrollBarValueChangedSlot();

    m_ScheduleRemindWidget = new ScheduleRemindWidget(this);
    // move focus to m_graphicsView
    setFocusProxy(m_graphicsView);
}

void CScheduleView::initConnection()
{
    connect(m_graphicsView, &CGraphicsView::signalsUpdateSchedule, this,
            &CScheduleView::slotupdateSchedule);
    connect(m_alldaylist, &CAllDayEventWeekView::signalsUpdateSchedule, this,
            &CScheduleView::slotupdateSchedule);
    connect(m_graphicsView, &CGraphicsView::signalsCurrentScheduleDate, this,
            &CScheduleView::slotCurrentScheduleDate);
    connect(m_graphicsView, &CGraphicsView::signalGotoDayView, this,
            &CScheduleView::slotCurrentScheduleDate);

    //切换前后时间信号关联
    connect(m_graphicsView, &CAllDayEventWeekView::signalAngleDelta, this, &CScheduleView::signalAngleDelta);
    connect(m_alldaylist, &CAllDayEventWeekView::signalAngleDelta, this, &CScheduleView::signalAngleDelta);

    connect(m_graphicsView, &CGraphicsView::signalScheduleShow, this, &CScheduleView::slotScheduleShow);

    connect(m_alldaylist, &CAllDayEventWeekView::signalScheduleShow, this, &CScheduleView::slotScheduleShow);

    connect(m_alldaylist, &CAllDayEventWeekView::signalUpdatePaint,
            this, &CScheduleView::slotUpdatePaint);
    connect(m_alldaylist, &CAllDayEventWeekView::signalSceneUpdate,
            this, &CScheduleView::slotUpdateScene);
    connect(m_graphicsView, &CGraphicsView::signalSceneUpdate,
            this, &CScheduleView::slotUpdateScene);


    connect(m_alldaylist, &CAllDayEventWeekView::signaleSwitchToView, this, &CScheduleView::slotSwitchView);
    connect(m_graphicsView, &CGraphicsView::signaleSwitchToView, this, &CScheduleView::slotSwitchView);

    connect(m_alldaylist, &CAllDayEventWeekView::signalViewFocusInit, m_graphicsView, &CGraphicsView::slotViewInit);

    connect(m_alldaylist, &CAllDayEventWeekView::signalSwitchPrePage, this, &CScheduleView::signalSwitchPrePage);
    connect(m_alldaylist, &CAllDayEventWeekView::signalSwitchNextPage, this, &CScheduleView::signalSwitchNextPage);
    connect(m_graphicsView, &CAllDayEventWeekView::signalSwitchPrePage, this, &CScheduleView::signalSwitchPrePage);
    connect(m_graphicsView, &CAllDayEventWeekView::signalSwitchNextPage, this, &CScheduleView::signalSwitchNextPage);
}

/**
 * @brief CScheduleView::slotDeleteitem     快捷键删除日程
 */
void CScheduleView::slotDeleteitem()
{
    //"delete"快捷键删除日程，因为只有一个点击选中日程所以全天或非全天只需要有一个删除就可以了
    m_graphicsView->slotDeleteItem();
    //因添加了对焦点选中item的快捷删除，添加对全天选中日程的删除
    m_alldaylist->slotDeleteItem();
}

void CScheduleView::slotCurrentScheduleDate(QDate date)
{
    if (m_viewPos == ScheduleViewPos::DayPos)
        return;
    emit signalsCurrentScheduleDate(date);
}

void CScheduleView::slotScheduleShow(const bool isShow, const DSchedule::Ptr &out)
{
    if (isShow) {
        QVariant variant;
        CalendarGlobalEnv::getGlobalEnv()->getValueByKey(DDECalendar::CursorPointKey, variant);
        QPoint pos22 = variant.value<QPoint>();
        //根据日程类型获取颜色
        CSchedulesColor gdColor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(
            out->scheduleTypeID());
        QDesktopWidget *w = QApplication::desktop();
        m_ScheduleRemindWidget->setData(out, gdColor);

        if ((pos22.x() + m_ScheduleRemindWidget->width() + 15) > w->width()) {
            m_ScheduleRemindWidget->setDirection(DArrowRectangle::ArrowRight);
            m_ScheduleRemindWidget->show(pos22.x() - 15, pos22.y());
        } else {
            m_ScheduleRemindWidget->setDirection(DArrowRectangle::ArrowLeft);
            m_ScheduleRemindWidget->show(pos22.x() + 15, pos22.y());
        }
    } else {
        m_ScheduleRemindWidget->hide();
    }
}

void CScheduleView::slotUpdatePaint(const int topM)
{
    m_topMargin = topM;
    update();
}

void CScheduleView::slotUpdateScene()
{
    m_graphicsView->slotUpdateScene();
    m_alldaylist->slotUpdateScene();
}

/**
 * @brief CScheduleView::slotSwitchView     焦点切换到某个视图
 * @param viewtype
 */
void CScheduleView::slotSwitchView(const QDate &focusDate, CWeekDayGraphicsview::ViewType viewtype, bool setItemFocus)
{
    if (viewtype == CWeekDayGraphicsview::ALLDayView) {
        m_alldaylist->setCurrentFocusItem(focusDate, setItemFocus);
        m_alldaylist->setFocus(Qt::TabFocusReason);
    } else {
        m_graphicsView->setCurrentFocusItem(focusDate, setItemFocus);
        m_graphicsView->setFocus(Qt::TabFocusReason);
    }
}

/**
 * @brief CScheduleView::updateSchedule         更新日程显示
 */
void CScheduleView::updateSchedule()
{
    //获取一个月的日程信息
    m_graphicsView->clearSchedule();
    DSchedule::List allInfo;
    DSchedule::List nonAllInfo;

    QMap<QDate, DSchedule::List>::const_iterator _iterator = m_showSchedule.constBegin();
    for (; _iterator != m_showSchedule.constEnd(); ++_iterator) {
        for (int i = 0; i < _iterator->size(); ++i) {
            if (_iterator.value().at(i)->allDay()) {
                //过滤
                if (!allInfo.contains(_iterator.value().at(i))) {
                    allInfo.append(_iterator.value().at(i));
                }
            } else {
                //过滤
                if (!nonAllInfo.contains(_iterator.value().at(i))) {
                    nonAllInfo.append(_iterator.value().at(i));
                }
            }
        }
    }
    m_alldaylist->setInfo(allInfo);
    m_graphicsView->setInfo(nonAllInfo);
    updateAllday();
    m_graphicsView->updateInfo();
    m_graphicsView->update();
    m_graphicsView->scene()->update();
}

void CScheduleView::updateAllday()
{
    m_alldaylist->updateInfo();
    update();
    m_graphicsView->resize(m_graphicsView->width(), this->height() - m_alldaylist->height());
}

int CScheduleView::scheduleViewHeight()
{
    qreal mHeight = 0;

    if (m_viewPos == ScheduleViewPos::DayPos) {
        mHeight = 24 * (0.0968 * height() + 0.5);
    } else {
        mHeight = 24 * (0.083 * height() + 0.5);
    }
    //现在最小高度为20;
    mHeight = mHeight < 500 ? 1035 : mHeight;
    int m_minTime = qRound((20.0 / mHeight) * 86400);
    m_graphicsView->setMinTime(m_minTime);
    return qRound(mHeight);
}
