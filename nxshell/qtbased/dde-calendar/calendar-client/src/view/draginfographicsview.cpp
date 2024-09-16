// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "draginfographicsview.h"
#include "scheduledlg.h"
#include "schedulectrldlg.h"
#include "myscheduleview.h"
#include "constants.h"
#include "cscheduleoperation.h"
#include "graphicsItem/cscenebackgrounditem.h"
#include "calendarglobalenv.h"
#include "scheduledatamanage.h"
#include "graphicsItem/scheduleitem.h"

#include <DMenu>

#include <QMimeData>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QtMath>
#include <QPainter>
#include <QPainterPath>

//定义拖拽日程
DSchedule::Ptr DragInfoGraphicsView::m_DragScheduleInfo;
bool DragInfoGraphicsView::m_hasUpdateMark = false;

DragInfoGraphicsView::DragInfoGraphicsView(DWidget *parent)
    : DGraphicsView(parent)
    , m_Scene(new CGraphicsScene(this))
    , m_rightMenu(new DMenu(this))
    , m_MoveDate(QDateTime::currentDateTime())
{
    setFrameShape(QFrame::NoFrame);
    setScene(m_Scene);
    setContentsMargins(0, 0, 0, 0);

    m_editAction = new QAction(tr("Edit"), this);
    m_deleteAction = new QAction(tr("Delete"), this);
    m_createAction = new QAction(tr("New event"), this);
    connect(m_createAction, &QAction::triggered, this,
            static_cast<void (DragInfoGraphicsView::*)()>(&DragInfoGraphicsView::slotCreate));
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setViewportMargins(0, 0, 0, 0);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_Drag = new QDrag(this);
    //设置接受触摸事件
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::SwipeGesture);

    m_touchAnimation = new QPropertyAnimation(this, "touchSlidePos", this);
    //设置动画时间
    m_touchAnimation->setDuration(1000);
    //设置动画曲线
    m_touchAnimation->setEasingCurve(QEasingCurve::OutQuart);

    connect(m_Scene, &CGraphicsScene::signalSwitchPrePage, this, &DragInfoGraphicsView::slotSwitchPrePage);
    connect(m_Scene, &CGraphicsScene::signalSwitchNextPage, this, &DragInfoGraphicsView::slotSwitchNextPage);
    connect(m_Scene, &CGraphicsScene::signalGotoDayView, this, &DragInfoGraphicsView::signalGotoDayView);
    connect(m_Scene, &CGraphicsScene::signalContextMenu, this, &DragInfoGraphicsView::slotContextMenu);
    connect(m_Scene, &CGraphicsScene::signalsetNextFocus, this, &DragInfoGraphicsView::slotsetNextFocus);
    setFocusPolicy(Qt::StrongFocus);
    //日程类型发生改变，刷新界面
    connect(gAccountManager, &AccountManager::signalScheduleTypeUpdate, [&]() {
        this->viewport()->update();
    });
}

DragInfoGraphicsView::~DragInfoGraphicsView()
{
}

void DragInfoGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }
    stopTouchAnimation();
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸点击则记录相关状态并改变触摸状态
        DGraphicsView::mousePressEvent(event);
        m_TouchBeginPoint = event->pos();
        m_TouchBeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_touchState = TS_PRESS;
        return;
    }
    mousePress(event->pos());
    m_Scene->currentItemInit();
    //更新其它view中item状态显示
    emit signalSceneUpdate();
}

void DragInfoGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸点击状态则调用左击事件处理
        if (m_touchState == TS_PRESS) {
            mousePress(m_TouchBeginPoint.toPoint());
        }
        if (m_touchState == TS_SLIDE) {
            stopTouchAnimation();
            const qint64 timeOffset = QDateTime::currentDateTime().toMSecsSinceEpoch() - m_TouchBeginTime;
            //如果为快速滑动则开启滑动动画效果
            if (timeOffset < 150) {
                m_touchAnimation->setStartValue(verticalScrollBar()->sliderPosition());
                m_touchAnimation->setEndValue(verticalScrollBar()->minimum());
                switch (m_touchMovingDir) {
                case touchGestureOperation::T_TOP: {
                    //如果手势往上
                    m_touchAnimation->setStartValue(verticalScrollBar()->sliderPosition());
                    m_touchAnimation->setEndValue(verticalScrollBar()->maximum());
                    m_touchAnimation->start();
                    break;
                }
                case touchGestureOperation::T_BOTTOM: {
                    //如果手势往下
                    m_touchAnimation->setStartValue(verticalScrollBar()->sliderPosition());
                    m_touchAnimation->setEndValue(verticalScrollBar()->minimum());
                    m_touchAnimation->start();
                    break;
                }
                default:

                    break;
                }
            }
        }
    }
    m_touchState = TS_NONE;
    mouseReleaseScheduleUpdate();
    //update scene
    this->scene()->update();
}


void DragInfoGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //移动偏移
    const int lengthOffset = 5;
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        m_touchMovingDir = touchGestureOperation::T_MOVE_NONE;
        switch (m_touchState) {
        case TS_NONE: {
            break;
        }
        case TS_PRESS: {
            //1 点击
            qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            qint64 timeOffset = currentTime - m_TouchBeginTime;
            //获取移动距离
            qreal movingLength = QLineF(m_TouchBeginPoint, event->pos()).length();
            //如果移动距离小于5且点击时间大于250毫秒小于900毫秒则为拖拽移动状态
            if (movingLength < lengthOffset && (timeOffset > 250 && timeOffset < 900)) {
                m_touchState = TS_DRAG_MOVE;
                m_touchDragMoveState = 1;
            }
            //如果移动距离大于5则为滑动状态
            if (movingLength > lengthOffset) {
                m_touchState = TS_SLIDE;
            }
            break;
        }
        case TS_DRAG_MOVE: {
            //2 拖拽移动
            qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            qint64 timeOffset = currentTime - m_TouchBeginTime;
            qreal movingLength = QLineF(m_TouchBeginPoint, event->pos()).length();
            //如果移动距离小于5且点击时间大于900毫秒则为长按状态
            if (movingLength < lengthOffset && (timeOffset > 900)) {
                m_touchState = TS_LONG_PRESS;
            }
            if (movingLength > lengthOffset) {
                if (m_touchDragMoveState == 1) {
                    mousePress(m_TouchBeginPoint.toPoint());
                }
                m_touchDragMoveState = 2;
            }
            break;
        }
        case TS_SLIDE: {
            //3 滑动
            QPointF _currentPoint = event->pos();
            slideEvent(m_TouchBeginPoint, _currentPoint);
            break;
        }
        default:
            break;
        }
    }

    if (m_press) {
        emit signalScheduleShow(false);
        m_press = false;
        DragInfoItem::setPressFlag(false);
    }

    if (m_DragStatus == NONE) {
        DragInfoItem *item = dynamic_cast<DragInfoItem *>(itemAt(event->pos()));
        if (item != nullptr) {
            if (isCanDragge(item->getData())) {
                if (m_DragStatus == NONE) {
                    switch (getPosInItem(event->pos(), item->rect())) {
                    case LEFT:
                    case RIGHT:
                        setCursor(Qt::SplitHCursor);
                        break;
                    case TOP:
                    case BOTTOM:
                        setCursor(Qt::SplitVCursor);
                        break;
                    default:
                        setCursor(Qt::ArrowCursor);
                        break;
                    }
                }
            }
        } else {
            if (m_DragStatus == NONE) {
                setCursor(Qt::ArrowCursor);
            }
        }
    } else {
        QDateTime gDate = getPosDate(event->pos());
        switch (m_DragStatus) {
        case IsCreate:
            if (gDate.date().year() >= DDECalendar::QueryEarliestYear && gDate.date().year() <= DDECalendar::QueryLatestYear) {
                if (m_PressDate.date().year() >= DDECalendar::QueryEarliestYear && m_PressDate.date().year() <= DDECalendar::QueryLatestYear) {
                    //如果拖拽创建为false则判断是否可被创建。如果为true则不需要判断
                    m_isCreate = m_isCreate ? m_isCreate : JudgeIsCreate(event->pos());
                    if (m_isCreate) {
                        if (!IsEqualtime(m_MoveDate, gDate)) {
                            m_MoveDate = gDate;
                            m_DragScheduleInfo = getScheduleInfo(m_PressDate, m_MoveDate);
                            upDateInfoShow(IsCreate, m_DragScheduleInfo);
                            //更新背景上显示的item
                            updateBackgroundShowItem();
                            setPressSelectInfo(m_DragScheduleInfo);
                        }
                    }
                }
            }
            break;
        case ChangeBegin:
            if (!IsEqualtime(m_MoveDate, gDate)) {
                m_MoveDate = gDate;
                //获取日程开始时间
                QDateTime _beginTime = getDragScheduleInfoBeginTime(m_MoveDate);
                m_DragScheduleInfo->setDtStart(_beginTime);
                m_DragScheduleInfo->setDtEnd(m_InfoEndTime);
                upDateInfoShow(ChangeBegin, m_DragScheduleInfo);
            }
            break;
        case ChangeEnd:
            if (!IsEqualtime(m_MoveDate, gDate)) {
                m_MoveDate = gDate;
                m_DragScheduleInfo->setDtStart(m_InfoBeginTime);
                //获取结束时间
                QDateTime _endTime = getDragScheduleInfoEndTime(m_MoveDate);
                m_DragScheduleInfo->setDtEnd(_endTime);
                upDateInfoShow(ChangeEnd, m_DragScheduleInfo);
            }
            break;
        case ChangeWhole: {
            if (!m_PressRect.contains(event->pos())) {
                //拖拽前设置是否已经更新日程界面标志为否
                m_hasUpdateMark = false;
                m_Drag->exec(Qt::MoveAction);
                m_Drag = nullptr;
                m_DragStatus = NONE;
                setCursor(Qt::ArrowCursor);
                //如果拖拽结束后没有修改日程则更新下界面日程显示
                if (!m_hasUpdateMark) {
                    updateInfo();
                }
            }
        } break;
        default:
            break;
        }
    }

    DGraphicsView::mouseMoveEvent(event);
}

