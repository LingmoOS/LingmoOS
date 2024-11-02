#include "cutwidget.h"

CutWIdget::CutWIdget(QWidget *parent) : QWidget(parent)
{
    initInteraction();
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
}

void CutWIdget::sizeChange(QSize currSize)
{
    this->resize(currSize);
}

void CutWIdget::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::releaseRegion, this,
            &CutWIdget::cutRegion); //裁剪区域的起点和终点
    connect(Interaction::getInstance(), &Interaction::cutSaveFinish, this, &CutWIdget::cutFinish); //裁剪完成-保存完成
    connect(Interaction::getInstance(), &Interaction::returnMouseShape, this,
            &CutWIdget::resetMouseType); //设置鼠标类型
}

//发送鼠标移动坐标
void CutWIdget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QPoint currpos = this->mapFromGlobal(QCursor().pos());
    if (m_isPress) {
        if (m_isCutRegion == false) {
            m_paintProcessing = true;
            //在裁剪框外部-或第一次时
            Interaction::getInstance()->cutRegion(currpos);
        }
    }
    //判断已经开始裁剪
    if (!(m_startPoint.x() == -1 && m_startPoint.y() == -1)) {
        //裁剪进行中
        m_isCutRegion = true;
        //改变鼠标形状
        changeMouseType(currpos);
        m_fixType = false;
    }

    //发信号给后端进行绘图
    if (m_isCutRegion && m_isPress) {
        Interaction::getInstance()->releaseCutChange(m_type, currpos);
    }
    QWidget::mouseMoveEvent(event);
}
//发送鼠标按下坐标
void CutWIdget::mousePressEvent(QMouseEvent *event)
{
    //判断左键
    if (event->button() == Qt::LeftButton) {
        this->setCursor(Qt::ClosedHandCursor);
        m_isPress = true;
        QPoint currpos = this->mapFromGlobal(QCursor().pos());
        m_enter = currpos;
        Interaction::getInstance()->startChange(currpos);
        Q_EMIT showBtnWid(false);
        if (m_isCutRegion) {
            if (m_startPoint.x() != -1 && m_startPoint.y() != -1) {
                //发送鼠标按下的一瞬间的位置--目的是计算此时的与相对的起点的偏移量
                m_cutEnterPos = currpos;
            }
        } else {
            m_startEnterPoint = currpos;
        }
    }

    QWidget::mousePressEvent(event);
}

void CutWIdget::mouseReleaseEvent(QMouseEvent *event)
{
    //判断非左键时要return，右键释放会触发释放事件，导致界面异常
    if (event->button() != Qt::LeftButton) {
        return;
    }
    QPoint currpos = this->mapFromGlobal(QCursor().pos());
    m_release = currpos;
    m_isPress = false;
    this->setCursor(Qt::CrossCursor);
    if (m_release != m_enter) {
        Interaction::getInstance()->releasePos(currpos);
        Q_EMIT showBtnWid(true);
    } else if (m_isCutRegion) {
        Q_EMIT showBtnWid(true);
    } else {
        //其他-暂不处理，需要时可进行处理
        Interaction::getInstance()->clickCutFullViableImage();
        Q_EMIT showBtnWid(true);
    }
    m_startEnterPoint = QPoint(-1, -1);
    m_cutEnterPos = QPoint(-1, -1);
    m_xOffest = 0;
    m_yOffest = 0;
    m_fixType = true; //更改状态
}
//处理鼠标样式
void CutWIdget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    this->setCursor(Qt::ArrowCursor);
    QWidget::leaveEvent(event);
}
//处理鼠标样式
void CutWIdget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    this->setCursor(Qt::CrossCursor);
    QWidget::enterEvent(event);
}

bool CutWIdget::touchBeginEventProcess(QEvent *event)
{
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    QList<QTouchEvent::TouchPoint> touchStartPoints = touchEvent->touchPoints();

    //touch开始触控的点坐标
    QPoint currpos = this->mapFromGlobal(touchStartPoints.at(0).screenPos().toPoint());
    m_isPress = true;
    m_enter = currpos;
    Interaction::getInstance()->startChange(currpos);
    Q_EMIT showBtnWid(false);
    if (m_isCutRegion) {
        if (m_startPoint.x() != -1 && m_startPoint.y() != -1) {
            m_cutEnterPos = currpos;
        }
    } else {
        m_startEnterPoint = currpos;
    }

    touchEvent->accept();

    return true;
}

