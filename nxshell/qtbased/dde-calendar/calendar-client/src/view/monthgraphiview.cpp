// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthgraphiview.h"
#include "../widget/monthWidget/monthscheduleview.h"
#include "../dialog/scheduledlg.h"
#include "../dialog/myscheduleview.h"
#include "../widget/touchgestureoperation.h"
#include "constants.h"
#include "commondef.h"
#include "graphicsItem/cmonthschedulenumitem.h"
#include "cscenetabkeydeal.h"
#include "ckeyenabledeal.h"
#include "ckeyleftdeal.h"
#include "ckeyrightdeal.h"
#include "ckeyupdeal.h"
#include "ckeydowndeal.h"

#include <QShortcut>
#include <QMouseEvent>

CMonthGraphicsview::CMonthGraphicsview(QWidget *parent)
    : DragInfoGraphicsView(parent)
{
    //设置显示左右下角圆角
    setShowRadius(true, true);

    m_MonthScheduleView = new CMonthScheduleView(this, m_Scene);
    connect(this, &CMonthGraphicsview::signalFontChange, m_MonthScheduleView, &CMonthScheduleView::slotFontChange);

    for (int i = 0; i < DDEMonthCalendar::ItemSizeOfMonthDay; ++i) {
        CMonthDayItem *item = new CMonthDayItem();
        item->setZValue(-1);
        if (m_DayItem.size() > 0) {
            //设置对应左右和下一个                            关系
            m_DayItem.last()->setNextFocusItem(item);
            m_DayItem.last()->setRightItem(item);
            item->setLeftItem(m_DayItem.last());
        }
        int upNum = i - 7;
        //如果对应的上一排编号大于零则设置对应的上下关系
        if (upNum >= 0) {
            m_DayItem.at(upNum)->setDownItem(item);
            item->setUpItem(m_DayItem.at(upNum));
        }
        //设置编号
        item->setBackgroundNum(i);
        m_DayItem.append(item);
        m_Scene->addItem(item);
    }
    updateSize();
    m_Scene->setFirstFocusItem(m_DayItem.first());
    //添加键盘事件处理
    CKeyPressPrxy *m_keyPrxy = new CKeyPressPrxy();
    m_keyPrxy->addkeyPressDeal(new CSceneTabKeyDeal(m_Scene));
    m_keyPrxy->addkeyPressDeal(new CKeyEnableDeal(m_Scene));
    m_keyPrxy->addkeyPressDeal(new CKeyLeftDeal(m_Scene));
    m_keyPrxy->addkeyPressDeal(new CKeyRightDeal(m_Scene));
    m_keyPrxy->addkeyPressDeal(new CKeyUpDeal(m_Scene));
    m_keyPrxy->addkeyPressDeal(new CKeyDownDeal(m_Scene));
    m_Scene->setKeyPressPrxy(m_keyPrxy);
}

CMonthGraphicsview::~CMonthGraphicsview()
{
    m_DayItem.clear();
}

void CMonthGraphicsview::setTheMe(int type)
{
    m_themetype = type;
    DragInfoGraphicsView::setTheMe(type);

    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setTheMe(type);
    }
}

void CMonthGraphicsview::setDate(const QVector<QDate> &showDate)
{
    Q_ASSERT(showDate.size() == 42);
    if (showDate.at(0).day() != 1) {
        m_currentMonth = showDate.at(0).addMonths(1).month();
    } else {
        m_currentMonth = showDate.at(0).month();
    }
    //根据当前时间设置当前场景的第一个焦点item
    int currentIndex = 0;
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(showDate.at(i));
        m_DayItem.at(i)->setCurrentMonth(showDate.at(i).month() == m_currentMonth);
        if (showDate.at(i) == getCurrentDate()) {
            currentIndex = i;
        }
    }
    m_Scene->setFirstFocusItem(m_DayItem.at(currentIndex));
    m_schedulelistdata.clear();
    this->scene()->update();
}

void CMonthGraphicsview::setFestival(const QMap<QDate, int> &festivalInfo)
{
    m_festivallist = festivalInfo;
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setStatus(static_cast<CMonthDayItem::HolidayStatus>(m_festivallist[m_DayItem.at(i)->getDate()]));
    }
    this->scene()->update();
}

void CMonthGraphicsview::setLunarInfo(const QMap<QDate, CaHuangLiDayInfo> &lunarCache)
{
    m_lunarCache = lunarCache;
    updateLunar();
}

void CMonthGraphicsview::setLunarVisible(bool visible)
{
    CMonthDayItem::m_LunarVisible = visible;
}