void DragInfoGraphicsView::wheelEvent(QWheelEvent *event)
{
    stopTouchAnimation();
    DGraphicsView::wheelEvent(event);
}

void DragInfoGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    DGraphicsView::contextMenuEvent(event);
    //如果不为默认状态则不执行右击事件
    if (m_DragStatus != NONE) {
        return;
    }
    emit signalScheduleShow(false);
    m_press = false;
    m_DragStatus = NONE;
    //触摸状态恢复为默认状态
    m_touchState = TS_NONE;
    QGraphicsItem *listItem = itemAt(event->pos());
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(listItem);

    CScheduleItem *tt = dynamic_cast<CScheduleItem *>(listItem);
    if (tt != nullptr && tt->getType() != 0) {
        return;
    }
    if (infoitem != nullptr) {
        //是否为节假日日程判断
        if (!CScheduleOperation::isFestival(infoitem->getData())) {
            m_rightMenu->clear();
            m_rightMenu->addAction(m_editAction);
            m_rightMenu->addAction(m_deleteAction);
            //如果日程是不可修改的则设置删除按钮无效
            m_deleteAction->setEnabled(!CScheduleOperation::scheduleIsInvariant(infoitem->getData()));

            QAction *action_t = m_rightMenu->exec(QCursor::pos());

            if (action_t == m_editAction) {
                CScheduleDlg dlg(0, this);
                dlg.setData(infoitem->getData());
                if (dlg.exec() == DDialog::Accepted) {
                    emit signalsUpdateSchedule();
                }
            } else if (action_t == m_deleteAction) {
                DeleteItem(infoitem->getData());
            }
        } else {
            CMyScheduleView dlg(infoitem->getData(), this);
            dlg.exec();
        }
    } else {
        RightClickToCreate(listItem, event->pos());
    }
}

void DragInfoGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("Info")) {
        QString str = event->mimeData()->data("Info");
        DSchedule::Ptr info;
        DSchedule::fromJsonString(info, str);

        if (info.isNull()) {
            event->ignore();
        }

        //如果该日程是不能被拖拽的则忽略不接受
        //重复日程不能被切换全天和非全天
        if ((event->source() != this && info->recurs()) || !isCanDragge(info)) {
            event->ignore();
        } else {
            event->accept();
            //设置被修改的日程原始信息
            m_PressScheduleInfo = info;
        }
    } else {
        event->ignore();
    }

}

void DragInfoGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    upDateInfoShow();
    m_MoveDate = m_MoveDate.addMonths(-2);
}

void DragInfoGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    QString str = event->mimeData()->data("Info");
    QDateTime gDate =  getPosDate(event->pos());
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(str.toLocal8Bit(), &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        return;
    }

    if (!IsEqualtime(m_MoveDate, gDate)) {
        m_MoveDate = gDate;
        QJsonObject rootobj = jsonDoc.object();
        DSchedule::fromJsonString(m_DragScheduleInfo, str);
        m_DragScheduleInfo->setMoved(true);
        MoveInfoProcess(m_DragScheduleInfo, event->posF());
        DragInfoItem::setPressSchedule(m_DragScheduleInfo);
    }
}

void DragInfoGraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("Info")) {
        if (event->source() != this || m_MoveDate != m_PressDate) {
            //后面方法出现模态框，阻塞，导致拖拽图片不消失，手动先调用取消接口解决
            QDrag::cancel();

            auto startDate = m_DragScheduleInfo->dtStart();
            auto endDate = m_DragScheduleInfo->dtEnd();
            if (startDate.date().year() >= DDECalendar::QueryEarliestYear && endDate.date().year() <= DDECalendar::QueryLatestYear) {
                updateScheduleInfo(m_DragScheduleInfo);
            } else {
                emit signalsUpdateSchedule();
            }
            m_DragStatus = NONE;
            m_MoveDate = m_MoveDate.addMonths(-2);
        }
    }
}

bool DragInfoGraphicsView::event(QEvent *e)
{
    if (e->type() == QEvent::Leave) {
        if (m_DragStatus == IsCreate ||
                m_DragStatus == ChangeBegin ||
                m_DragStatus == ChangeEnd)
            mouseReleaseScheduleUpdate();
    }
    return DGraphicsView::event(e);
}

void DragInfoGraphicsView::keyPressEvent(QKeyEvent *event)
{
    DGraphicsView::keyPressEvent(event);
}

void DragInfoGraphicsView::paintEvent(QPaintEvent *event)
{
    DGraphicsView::paintEvent(event);
    //绘制圆角效果
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_outerBorderColor);
    //左下角绘制圆角补角,颜色与外框背景色一致
    if (m_leftShowRadius) {
        QPainterPath _leftPath;
        _leftPath.moveTo(0, this->height() - m_radius);
        _leftPath.arcTo(0, this->height() - m_radius * 2, m_radius * 2, m_radius * 2, 180, 90);
        _leftPath.lineTo(0, this->height());
        _leftPath.lineTo(0, this->height() - m_radius);
        painter.drawPath(_leftPath);
    }
    //右下角绘制圆角补角
    if (m_rightShowRadius) {
        QPainterPath _rightPath;
        _rightPath.moveTo(this->width() - m_radius, this->height());
        _rightPath.arcTo(this->width() - m_radius * 2, this->height() - m_radius * 2, m_radius * 2, m_radius * 2, 270, 90);
        _rightPath.lineTo(this->width(), this->height());
        _rightPath.lineTo(this->width() - m_radius, this->height());
        painter.drawPath(_rightPath);
        painter.end();
    }
}

void DragInfoGraphicsView::showEvent(QShowEvent *event)
{
    //显示时重置大小
    resize(this->width(), this->height());
    DGraphicsView::showEvent(event);
}

