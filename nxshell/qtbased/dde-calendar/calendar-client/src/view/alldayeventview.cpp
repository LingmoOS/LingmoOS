// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "alldayeventview.h"
#include "schedulecoormanage.h"
#include "schedulectrldlg.h"
#include "scheduledlg.h"
#include "myscheduleview.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "scheduledaterangeinfo.h"

#include <DHiDPIHelper>
#include <DPalette>
#include <DMenu>

#include <QAction>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include <QMimeData>
#include <QDrag>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGraphicsOpacityEffect>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

void CAllDayEventWeekView::setTheMe(int type)
{
    CWeekDayGraphicsview::setTheMe(type);
}

void CAllDayEventWeekView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateItemHeightByFontSize();
        updateInfo();
    }
}

bool CAllDayEventWeekView::MeetCreationConditions(const QDateTime &date)
{
    return qAbs(date.daysTo(m_PressDate)) < 7;
}

void CAllDayEventWeekView::slotCreate(const QDateTime &date)
{
    CScheduleDlg dlg(1, this);
    dlg.setDate(date);
    dlg.setAllDay(true);
    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateSchedule();
    }
}

bool CAllDayEventWeekView::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    return timeFirst.date() == timeSecond.date();
}

bool CAllDayEventWeekView::JudgeIsCreate(const QPointF &pos)
{
    return qAbs(pos.x() - m_PressPos.x()) > 20 || qAbs(m_PressDate.date().daysTo(m_coorManage->getsDate(mapFrom(this, pos.toPoint())))) > 0;
}

void CAllDayEventWeekView::RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
{
    Q_UNUSED(listItem);
    m_rightMenu->clear();
    m_rightMenu->addAction(m_createAction);

    m_createDate.setDate(m_coorManage->getsDate(mapFrom(this, pos)));
    m_createDate.setTime(QTime::currentTime());
    m_rightMenu->exec(QCursor::pos());
}

void CAllDayEventWeekView::MoveInfoProcess(DSchedule::Ptr &info, const QPointF &pos)
{
    Q_UNUSED(pos);
    if (info->allDay()) {
        qint64 offset = m_PressDate.daysTo(m_MoveDate);
        info->setDtStart(info->dtStart().addDays(offset));
        info->setDtEnd(info->dtEnd().addDays(offset));
    } else {
        qint64 offset = info->dtStart().daysTo(info->dtEnd());
        info->setAllDay(true);
        info->setAlarmType(DSchedule::Alarm_15Hour_Front);
        m_DragScheduleInfo->setDtStart(QDateTime(m_MoveDate.date(), QTime(0, 0, 0)));
        m_DragScheduleInfo->setDtEnd(QDateTime(m_MoveDate.addDays(offset).date(), QTime(23, 59, 59)));
    }
    upDateInfoShow(ChangeWhole, info);
}

QDateTime CAllDayEventWeekView::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    return moveDateTime.daysTo(m_InfoEndTime) < 0 ? QDateTime(m_InfoEndTime.date(), QTime(0, 0, 0)) : QDateTime(moveDateTime.date(), QTime(0, 0, 0));
}

QDateTime CAllDayEventWeekView::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    return m_InfoBeginTime.daysTo(m_MoveDate) < 0 ? QDateTime(m_InfoBeginTime.date(), QTime(23, 59, 0)) : QDateTime(moveDateTime.date(), QTime(23, 59, 0));
}

void CAllDayEventWeekView::updateHeight()
{
    for (int i = 0; i < m_baseShowItem.count(); i++) {
        m_baseShowItem.at(i)->update();
    }
}

/**
 * @brief CAllDayEventWeekView::setSelectSearchSchedule     设置搜索选中日程
 * @param info
 */
void CAllDayEventWeekView::setSelectSearchSchedule(const DSchedule::Ptr &info)
{
    DragInfoGraphicsView::setSelectSearchSchedule(info);
    for (int i = 0; i < m_baseShowItem.size(); ++i) {
        CAllDayScheduleItem *item = m_baseShowItem.at(i);
        if (item->hasSelectSchedule(info)) {
            QRectF rect = item->rect();
            centerOn(0, rect.y());
            setTransformationAnchor(QGraphicsView::AnchorViewCenter);
            item->setStartValue(0);
            item->setEndValue(4);
            item->startAnimation();
        }
    }
}

void CAllDayEventWeekView::setMargins(int left, int top, int right, int bottom)
{
    setViewportMargins(QMargins(left, top, right, bottom));
}