/**
 * @brief CMonthGraphicsview::setScheduleInfo 设置日程信息
 * @param info
 */
void CMonthGraphicsview::setScheduleInfo(const QMap<QDate, DSchedule::List> &info)
{
    m_schedulelistdata = info;
    updateInfo();
}

/**
 * @brief CMonthGraphicsview::setSelectSearchSchedule     设置选择搜索日程
 * @param scheduleInfo
 */
void CMonthGraphicsview::setSelectSearchSchedule(const DSchedule::Ptr &scheduleInfo)
{
    DragInfoGraphicsView::setSelectSearchSchedule(scheduleInfo);
    //获取所有的日程item
    QVector<QGraphicsRectItem *> mScheduleShowBtn = m_MonthScheduleView->getScheduleShowItem();

    for (int i = 0; i < mScheduleShowBtn.size(); ++i) {
        CMonthScheduleItem *item = dynamic_cast<CMonthScheduleItem *>(mScheduleShowBtn.at(i));

        if (item == nullptr) continue;

        if (scheduleInfo == item->getData()) {
            item->setStartValue(0);
            item->setEndValue(4);
            item->startAnimation();
        }
    }
}

/**
 * @brief CMonthGraphicsview::setSearchScheduleInfo       设置搜索日程信息
 * @param searchScheduleInfo
 */
void CMonthGraphicsview::setSearchScheduleInfo(const DSchedule::List &searchScheduleInfo)
{
    DragInfoItem::setSearchScheduleInfo(searchScheduleInfo);
    this->scene()->update();
}

void CMonthGraphicsview::updateSize()
{
    //场景的大小和位置
    QRectF sceneRect(0, 0, viewport()->rect().width(), viewport()->rect().height());
    m_Scene->setSceneRect(sceneRect);
    qreal w = m_Scene->width() / DDEMonthCalendar::AFewDaysOfWeek;
    qreal h = m_Scene->height() / DDEMonthCalendar::LinesNumOfMonth;
    QRectF rect ;
    int w_offset = 0;
    int h_offset = 0;

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        h_offset = i / DDEMonthCalendar::AFewDaysOfWeek;
        w_offset = i % DDEMonthCalendar::AFewDaysOfWeek;
        rect.setRect(w * w_offset,
                     h * h_offset,
                     w,
                     h);
        m_DayItem.at(i)->setRect(rect);
    }
}

void CMonthGraphicsview::updateLunar()
{
    QDate date;
    CaHuangLiDayInfo info;
    QString lunarStr("");

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        date = m_DayItem.at(i)->getDate();
        if (m_lunarCache.contains(date)) {
            info = m_lunarCache.value(date);

            if (info.mLunarDayName == "初一") {
                //如果为闰月只显示月份不显示天
                if (info.mLunarMonthName.contains("闰")) {
                    info.mLunarDayName = info.mLunarMonthName;
                } else {
                    info.mLunarDayName = info.mLunarMonthName + info.mLunarDayName;
                }
            }

            if (info.mTerm.isEmpty()) {
                lunarStr = info.mLunarDayName;
            } else {
                lunarStr = info.mTerm;
            }
        } else {
            lunarStr = "";
        }
        m_DayItem.at(i)->setLunar(lunarStr);
    }
}

/**
 * @brief CMonthGraphicsview::updateInfo      更新日程数据显示
 */
void CMonthGraphicsview::updateInfo()
{
    DragInfoGraphicsView::updateInfo();
    int h = m_MonthScheduleView->getScheduleHeight();
    m_MonthScheduleView->setallsize(this->viewport()->width(),
                                    this->viewport()->height(),
                                    0, 0, 0, h);
    m_MonthScheduleView->setData(m_schedulelistdata, 1);

    switch (m_DragStatus) {
    case IsCreate:
        upDateInfoShow(IsCreate, m_DragScheduleInfo);
        break;
    case ChangeWhole:
        upDateInfoShow(ChangeWhole, m_DragScheduleInfo);
        break;
    default:

        break;
    }
    viewport()->update();
    update();
    //更新背景上显示的item
    updateBackgroundShowItem();
}

QPointF CMonthGraphicsview::getItemPos(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    return  QPointF(scenePos.x() - itemRect.x(),
                    scenePos.y() - itemRect.y());
}

CMonthGraphicsview::PosInItem CMonthGraphicsview::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    QPointF itemPos = QPointF(scenePos.x() - itemRect.x(),
                              scenePos.y() - itemRect.y());
    qreal bottomY = itemRect.width() - itemPos.x();

    if (itemPos.x() < 5) {
        return LEFT;
    }
    if (bottomY < 5) {
        return RIGHT;
    }
    return MIDDLE;
}