void DragInfoGraphicsView::slotCreate()
{
    slotCreate(m_createDate);
}

/**
 * @brief DragInfoGraphicsView::setPressSelectInfo      设置点击选中日程
 * @param info
 */
void DragInfoGraphicsView::setPressSelectInfo(const DSchedule::Ptr &info)
{
    DragInfoItem::setPressSchedule(info);
}

/**
 * @brief DragInfoGraphicsView::updateScheduleInfo      拖拽更新日程信息
 * @param info
 */
void DragInfoGraphicsView::updateScheduleInfo(const DSchedule::Ptr &info)
{
    QVariant variant;
    //获取主窗口指针
    CalendarGlobalEnv::getGlobalEnv()->getValueByKey("MainWindow", variant);
    QObject *parent = static_cast<QObject *>(variant.value<void *>());
    //设置父类为主窗口
    CScheduleOperation _scheduleOperation(info->scheduleTypeID(), qobject_cast<QWidget *>(parent));
    if (_scheduleOperation.changeSchedule(info, m_PressScheduleInfo)) {
        //如果日程修改成功则更新更新标志
        m_hasUpdateMark = true;
    } else {
        //如果取消更新则主动更新显示
        updateInfo();
    }
}

void DragInfoGraphicsView::DragPressEvent(const QPoint &pos, DragInfoItem *item)
{
    m_PressPos = pos;
    m_PressDate = getPosDate(pos);
    m_MoveDate = m_PressDate.addMonths(-2);

    CalendarGlobalEnv::getGlobalEnv()->reviseValue(DDECalendar::CursorPointKey, mapToGlobal(pos));

    if (item != nullptr) {
        PosInItem mpressstatus = getPosInItem(pos, item->boundingRect());
        if (mpressstatus != MIDDLE && !isCanDragge(item->getData())) {
            return;
        }
        //拖拽使用副本，不更改原始日程
        m_DragScheduleInfo.reset(item->getData()->clone());
        m_PressScheduleInfo = item->getData();
        m_InfoBeginTime = m_DragScheduleInfo->dtStart();
        m_InfoEndTime = m_DragScheduleInfo->dtEnd();
        switch (mpressstatus) {
        case TOP:
            m_DragStatus = ChangeBegin;
            setCursor(Qt::SplitVCursor);
            break;
        case BOTTOM:
            m_DragStatus = ChangeEnd;
            setCursor(Qt::SplitVCursor);
            break;
        case LEFT:
            m_DragStatus = ChangeBegin;
            setCursor(Qt::SplitHCursor);
            break;
        case RIGHT:
            m_DragStatus = ChangeEnd;
            setCursor(Qt::SplitHCursor);
            break;
        default:
            ShowSchedule(item);
            m_DragStatus = ChangeWhole;
            QMimeData *mimeData = new QMimeData();
            mimeData->setText(m_DragScheduleInfo->summary());
            QString strData;
            DSchedule::toJsonString(m_DragScheduleInfo, strData);
            //数据转换
            mimeData->setData("Info", strData.toUtf8());

            if (m_Drag == nullptr) {
                m_Drag = new QDrag(this);
            }
            m_Drag->setMimeData(mimeData);
            QPointF itemPos = QPointF(pos.x() - item->boundingRect().x(),
                                      pos.y() - item->boundingRect().y());
            m_Drag->setHotSpot(itemPos.toPoint());
            setDragPixmap(m_Drag, item);
            break;
        }
    } else {
        m_DragStatus = IsCreate;
        m_isCreate = false;
    }
}

/**
 * @brief DragInfoGraphicsView::mouseReleaseScheduleUpdate      鼠标左击释放数据处理
 */