/**
 * @brief CAllDayEventWeekView::updateInfo  更新日程显示
 */
void CAllDayEventWeekView::updateInfo()
{
    DragInfoGraphicsView::updateInfo();
    switch (m_DragStatus) {
    case IsCreate:
        upDateInfoShow(IsCreate, m_DragScheduleInfo);
        break;
    default:
        upDateInfoShow();
        break;
    }
}

void CAllDayEventWeekView::upDateInfoShow(const DragStatus &status, const DSchedule::Ptr &info)
{
    DSchedule::List vListData;
    vListData = m_scheduleInfo;
    switch (status) {
    case NONE:
        Q_UNUSED(info);
        break;
    case ChangeBegin:
    case ChangeEnd: {
        int index = vListData.indexOf(info);
        if (index >= 0)
            vListData[index] = info;
    } break;
    case ChangeWhole:
        vListData.append(info);
        break;
    case IsCreate:
        vListData.append(info);
        break;
    }

    std::sort(vListData.begin(), vListData.end());

    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    for (int i = 0; i < vListData.count(); i++) {
        DSchedule::Ptr ptr = vListData.at(i);
        if (ptr.isNull()) {
            continue;
        }

        QDate tbegindate = ptr->dtStart().date();
        QDate tenddate = ptr->dtEnd().date();
        if (tbegindate < m_beginDate)
            tbegindate = m_beginDate;
        if (tenddate > m_endDate)
            tenddate = m_endDate;
        MScheduleDateRangeInfo sinfo;
        sinfo.bdate = tbegindate;
        sinfo.edate = tenddate;
        sinfo.tData = ptr;
        sinfo.state = false;
        vMDaySchedule.append(sinfo);
    }
    QVector<QVector<int>> vCfillSchedule;
    vCfillSchedule.resize(vListData.count());
    int tNum = static_cast<int>(m_beginDate.daysTo(m_endDate) + 1);
    for (int i = 0; i < vListData.count(); i++) {
        vCfillSchedule[i].resize(tNum);
        vCfillSchedule[i].fill(-1);
    }
    //首先填充跨天日程
    for (int i = 0; i < vMDaySchedule.count(); i++) {
        if (vMDaySchedule[i].state)
            continue;
        int bindex = static_cast<int>(m_beginDate.daysTo(vMDaySchedule[i].bdate));
        int eindex = static_cast<int>(m_beginDate.daysTo(vMDaySchedule[i].edate));
        int c = -1;
        for (int k = 0; k < vListData.count(); k++) {
            int t = 0;
            for (t = bindex; t <= eindex; t++) {
                if (vCfillSchedule[k][t] != -1) {
                    break;
                }
            }
            if (t == eindex + 1) {
                c = k;
                break;
            }
        }
        if (c == -1)
            continue;

        bool flag = false;
        for (int sd = bindex; sd <= eindex; sd++) {
            if (vCfillSchedule[c][sd] != -1)
                continue;
            vCfillSchedule[c][sd] = i;
            flag = true;
        }
        if (flag)
            vMDaySchedule[i].state = true;
    }
    QVector<DSchedule::List> vResultData;
    for (int i = 0; i < vListData.count(); i++) {
        QVector<int> vId;
        for (int j = 0; j < tNum; j++) {
            if (vCfillSchedule[i][j] != -1) {
                int k = 0;
                for (; k < vId.count(); k++) {
                    if (vId[k] == vCfillSchedule[i][j])
                        break;
                }
                if (k == vId.count())
                    vId.append(vCfillSchedule[i][j]);
            }
        }
        DSchedule::List tData;
        for (int j = 0; j < vId.count(); j++) {
            tData.append(vMDaySchedule[vId[j]].tData);
        }
        if (!tData.isEmpty())
            vResultData.append(tData);
    }

    int m_topMagin;
    if (vResultData.count() < 2) {
        m_topMagin = 32;
    } else if (vResultData.count() < 6) {
        m_topMagin = 31 + (vResultData.count() - 1) * (itemHeight + 1);
    } else {
        m_topMagin = 123;
    }
    setFixedHeight(m_topMagin - 3);
    setDayData(vResultData);
    update();
    emit signalUpdatePaint(m_topMagin);
}

CAllDayEventWeekView::CAllDayEventWeekView(QWidget *parent, ViewPosition type)
    : CWeekDayGraphicsview(parent, type, ViewType::ALLDayView)
{
    updateItemHeightByFontSize();
}