bool CutWIdget::touchUpdateEventProcess(QEvent *event)
{
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    QList<QTouchEvent::TouchPoint> touchUpdatePoints = touchEvent->touchPoints();

    //设置单指触控
    if (TOUCH_ONE_POINT == touchUpdatePoints.count()) {
        QPoint currpos = this->mapFromGlobal(touchUpdatePoints.at(0).screenPos().toPoint());
        QPoint lastPoint = this->mapFromGlobal(touchUpdatePoints.at(0).lastScreenPos().toPoint());
        //touch结束触控的点坐标
        m_release = lastPoint;
        if (m_isPress) {
            if (m_isCutRegion == false) {
                m_paintProcessing = true;
                Interaction::getInstance()->cutRegion(currpos);
            }
        }

        if (!(m_startPoint.x() == -1 && m_startPoint.y() == -1)) {
            m_isCutRegion = true;
            changeMouseType(currpos);
            m_fixType = false;
        }
        if (m_isCutRegion && m_isPress) {
            Interaction::getInstance()->releaseCutChange(m_type, currpos);
        }
    }

    return true;
}

bool CutWIdget::touchEndEventProcess(QEvent *event)
{
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);

    m_isPress = false;
    if (m_release != m_enter) {
        Interaction::getInstance()->releasePos(m_release);
        Q_EMIT showBtnWid(true);
    } else if (m_isCutRegion) {
        Q_EMIT showBtnWid(true);
    } else {
        Interaction::getInstance()->clickCutFullViableImage();
        Q_EMIT showBtnWid(true);
    }
    m_startEnterPoint = QPoint(-1, -1);
    m_cutEnterPos = QPoint(-1, -1);
    m_xOffest = 0;
    m_yOffest = 0;
    m_fixType = true; 

    touchEvent->accept();

    return true;
}
void CutWIdget::cutRegion(QPoint start, QPoint end)
{
    m_startPoint = start;
    m_endPoint = end;
}