void DragInfoGraphicsView::mouseReleaseScheduleUpdate()
{
    setCursor(Qt::ArrowCursor);
    m_press = false;
    DragInfoItem::setPressFlag(false);

    switch (m_DragStatus) {
    case IsCreate:
        if (MeetCreationConditions(m_MoveDate)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            CScheduleDlg dlg(1, this);
            dlg.setData(m_DragScheduleInfo);
            //如果取消新建则主动刷新日程信息
            dlg.exec();
            //因dtk override了exec函数，这里使用result判断返回值类型，如果不为Accepted则刷新界面
            if (dlg.result() != DDialog::Accepted) {
                updateInfo();
            }
            //设置选中日程为无效日程
            setPressSelectInfo(DSchedule::Ptr());
        }
        break;
    case ChangeBegin:
        if (!IsEqualtime(m_MoveDate, m_InfoBeginTime)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            updateScheduleInfo(m_DragScheduleInfo);
        }
        break;
    case ChangeEnd:
        if (!IsEqualtime(m_MoveDate, m_InfoEndTime)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            updateScheduleInfo(m_DragScheduleInfo);
        }
        break;
    default:
        break;
    }
    m_DragStatus = NONE;
    update();
}

void DragInfoGraphicsView::mousePress(const QPoint &point)
{
    setPressSelectInfo(DSchedule::Ptr());
    //设置拖拽日程为无效日程
    m_DragScheduleInfo = DSchedule::Ptr();
    pressScheduleInit();
    QGraphicsItem *listItem = itemAt(point);
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(listItem);
    //不满足拖拽条件的日程不进行拖拽事件
    if (infoitem) {
        setPressSelectInfo(infoitem->getData());
        m_PressScheduleInfo = infoitem->getData();
        m_press = true;
        if (isCanDragge(infoitem->getData())) {
            //满足拖拽条件
            DragInfoItem::setPressFlag(true);
            DragPressEvent(point, infoitem);
        } else {
            //不满足拖拽条件，只展示信息弹窗
            m_PressPos = point;
            m_PressDate = getPosDate(point);
            CalendarGlobalEnv::getGlobalEnv()->reviseValue(DDECalendar::CursorPointKey, mapToGlobal(point));
            ShowSchedule(infoitem);
        }
    } else {
        //没有日程信息，可滑动
        emit signalScheduleShow(false);
        DragPressEvent(point, infoitem);
    }
    this->scene()->update();
    update();
}

int DragInfoGraphicsView::getSlidePos() const
{
    return m_touchSlidePos;
}

void DragInfoGraphicsView::setSlidePos(int pos)
{
    m_touchSlidePos = pos;
    verticalScrollBar()->setValue(m_touchSlidePos);
}

void DragInfoGraphicsView::stopTouchAnimation()
{
    m_touchAnimation->stop();
}

/**
 * @brief DragInfoGraphicsView::DeleteItem      删除日程
 * @param info
 */
void DragInfoGraphicsView::DeleteItem(const DSchedule::Ptr &info)
{
    if (info.isNull()) return;
    //删除日程
    CScheduleOperation _scheduleOperation(info->scheduleTypeID(), this);
    _scheduleOperation.deleteSchedule(info);
}

/**
 * @brief DragInfoGraphicsView::setSelectSearchSchedule     设置选中搜索日程
 * @param scheduleInfo
 */
void DragInfoGraphicsView::setSelectSearchSchedule(const DSchedule::Ptr &scheduleInfo)
{
    setPressSelectInfo(scheduleInfo);
}

void DragInfoGraphicsView::setDragPixmap(QDrag *drag, DragInfoItem *item)
{
    Q_UNUSED(item);
    //设置一个1*1的透明图片，要不然关闭窗口特效会有一个小黑点
    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::transparent);
    drag->setPixmap(pixmap);
    //设置图标位置为鼠标位置
    drag->setHotSpot(QPoint(0, 0));
}

void DragInfoGraphicsView::slotCreate(const QDateTime &date)
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
    }
}

