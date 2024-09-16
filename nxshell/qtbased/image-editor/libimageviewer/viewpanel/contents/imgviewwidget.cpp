// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imgviewwidget.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "imgviewlistview.h"
#include "accessibility/ac-desktop-define.h"

#include <QTimer>
#include <QScroller>
#include <QScrollBar>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainterPath>
#include <QAbstractItemModel>
#include <QtMath>

#include <DLabel>
#include <DImageButton>
#include <DThumbnailProvider>
#include <DApplicationHelper>
#include <DSpinner>

#include "service/commonservice.h"

DWIDGET_USE_NAMESPACE

#define LEFT_LISTVIEW_MOVE_MAXLEN 60
#define RIGHT_LISTVIEW_MOVE_UPDATE 35

MyImageListWidget::MyImageListWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    this->setLayout(hb);

    m_listview = new LibImgViewListView(this);
    m_listview->setObjectName("ImgViewListView");
    //    hb->addWidget(m_listview);
    m_listview->viewport()->installEventFilter(this);
    // 设置一个可以放置的高度
    m_listview->viewport()->setFixedHeight(90);
    connect(m_listview, &LibImgViewListView::clicked, this, &MyImageListWidget::onClicked);
    //    connect(m_listview->selectionModel(), &QItemSelectionModel::selectionChanged,
    //            this, &MyImageListWidget::ONselectionChanged);

    connect(m_listview, &LibImgViewListView::openImg, this, &MyImageListWidget::openImg);
    connect(m_listview->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MyImageListWidget::onScrollBarValueChanged);
    initAnimation();

    // 解决释放到程序外,不会动画的问题
    connect(LibCommonService::instance(), &LibCommonService::sigRightMousePress, this, [=]
            {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - 100 > m_lastReleaseTime)
        {
            m_lastReleaseTime = currentTime;
            animationStart(true, 0, 400);
        } });
}
#include "service/imagedataservice.h"
bool MyImageListWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Leave)
    {
        qDebug() << "QEvent::Leave" << obj;
    }
    if (e->type() == QEvent::MouseButtonPress)
    {
        if (!isEnabled())
        {
            return true;
        }

        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
        m_pressPoint = mouseEvent->globalPos();
        m_movePoint = mouseEvent->globalPos();
        qDebug() << m_movePoint;
        m_moveViewPoint = mouseEvent->globalPos();

        m_timer->start();
        m_movePoints.clear();

        m_preListGeometryLeft = m_listview->geometry().left();

        m_listview->update();
        qDebug() << "------------getCount = " << LibImageDataService::instance()->getCount();
    }
    if (e->type() == QEvent::MouseButtonRelease)
    {
        if (!isEnabled())
        {
            return true;
        }

        if (m_movePoints.size() > 0)
        {
            int endPos = m_movePoints.last().x() - m_movePoints.first().x();
            // 过滤掉触屏点击时的move误操作
            if (abs(m_movePoints.last().x() - m_movePoints.first().x()) > 15)
            {
                animationStart(false, endPos, 500);
            }
            else
            {
                animationStart(true, 0, 400);
            }
        }
        // 松手，动画重置按钮应该所在的位置
        //        animationStart(true, 0, 400);
    }
    if (e->type() == QEvent::MouseMove || e->type() == QEvent::TouchUpdate)
    {
        if (!isEnabled())
        {
            return true;
        }

        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
        if (!mouseEvent)
        {
            return false;
        }
        QPoint p = mouseEvent->globalPos();
        if (m_movePoints.size() < 20)
        {
            m_movePoints.push_back(p);
        }
        else
        {
            m_movePoints.pop_front();
            m_movePoints.push_back(p);
        }
        // 限制缩略图List的移动范围
        int moveRangeX = m_listview->x() + p.x() - m_moveViewPoint.x();
        int rowWidth = m_listview->getRowWidth();
        int offsetRowMove = rowWidth - ((rowWidth / RIGHT_LISTVIEW_MOVE_UPDATE) > 14 ? 12 : (rowWidth / RIGHT_LISTVIEW_MOVE_UPDATE - 3)) * RIGHT_LISTVIEW_MOVE_UPDATE;
        moveRangeX = moveRangeX > LEFT_LISTVIEW_MOVE_MAXLEN ? LEFT_LISTVIEW_MOVE_MAXLEN : moveRangeX;
        moveRangeX = moveRangeX < (LEFT_LISTVIEW_MOVE_MAXLEN - offsetRowMove) ? (LEFT_LISTVIEW_MOVE_MAXLEN - offsetRowMove) : moveRangeX;
        m_listview->move(moveRangeX, m_listview->y());
        m_moveViewPoint = p;

        // 目的为了获取moveX的值,中心离当前位置的差值
        int moveX = 0;
        int middle = (this->geometry().right() - this->geometry().left()) / 2;
        int itemX = m_listview->x() + m_listview->getCurrentItemX() + 31;
        if (rowWidth - m_listview->getCurrentItemX() < (this->geometry().width() / 2))
        {
            moveX = this->geometry().width() - rowWidth - m_listview->x();
        }
        else if (m_listview->getCurrentItemX() < (this->geometry().width() / 2))
        {
            moveX = 0 - m_listview->pos().x();
        }
        else if (m_listview->geometry().width() <= this->width())
        {
            moveX = 0;
        }
        else
        {
            moveX = middle - itemX;
        }
        // moveX > 32或moveX <- 32则滑动两次(代表没有居中)
        // 如果这一次的移动并没有超过很大范围32-50,向右切换,并且m_movePoint位置记录为移动了32,这样的话可以保证每次切换流畅
        if (m_listview->x() < 0 && m_movePoint.x() - p.x() >= 32 && m_movePoint.x() - p.x() <= 50)
        {
            m_listview->openNext();
            m_movePoint = QPoint(m_movePoint.x() - 32, m_movePoint.y());
            if (moveX > 32)
            {
                m_listview->openNext();
            }
        }
        // 如果这一次的移动并超过很大范围>50,向右切换,并且m_movePoint位置记录为移动了到了当前的位置,目的是在最前面滑动的时候做到始终显示在屏幕上,到开头才滑动
        else if (m_listview->x() < 0 && m_movePoint.x() - p.x() > 32)
        {
            m_listview->openNext();
            m_movePoint = QPoint(p.x(), m_movePoint.y());
            if (moveX > 32)
            {
                m_listview->openNext();
            }
        }
        // 同上,32-50之间则m_movePoint移动32
        else if ((rowWidth - m_listview->getCurrentItemX() - moveX) > 0 && m_movePoint.x() - p.x() <= -32 && m_movePoint.x() - p.x() >= -50)
        {
            m_listview->openPre();
            m_movePoint = QPoint(m_movePoint.x() + 32, m_movePoint.y());
            if (moveX < -32)
            {
                m_listview->openPre();
            }
        }
        //>50,超过边界则不移动,框框要在屏幕范围内
        else if ((rowWidth - m_listview->getCurrentItemX() - moveX) > 0 && m_movePoint.x() - p.x() <= -32)
        {
            m_listview->openPre();
            m_movePoint = QPoint(p.x(), m_movePoint.y());
            if (moveX < -32)
            {
                m_listview->openPre();
            }
        }

        return true;
    }
    return QWidget::eventFilter(obj, e);
}

void MyImageListWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void MyImageListWidget::resizeEvent(QResizeEvent *event)
{
    // resize之后需要重新找到中心点
    moveCenterWidget();
    Q_UNUSED(event);
}
MyImageListWidget::~MyImageListWidget()
{
    if (m_resetAnimation != nullptr)
    {
        m_resetAnimation->deleteLater();
    }
}

void MyImageListWidget::setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path)
{
    m_listview->setAllFile(itemInfos, path);
    this->setVisible(itemInfos.size() > 1);
    setSelectCenter();
    emit openImg(m_listview->getSelectIndexByPath(path), path);
}

imageViewerSpace::ItemInfo MyImageListWidget::getImgInfo(QString path)
{
    imageViewerSpace::ItemInfo info;
    for (int i = 0; i < m_listview->m_model->rowCount(); i++)
    {
        QModelIndex indexImg = m_listview->m_model->index(i, 0);
        imageViewerSpace::ItemInfo infoImg = indexImg.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (infoImg.path == path)
        {
            info = infoImg;
            break;
        }
    }
    return info;
}

imageViewerSpace::ItemInfo MyImageListWidget::getCurrentImgInfo()
{
    imageViewerSpace::ItemInfo infoImg;
    if (m_listview->m_currentRow < m_listview->m_model->rowCount())
    {
        QModelIndex indexImg = m_listview->m_model->index(m_listview->m_currentRow, 0);
        infoImg = indexImg.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    }
    return infoImg;
}
// 将选中项移到最前面，后期可能有修改，此时获取的列表宽度不正确
void MyImageListWidget::setSelectCenter()
{
    m_listview->setSelectCenter();
}