QDateTime CMonthGraphicsview::getPosDate(const QPoint &p)
{
    if (!this->sceneRect().contains(p))
        return m_MoveDate;
    QRectF rect = this->sceneRect();
    qreal x = 0;
    qreal y = 0;

    if (p.x() < 0) {
        x = 0;
    } else if (p.x() > (rect.width() - 10)) {
        x = rect.width() - 10;
    } else {
        x = p.x();
    }

    if (p.y() < 0) {
        y = 0;
    } else if (p.y() > (rect.height() - 10)) {
        y = rect.height() - 10;
    } else {
        y = p.y();
    }

    int xoffset = qFloor(x / (rect.width() / DDEMonthCalendar::AFewDaysOfWeek)) % DDEMonthCalendar::AFewDaysOfWeek;
    int yoffset = qFloor(y / (rect.height() / DDEMonthCalendar::LinesNumOfMonth)) % DDEMonthCalendar::LinesNumOfMonth;

    return  QDateTime(m_DayItem[xoffset + yoffset * 7]->getDate(),
                      QTime(0, 0, 0));
}

void CMonthGraphicsview::upDateInfoShow(const CMonthGraphicsview::DragStatus &status, const DSchedule::Ptr &info)
{
    switch (status) {
    case NONE:
        Q_UNUSED(info);
        break;
    case ChangeBegin:
    case ChangeEnd: {
        m_MonthScheduleView->changeDate(info);
    }
    break;
    case ChangeWhole: {
    }
    break;
    case IsCreate:
        m_MonthScheduleView->updateDate(info);
        break;
    }
}

void CMonthGraphicsview::slideEvent(QPointF &startPoint, QPointF &stopPort)
{
    qreal _movingLine {0};
    touchGestureOperation::TouchMovingDirection _touchMovingDir =
        touchGestureOperation::getTouchMovingDir(startPoint, stopPort, _movingLine);
    //切换月份 0 不切换  1 下一个  -1 上个月
    int delta {0};
    //移动偏移 25则切换月份
    const int moveOffset = 25;
    switch (_touchMovingDir) {
    case touchGestureOperation::T_TOP: {
        if (_movingLine > moveOffset) {
            delta = -1;
            startPoint = stopPort;
        }
        break;
    }
    case touchGestureOperation::T_BOTTOM: {
        if (_movingLine > moveOffset) {
            delta = 1;
            startPoint = stopPort;
        }
        break;
    }
    default:
        break;
    }
    if (delta != 0) {
        emit signalAngleDelta(delta);
    }
}

void CMonthGraphicsview::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    QGraphicsItem *listItem = itemAt(event->pos());
    CMonthScheduleNumItem *item = dynamic_cast<CMonthScheduleNumItem *>(listItem);

    if (item != nullptr) {
        //双击切换视图
        if (item->getDate().year() > DDECalendar::QueryEarliestYear) {
            emit signalsViewSelectDate(item->getDate());
        }
        return;
    }

    CMonthScheduleItem *infoitem = dynamic_cast<CMonthScheduleItem *>(listItem);

    if (infoitem != nullptr) {
        CMyScheduleView dlg(infoitem->getData(), this);
        connect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CMonthGraphicsview::signalsUpdateSchedule);
        dlg.exec();
        return;
    }

    CMonthDayItem *Dayitem = dynamic_cast<CMonthDayItem *>(listItem);

    if (Dayitem != nullptr) {
        QPointF point = getItemPos(event->pos(), Dayitem->rect());
        if (point.y() < 38) {
            //双击切换视图
            if (Dayitem->getDate().year() > DDECalendar::QueryEarliestYear) {
                emit signalsViewSelectDate(Dayitem->getDate());
            }
        } else {
            //双击新建日程
            if (Dayitem->getDate().year() >= DDECalendar::QueryEarliestYear) {
                if (Dayitem->getDate().year() <= DDECalendar::QueryLatestYear) {
                    slotCreate(QDateTime(Dayitem->getDate(), QTime(0, 0, 0)));
                }
            }
        }
    }
}

void CMonthGraphicsview::resizeEvent(QResizeEvent *event)
{
    DragInfoGraphicsView::resizeEvent(event);
    updateSize();
    updateInfo();
}
void CMonthGraphicsview::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        emit signalFontChange();
    }
}

void CMonthGraphicsview::wheelEvent(QWheelEvent *e)
{
    //如果滚动为上下则发送信号
    if (e->orientation() == Qt::Orientation::Vertical) {
        emit signalAngleDelta(e->angleDelta().y());
    }
}