DSchedule::Ptr DragInfoGraphicsView::getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
{
    DSchedule::Ptr info;
    info.reset(new DSchedule());
    if (beginDate.daysTo(endDate) > 0) {
        info->setDtStart(QDateTime(beginDate.date(), QTime(0, 0, 0)));
        info->setDtEnd(QDateTime(endDate.date(), QTime(23, 59, 59)));
    } else {
        info->setDtStart(QDateTime(endDate.date(), QTime(0, 0, 0)));
        info->setDtEnd(QDateTime(beginDate.date(), QTime(23, 59, 00)));
    }
    info->setSummary(tr("New Event"));
    info->setAllDay(true);
    //设置默认日程类型为工作
    info->setScheduleTypeID("107c369e-b13a-4d45-9ff3-de4eb3c0475b");
    //设置提醒规则
    info->setAlarmType(DSchedule::Alarm_15Hour_Front);
    return info;
}

void DragInfoGraphicsView::ShowSchedule(DragInfoItem *infoitem)
{
    if (infoitem == nullptr)
        return;
    emit signalScheduleShow(true, infoitem->getData());

}

void DragInfoGraphicsView::setTheMe(int type)
{
    Q_UNUSED(type);
    DPalette _painte;
    //获取外框背景色
    m_outerBorderColor = _painte.color(QPalette::Active, QPalette::Window);
}