int MyImageListWidget::getImgCount()
{
    //    qDebug() << "---" << __FUNCTION__ << "---m_listview->m_model->rowCount() = " << m_listview->m_model->rowCount();
    return m_listview->m_model->rowCount();
}

void MyImageListWidget::clearListView()
{
    m_listview->m_model->clear();
}

void MyImageListWidget::initAnimation()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    m_timer->setSingleShot(true);
    if (m_listview)
    {
        m_resetAnimation = new QPropertyAnimation(m_listview, "pos", nullptr); // 和上层m_obj的销毁绑在一起
    }
    connect(m_resetAnimation, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(m_resetAnimation, SIGNAL(valueChanged(const QVariant)), this, SLOT(animationValueChanged(const QVariant)));
}

QString MyImageListWidget::getCurrentPath()
{
    return m_listview->m_currentPath;
}

int MyImageListWidget::getCurrentCount()
{
    return m_listview->m_currentRow;
}

void MyImageListWidget::removeCurrent()
{
    m_listview->removeCurrent();
    this->setVisible(getImgCount() > 1);
}

void MyImageListWidget::flushCurrentImg(QPixmap pix, const QSize &originalSize)
{
    m_listview->slotCurrentImgFlush(pix, originalSize);
}

void MyImageListWidget::setCurrentPath(const QString &path)
{
    m_listview->setCurrentPath(path);
}

QStringList MyImageListWidget::getAllPath()
{
    return m_listview->getAllPath();
}

void MyImageListWidget::animationFinished()
{
    // 设置type标志用来判断是惯性动画还是复位动画
    if (m_resetAnimation->property("type") == "500")
    {
        m_resetFinish = false;
        animationStart(true, 0, 400);
    }
    if (m_resetAnimation->property("type") == "400")
    {
        m_resetFinish = true;
    }
}

void MyImageListWidget::animationValueChanged(const QVariant value)
{
    if (m_resetAnimation->property("type") != "500")
    {
        return;
    }
    // 惯性滑动
    thumbnailIsMoving();
    Q_UNUSED(value)
}

void MyImageListWidget::animationStart(bool isReset, int endPos, int duration)
{
    if (m_resetAnimation->state() == QPropertyAnimation::State::Running)
    {
        m_resetAnimation->stop();
    }

    // 目的为了获取moveX的值,中心离当前位置的差值
    int moveX = 0;
    int middle = (this->geometry().right() - this->geometry().left()) / 2;
    int itemX = m_listview->x() + m_listview->getCurrentItemX() + 31;
    int rowWidth = m_listview->getRowWidth();
    if (rowWidth - m_listview->getCurrentItemX() < (this->geometry().width() / 2))
    {
        moveX = this->geometry().width() - rowWidth - m_listview->x();
    }
    else if (m_listview->getCurrentItemX() < (this->geometry().width() / 2))
    {
        moveX = 0 - m_listview->pos().x();
    }
    else if (m_listview->geometry().width() <= this->width())
    {
        moveX = 0;
    }
    else
    {
        moveX = middle - itemX;
    }

    if (!isReset)
    {
        moveX = endPos;
    }
    m_resetAnimation->setDuration(duration);
    if (duration == 500)
    {
        m_resetAnimation->setProperty("type", "500");
    }
    else
    {
        m_resetAnimation->setProperty("type", "400");
    }
    m_resetAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_resetAnimation->setStartValue(m_listview->pos());
    // 限制缩略图List的移动范围
    int moveRangeX = m_listview->pos().x() + moveX;
    int offsetRowMove = rowWidth - ((rowWidth / RIGHT_LISTVIEW_MOVE_UPDATE) >= 12 ? 9 : (rowWidth / RIGHT_LISTVIEW_MOVE_UPDATE - 3)) * RIGHT_LISTVIEW_MOVE_UPDATE;
    moveRangeX = moveRangeX > LEFT_LISTVIEW_MOVE_MAXLEN ? LEFT_LISTVIEW_MOVE_MAXLEN : moveRangeX;
    moveRangeX = moveRangeX < (LEFT_LISTVIEW_MOVE_MAXLEN - offsetRowMove) ? (LEFT_LISTVIEW_MOVE_MAXLEN - offsetRowMove) : moveRangeX;
    m_resetAnimation->setEndValue(QPoint(moveRangeX, m_listview->pos().y()));
    m_resetAnimation->start();
}