CAllDayEventWeekView::~CAllDayEventWeekView()
{
}

void CAllDayEventWeekView::setDayData(const QVector<DSchedule::List> &vlistData)
{
    m_vlistData = vlistData;
    updateDateShow();
}

void CAllDayEventWeekView::setInfo(const DSchedule::List &info)
{
    m_scheduleInfo = info;
}

void CAllDayEventWeekView::slotDoubleEvent()
{
    m_updateDflag = true;
    emit signalsUpdateSchedule();
}

void CAllDayEventWeekView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }
    emit signalScheduleShow(false);
    DGraphicsView::mouseDoubleClickEvent(event);
    CAllDayScheduleItem *item = dynamic_cast<CAllDayScheduleItem *>(itemAt(event->pos()));
    if (item == nullptr) {
        m_createDate.setDate(m_coorManage->getsDate(mapFrom(this, event->pos())));
        m_createDate.setTime(QTime::currentTime());
        slotCreate(m_createDate);
    } else {
        m_updateDflag = false;
        CMyScheduleView dlg(item->getData(), this);
        connect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CAllDayEventWeekView::slotDoubleEvent);
        dlg.exec();
        disconnect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CAllDayEventWeekView::slotDoubleEvent);
    }
}

void CAllDayEventWeekView::wheelEvent(QWheelEvent *event)
{
    //若滚轮事件为左右方向则退出
    if (event->orientation() == Qt::Orientation::Horizontal) {
        return;
    }
    emit signalScheduleShow(false);
    DGraphicsView::wheelEvent(event);
}

void CAllDayEventWeekView::updateDateShow()
{
    qreal sceneHeight;
    qreal itemsHeight = (itemHeight + 1) * m_vlistData.size();
    if (itemsHeight < 32) {
        sceneHeight = 29;
    } else {
        sceneHeight = itemsHeight + 6;
    }
    //如果设置场景的高度小于viewport的高度则设置场景的高度为viewport的高度
    sceneHeight = sceneHeight < viewport()->height() ? viewport()->height() : sceneHeight;
    setSceneRect(0, 0, m_Scene->width(), sceneHeight);

    for (int i = 0; i < m_baseShowItem.count(); i++) {
        delete m_baseShowItem[i];
    }
    m_baseShowItem.clear();
    for (int i = 0; i < m_vlistData.size(); ++i) {
        createItemWidget(i);
    }
    //更新每个背景上的日程标签
    updateBackgroundShowItem();
}

void CAllDayEventWeekView::createItemWidget(int index, bool average)
{
    Q_UNUSED(average)
    for (int i = 0; i < m_vlistData[index].size(); ++i) {
        const DSchedule::Ptr &info = m_vlistData[index].at(i);
        QRectF drawrect = m_coorManage->getAllDayDrawRegion(info->dtStart().date(), info->dtEnd().date());
        drawrect.setY(2 + (itemHeight + 1) * index);
        drawrect.setHeight(itemHeight);

        CAllDayScheduleItem *gwi = new CAllDayScheduleItem(drawrect, nullptr);
        gwi->setData(m_vlistData[index].at(i));
        m_Scene->addItem(gwi);
        m_baseShowItem.append(gwi);
    }
}

void CAllDayEventWeekView::updateItemHeightByFontSize()
{
    QFont font;
    DFontSizeManager::instance()->setFontGenericPixelSize(
        static_cast<quint16>(DFontSizeManager::instance()->fontPixelSize(qGuiApp->font())));
    font = DFontSizeManager::instance()->t8(font);
    QFontMetrics fm(font);
    int h = fm.height();
    if (itemHeight != h) {
        itemHeight = h;
    }
}

CAllDayEventWeekView::PosInItem CAllDayEventWeekView::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    QPointF itemPos = QPointF(scenePos.x() - itemRect.x(),
                              scenePos.y() - itemRect.y());
    double bottomy = itemRect.width() - itemPos.x();
    if (itemPos.x() < 5) {
        return LEFT;
    }
    if (bottomy < 5) {
        return RIGHT;
    }
    return MIDDLE;
}

QDateTime CAllDayEventWeekView::getPosDate(const QPoint &p)
{
    return QDateTime(m_coorManage->getsDate(mapFrom(this, p)), QTime(0, 0, 0));
}

void CAllDayEventWeekView::slotUpdateScene()
{
    this->scene()->update();
}