void DragInfoGraphicsView::slideEvent(QPointF &startPoint, QPointF &stopPort)
{
    qreal _movingLine {0};
    //获取滑动方向
    touchGestureOperation::TouchMovingDirection _touchMovingDir =
        touchGestureOperation::getTouchMovingDir(startPoint, stopPort, _movingLine);
    m_touchMovingDir = _touchMovingDir;
    //切换标志 0 不切换  1 下一页  -1 上一页
    int delta {0};
    //移动偏移 25则切换
    const int moveOffset = 25;
    switch (_touchMovingDir) {
    case touchGestureOperation::T_TOP:
    case touchGestureOperation::T_BOTTOM: {
        const int pos_Diff_Y = qFloor(stopPort.y() - startPoint.y());
        verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition() - pos_Diff_Y);
        startPoint = stopPort;
        break;
    }
    case touchGestureOperation::T_LEFT: {
        if (_movingLine > moveOffset) {
            delta = 1;
            startPoint = stopPort;
        }
        break;
    }
    case touchGestureOperation::T_RIGHT: {
        if (_movingLine > moveOffset) {
            delta = -1;
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

/**
 * @brief DragInfoGraphicsView::updateInfo      更新日程数据显示
 */
void DragInfoGraphicsView::updateInfo()
{
    //如果拖拽日程有效则更新为不是移动日程
    if (m_DragScheduleInfo && m_DragScheduleInfo->isValid() && m_DragScheduleInfo->uid() != "0") {
        m_DragScheduleInfo->setMoved(false);
        //设置选择日程状态
        setPressSelectInfo(m_DragScheduleInfo);
    }
}

int DragInfoGraphicsView::getDragStatus() const
{
    return m_DragStatus;
}

void DragInfoGraphicsView::setShowRadius(bool leftShow, bool rightShow)
{
    m_leftShowRadius = leftShow;
    m_rightShowRadius = rightShow;
}

bool DragInfoGraphicsView::isCanDragge(const DSchedule::Ptr &info)
{
    if (info.isNull() || info->scheduleTypeID().isEmpty())
        return false;
    //是否为节假日日程判断
    if (CScheduleOperation::isFestival(info))
        return false;
    if (info->lunnar() && !QLocale::system().name().startsWith("zh_"))
        return false;
    //日程不可被修改
    if (CScheduleOperation::scheduleIsInvariant(info)) {
        return false;
    }
    return true;
}

/**
 * @brief DragInfoGraphicsView::slotDeleteItem      删除日程
 */
void DragInfoGraphicsView::slotDeleteItem()
{
    //获取选中日程
    DSchedule::Ptr _pressSchedule = DragInfoItem::getPressSchedule();

    //根据焦点状态获取当前焦点的item
    CSceneBackgroundItem *backgroundItem = dynamic_cast<CSceneBackgroundItem *>(m_Scene->getCurrentFocusItem());
    if (backgroundItem != nullptr) {
        DragInfoItem *dFocusItem = dynamic_cast<DragInfoItem *>(backgroundItem->getFocusItem());
        if (dFocusItem != nullptr && dFocusItem->getItemType() == CFocusItem::CITEM) {
            if (dFocusItem->getItemFocus()) {
                _pressSchedule = dFocusItem->getData();
            }
        }
    }

    //判断是否有效,如果为有效日程且日程类型不为节日或纪念日或不可更改日程则删除
    //判断日程信息
    if (!_pressSchedule.isNull() && _pressSchedule->isValid()
            && !CScheduleOperation::isFestival(_pressSchedule)
            && !CScheduleOperation::scheduleIsInvariant(_pressSchedule)) {
        CScheduleOperation _scheduleOperation(_pressSchedule->scheduleTypeID(), this);
        _scheduleOperation.deleteSchedule(_pressSchedule);
        //设置选择日程为无效日程
        setPressSelectInfo(DSchedule::Ptr());
        //设置拖拽日程为无效日程
        m_DragScheduleInfo = DragInfoItem::getPressSchedule();
    }
}

void DragInfoGraphicsView::slotSwitchPrePage(const QDate &focusDate, bool isSwitchView)
{
    emit signalSwitchPrePage();
    if (isSwitchView) {
        m_Scene->signalSwitchView(focusDate, true);
    } else {
        setSceneCurrentItemFocus(focusDate);
    }
}

void DragInfoGraphicsView::slotSwitchNextPage(const QDate &focusDate, bool isSwitchView)
{
    emit signalSwitchNextPage();
    //如果需要切换视图则
    if (isSwitchView) {
        m_Scene->signalSwitchView(focusDate, true);
    } else {
        setSceneCurrentItemFocus(focusDate);
    }
}

void DragInfoGraphicsView::slotContextMenu(CFocusItem *item)
{
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(item);
    if (infoitem != nullptr) {
        //如果为节假日则退出不展示右击菜单
        if (CScheduleOperation::isFestival(infoitem->getData()))
            return;
        //快捷键调出右击菜单
        m_Scene->setIsContextMenu(true);
        m_rightMenu->clear();
        m_rightMenu->addAction(m_editAction);
        m_rightMenu->addAction(m_deleteAction);
        //如果日程不可修改则设置删除无效
        m_deleteAction->setEnabled(!CScheduleOperation::scheduleIsInvariant(infoitem->getData()));
        QPointF itemPos = QPointF(infoitem->rect().x() + infoitem->rect().width() / 2, infoitem->rect().y() + infoitem->rect().height() / 2);
        QPointF scene_pos = infoitem->mapToScene(itemPos);
        QPointF view_pos = mapFromScene(scene_pos);
        QPoint screen_pos = mapToGlobal(view_pos.toPoint());
        QAction *action_t = m_rightMenu->exec(screen_pos);

        if (action_t == m_editAction) {
            CScheduleDlg dlg(0, this);
            dlg.setData(infoitem->getData());
            if (dlg.exec() == DDialog::Accepted) {
                emit signalsUpdateSchedule();
            }
        } else if (action_t == m_deleteAction) {
            DeleteItem(infoitem->getData());
        }
        m_Scene->setIsContextMenu(false);
        m_Scene->currentFocusItemUpdate();
    }
}

/**
 * @brief DragInfoGraphicsView::slotsetNextFocus    切换到下一个焦点
 */
void DragInfoGraphicsView::slotsetNextFocus()
{
    focusNextPrevChild(true);
}

void DragInfoGraphicsView::setSceneCurrentItemFocus(const QDate &focusDate)
{
    Q_UNUSED(focusDate);
}

/**
 * @brief DragInfoGraphicsView::pressScheduleInit 初始化点击日程
 */
void DragInfoGraphicsView::pressScheduleInit()
{
    m_PressScheduleInfo = DSchedule::Ptr();
}

QDate DragInfoGraphicsView::getCurrentDate() const
{
    return m_currentDate;
}

void DragInfoGraphicsView::setCurrentDate(const QDate &currentDate)
{
    m_currentDate = currentDate;
}