/**
 * @brief CMonthGraphicsview::updateBackgroundShowItem    更新背景上显示的item
 */
void CMonthGraphicsview::updateBackgroundShowItem()
{
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->updateShowItem();
    }
}

void CMonthGraphicsview::setSceneCurrentItemFocus(const QDate &focusDate)
{
    int offset = static_cast<int>(m_DayItem.first()->getDate().daysTo(focusDate));
    if (offset >= 0 && offset < m_DayItem.size()) {
        m_Scene->setCurrentFocusItem(m_DayItem.at(offset));
        m_Scene->currentFocusItemUpdate();
    } else {
        qCWarning(ClientLogger) << "Switching time range error! focusDate:" << focusDate << " first item date:" << m_DayItem.first()->getDate();
    }
}

void CMonthGraphicsview::setDragPixmap(QDrag *drag, DragInfoItem *item)
{
    CMonthScheduleItem *infoitem = dynamic_cast<CMonthScheduleItem *>(item);
    drag->setPixmap(infoitem->getPixmap());
}

bool CMonthGraphicsview::MeetCreationConditions(const QDateTime &date)
{
    return qAbs(date.daysTo(m_PressDate)) < 43;
}

bool CMonthGraphicsview::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    return timeFirst.date() == timeSecond.date();
}

bool CMonthGraphicsview::JudgeIsCreate(const QPointF &pos)
{
    return qAbs(pos.x() - m_PressPos.x()) > 20 || qAbs(m_PressDate.daysTo(getPosDate(pos.toPoint()))) > 0;
}

void CMonthGraphicsview::RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
{
    Q_UNUSED(pos);
    CMonthDayItem *Dayitem = dynamic_cast<CMonthDayItem *>(listItem);

    if (Dayitem != nullptr) {
        m_rightMenu->clear();
        m_rightMenu->addAction(m_createAction);
        m_createDate.setDate(Dayitem->getDate());
        m_rightMenu->exec(QCursor::pos());
    }
}

void CMonthGraphicsview::MoveInfoProcess(DSchedule::Ptr &info, const QPointF &pos)
{
    qint64 offset       = m_PressDate.daysTo(m_MoveDate);
    info->setDtStart(info->dtStart().addDays(offset));
    info->setDtEnd(info->dtEnd().addDays(offset));
    qreal y = 0;
    QRectF rect = this->sceneRect();

    if (pos.y() < 0) {
        y = 0;
    } else if (pos.y() > rect.height()) {
        y = rect.height();
    } else {
        y = pos.y();
    }

    int yoffset = qFloor(y / (rect.height() / DDEMonthCalendar::LinesNumOfMonth)) % DDEMonthCalendar::LinesNumOfMonth;
    m_MonthScheduleView->updateDate(yoffset, info);
}

/**
 * @brief CMonthGraphicsview::getDragScheduleInfoBeginTime        获取移动开始时间
 * @param moveDateTime
 * @return
 */
QDateTime CMonthGraphicsview::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    //获取移动开始时间
    QDateTime _beginTime = moveDateTime.daysTo(m_InfoEndTime) < 0 ?
                           QDateTime(m_InfoEndTime.date(), m_InfoBeginTime.time()) :
                           QDateTime(moveDateTime.date(), m_InfoBeginTime.time());
    //如果开始时间晚与结束时间则减少一天
    _beginTime = _beginTime > m_InfoEndTime ? _beginTime.addDays(-1) : _beginTime;
    return _beginTime;
}

/**
 * @brief CMonthGraphicsview::getDragScheduleInfoEndTime      获取结束时间
 * @param moveDateTime
 * @return
 */
QDateTime CMonthGraphicsview::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    //获取结束时间
    QDateTime _endTime = m_InfoBeginTime.daysTo(moveDateTime) < 0 ?
                         QDateTime(m_InfoBeginTime.date(), m_InfoEndTime.time()) :
                         QDateTime(moveDateTime.date(), m_InfoEndTime.time());
    //如果结束时间小于开始时间则添加一天
    _endTime = _endTime < m_InfoBeginTime ? _endTime.addDays(1)  : _endTime;
    return _endTime;
}

void CMonthGraphicsview::slotCreate(const QDateTime &date)
{
    CScheduleDlg dlg(1, this);
    QDateTime tDatatime;
    tDatatime.setDate(date.date());

    if (date.date() == QDate::currentDate()) {
        tDatatime.setTime(QTime::currentTime());
    } else {
        tDatatime.setTime(QTime(8, 0));
    }

    dlg.setDate(tDatatime);
    dlg.setAllDay(true);

    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateSchedule();
        emit signalsScheduleUpdate(0);
    }
}
