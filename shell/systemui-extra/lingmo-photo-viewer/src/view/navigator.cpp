#include "navigator.h"
#include "kyview.h"
Navigator::Navigator(QWidget *parent) : QWidget(parent)
{
    this->resize(Variable::NAVIGATION_SIZE);
    this->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.4);}");
    m_bottomImage = new QLabel(this);
    m_bottomImage->setAlignment(Qt::AlignCenter);
    m_bottomImage->resize(this->width(), this->height());
    m_bottomImage->move(0, 0);
    m_bottomImage->setMouseTracking(true);
    this->setMouseTracking(true);
    //此处绑定信号和槽
    connect(Interaction::getInstance(), &Interaction::showNavigation, this,
            &Navigator::showNavigation); //启动时打开图片
    //    connect(this, &Navigator::posChange, Interaction::getInstance(), &Interaction::clickNavigation);
    connect(this, &Navigator::posChange, Interaction::getInstance(), &Interaction::qmlClickNaviDisplay);
    connect(Interaction::getInstance(), &Interaction::sendHightlightPos, this, &Navigator::getHighLightRegion);
}
//显示导航器
void Navigator::showNavigation(QPixmap pix)
{
    if (pix.isNull()) {
        this->hide();
        Q_EMIT navigatorState(false);
        return;
    }
    m_bottomImage->setPixmap(pix);
    if (this->isHidden()) {
        this->show();
        Q_EMIT navigatorState(true);
        Q_EMIT naviChange();
    }
}
//发送鼠标移动坐标
void Navigator::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QPointF currpos = this->mapFromGlobal(QCursor().pos());

    if (m_isPress) {
        Q_EMIT notifyNotUpdated(true);
        Q_EMIT posChange(currpos, true);
        Q_EMIT needUpdateImagePosFromMainWid(false);
        this->setCursor(Qt::ClosedHandCursor);
    } else {
        if ((currpos.x() >= m_startPos.x() && currpos.x() <= m_endPos.x())
            && (currpos.y() >= m_startPos.y() && currpos.y() <= m_endPos.y())) {
            this->setCursor(Qt::ClosedHandCursor);
        } else {
            this->unsetCursor();
        }
    }

    QWidget::mouseMoveEvent(event);
}
//发送鼠标按下坐标
void Navigator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF currpos = this->mapFromGlobal(QCursor().pos());
        Q_EMIT posChange(currpos, true);
        Q_EMIT notifyNotUpdated(true);
        Q_EMIT needUpdateImagePosFromMainWid(false);
    }
    m_isPress = true;
    this->setCursor(Qt::ClosedHandCursor);
    QWidget::mousePressEvent(event);
}

void Navigator::mouseReleaseEvent(QMouseEvent *event)
{
    Q_EMIT notifyNotUpdated(false);
    m_isPress = false;
    QPointF currpos = this->mapFromGlobal(QCursor().pos());
    if ((currpos.x() >= m_startPos.x() && currpos.x() <= m_endPos.x())
        && (currpos.y() >= m_startPos.y() && currpos.y() <= m_endPos.y())) {
        this->setCursor(Qt::ClosedHandCursor);
    } else {
        this->unsetCursor();
    }
    Q_EMIT needUpdateImagePosFromMainWid(true);
    Q_EMIT needUpdateQmlImagePos(true);
    //判断在区域内，鼠标不变，否则，鼠标变化
    QWidget::mouseReleaseEvent(event);
}

void Navigator::getHighLightRegion(QPoint startPos, QPoint endPos)
{
    m_startPos = startPos;
    m_endPos = endPos;
}