void MyImageListWidget::stopAnimation()
{
    if (m_resetAnimation)
    {
        m_resetAnimation->stop();
    }
}

void MyImageListWidget::thumbnailIsMoving()
{
    if (m_resetAnimation->state() == QPropertyAnimation::State::Running && m_resetAnimation->duration() == 400)
    {
        return;
    }
    //    int endPos = m_movePoints.last().x() - m_movePoints.first().x();
    int offsetLimit = m_listview->geometry().left() - m_preListGeometryLeft;
    if (abs(offsetLimit) <= 32)
    {
        return;
    }
    qDebug() << offsetLimit;
    // 目的为了获取moveX的值,中心离当前位置的差值
    int moveX = 0;
    int middle = (this->geometry().right() - this->geometry().left()) / 2;
    int itemX = m_listview->x() + m_listview->getCurrentItemX() + 31;
    int rowWidth = m_listview->getRowWidth();
    if (rowWidth - m_listview->getCurrentItemX() < (this->geometry().width() / 2))
    {
        moveX = this->geometry().width() - rowWidth - m_listview->x();
    }
    else if (m_listview->getCurrentItemX() < (this->geometry().width() / 2))
    {
        moveX = 0 - m_listview->pos().x();
    }
    else if (m_listview->geometry().width() <= this->width())
    {
        moveX = 0;
    }
    else
    {
        moveX = middle - itemX;
    }
    // 如果图片没居中,会切换两次
    if (offsetLimit > 0)
    {
        m_listview->openPre();
        if (moveX < -32)
        {
            m_listview->openPre();
        }
    }
    else
    {
        m_listview->openNext();
        if (moveX > 32)
        {
            m_listview->openNext();
        }
    }
    m_preListGeometryLeft = m_listview->geometry().left();
}

void MyImageListWidget::moveCenterWidget()
{
    // 移动时停止动画
    stopAnimation();
    int moveX = 0;
    int middle = (this->geometry().right() - this->geometry().left()) / 2;
    int itemX = m_listview->x() + m_listview->getCurrentItemX() + 31;
    int rowWidth = m_listview->getRowWidth();
    if (rowWidth - m_listview->getCurrentItemX() < (this->geometry().width() / 2))
    {
        moveX = this->geometry().width() - rowWidth - m_listview->x();
    }
    else if (m_listview->getCurrentItemX() < (this->geometry().width() / 2))
    {
        moveX = 0 - m_listview->pos().x();
    }
    else if (m_listview->geometry().width() <= this->width())
    {
        moveX = 0;
    }
    else
    {
        moveX = middle - itemX;
    }
    m_listview->move(m_listview->x() + moveX, m_listview->y());
}

void MyImageListWidget::onScrollBarValueChanged(int value)
{
    QModelIndex index = m_listview->indexAt(QPoint((m_listview->width() - 15), 10));
    if (!index.isValid())
    {
        index = m_listview->indexAt(QPoint((m_listview->width() - 20), 10));
    }
    Q_UNUSED(value);
}

void MyImageListWidget::openNext()
{
    m_listview->openNext();
    moveCenterWidget();
}

void MyImageListWidget::openPre()
{
    m_listview->openPre();
    moveCenterWidget();
}

void MyImageListWidget::onClicked(const QModelIndex &index)
{
    qDebug() << "---------";
    if (m_timer->isActive())
    {
        m_listview->onClicked(index);
    }
    animationStart(true, 0, 400);
}

void MyImageListWidget::ONselectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "---ONselectionChanged------";
    if (!selected.indexes().isEmpty())
    {
        m_listview->onClicked(selected.indexes().first());
        animationStart(true, 0, 400);
    }
    Q_UNUSED(deselected);
}