void CutWIdget::changeMouseType(QPoint currpos)
{

    //在内部
    if (currpos.x() > m_startPoint.x() + CUT_DISTANCE && currpos.x() < m_endPoint.x() - CUT_DISTANCE
        && currpos.y() > m_startPoint.y() + CUT_DISTANCE && currpos.y() < m_endPoint.y() - CUT_DISTANCE) {
        //移动-0
        if (m_isPress) {
            m_type = INSIDE;
        } else {
            this->setCursor(Qt::ClosedHandCursor);
        }
    } else if ((currpos.y() >= m_startPoint.y() - CUT_DISTANCE && currpos.y() <= m_startPoint.y() + CUT_DISTANCE)
               && (currpos.x() > m_startPoint.x() + CUT_DISTANCE
                   && currpos.x() < m_endPoint.x() - CUT_DISTANCE)) { //在裁剪框现上
                                                                      //在上边线-1
        if (m_isPress) {
            m_type = TOP;
        } else {
            this->setCursor(Qt::SizeVerCursor);
        }
    } else if ((currpos.y() >= m_endPoint.y() - CUT_DISTANCE && currpos.y() <= m_endPoint.y() + CUT_DISTANCE)
               && (currpos.x() > m_startPoint.x() + CUT_DISTANCE
                   && currpos.x() < m_endPoint.x() - CUT_DISTANCE)) { //在裁剪框现上
                                                                      //下边线上-7
        if (m_isPress) {
            m_type = BOTTOM;
        } else {
            this->setCursor(Qt::SizeVerCursor);
        }
    } else if (((currpos.x() >= m_startPoint.x() - CUT_DISTANCE && currpos.x() <= m_startPoint.x() + CUT_DISTANCE))
               && (currpos.y() > m_startPoint.y() + CUT_DISTANCE
                   && currpos.y() < m_endPoint.y() - CUT_DISTANCE)) { //在裁剪框现上
                                                                      //在左边线
        if (m_isPress) {
            m_type = LEFT;
        } else {
            this->setCursor(Qt::SizeHorCursor);
        }
    } else if (((currpos.x() >= m_endPoint.x() - CUT_DISTANCE && currpos.x() <= m_endPoint.x() + CUT_DISTANCE))
               && (currpos.y() > m_startPoint.y() + CUT_DISTANCE
                   && currpos.y() < m_endPoint.y() - CUT_DISTANCE)) { //在裁剪框现上
                                                                      //右边线上
        if (m_isPress) {
            m_type = RIGHT;
        } else {
            this->setCursor(Qt::SizeHorCursor);
        }
    } else if ((currpos.y() >= m_startPoint.y() - CUT_DISTANCE && currpos.y() <= m_startPoint.y() + CUT_DISTANCE)
               && (currpos.x() >= m_startPoint.x() - CUT_DISTANCE
                   && currpos.x() <= m_startPoint.x() + CUT_DISTANCE)) { //起始点
        //左上角-3
        if (m_isPress) {
            m_type = LEFT_TOP;
        } else {
            this->setCursor(Qt::SizeFDiagCursor);
        }
    } else if ((currpos.y() >= m_endPoint.y() - CUT_DISTANCE && currpos.y() <= m_endPoint.y() + CUT_DISTANCE)
               && (currpos.x() >= m_endPoint.x() - CUT_DISTANCE
                   && currpos.x() <= m_endPoint.x() + CUT_DISTANCE)) { //在终点
        //右下角-4
        if (m_isPress) {
            m_type = RIGHT_BOTTOM;
        } else {
            this->setCursor(Qt::SizeFDiagCursor);
        }
    } else if ((currpos.y() >= m_endPoint.y() - CUT_DISTANCE && currpos.y() <= m_endPoint.y() + CUT_DISTANCE)
               && (currpos.x() >= m_startPoint.x() - CUT_DISTANCE
                   && currpos.x() <= m_startPoint.x() + CUT_DISTANCE)) { //在左下角
        //左下角-5
        if (m_isPress) {
            m_type = LEFT_BOTTOM;
        } else {
            this->setCursor(Qt::SizeBDiagCursor);
        }
    } else if (((currpos.y() >= m_startPoint.y() - CUT_DISTANCE && currpos.y() <= m_startPoint.y() + CUT_DISTANCE)
                && (currpos.x() >= m_endPoint.x() - CUT_DISTANCE
                    && currpos.x() <= m_endPoint.x() + CUT_DISTANCE))) { //在右上角
        //右上角-6
        if (m_isPress) {
            m_type = RIGHT_TOP;
        } else {
            this->setCursor(Qt::SizeBDiagCursor);
        }
    } else { //在裁剪框外部
        if (m_isPress) {
            m_type = OUTSIDE;
        } else {
            this->setCursor(Qt::CrossCursor);
        }
    }
}

void CutWIdget::resetMouseType(int type)
{
    if (type == INSIDE) {
        this->setCursor(Qt::ClosedHandCursor);
    } else if (type == TOP || type == BOTTOM) {
        this->setCursor(Qt::SizeVerCursor);
    } else if (type == LEFT || type == RIGHT) {
        this->setCursor(Qt::SizeHorCursor);
    } else if (type == LEFT_TOP) {
        this->setCursor(Qt::SizeFDiagCursor);
    } else if (type == RIGHT_BOTTOM) {
        this->setCursor(Qt::SizeFDiagCursor);
    } else if (type == LEFT_BOTTOM) {
        this->setCursor(Qt::SizeBDiagCursor);
    } else if (type == RIGHT_TOP) {
        this->setCursor(Qt::SizeBDiagCursor);
    } else {
        this->setCursor(Qt::CrossCursor);
    }
}

void CutWIdget::cutFinish()
{
    m_isCutRegion = false;
    m_startPoint = QPoint(-1, -1);
    m_endPoint = QPoint(-1, -1);
}
