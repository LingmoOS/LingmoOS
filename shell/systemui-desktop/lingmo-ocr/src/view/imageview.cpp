#include "imageview.h"

#include <QPaintDevice>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QMessageBox>
#include <QThreadPool>
#include <qmath.h>
#include <QObject>
#include <QGestureEvent>
#include <QPinchGesture>

const qreal MAX_SCALE_FACTOR = 20.0;
const qreal MIN_SCALE_FACTOR = 0.029;
#define devicePixelRatioF  devicePixelRatio
ImageView::ImageView(QWidget *parent):
    QGraphicsView(parent)
{
    setMouseTracking(true);
    setDragMode(ScrollHandDrag);
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->grabGesture(Qt::PinchGesture);
    setAttribute(Qt::WA_AcceptTouchEvents);
    viewport()->setCursor(Qt::ArrowCursor);
}

ImageView::~ImageView()
{
    if (m_currentImage) {
        delete m_currentImage;
    }
}

void ImageView::openImage(const QString &path)
{
    if (scene()) {

        if (m_currentImage) {
            delete m_currentImage;
            m_currentImage = nullptr;
        }
        m_currentImage = new QImage(path);

        if (!m_currentImage->isNull()) {
            QPixmap pic = QPixmap::fromImage(*m_currentImage);

            scene()->clear();
            m_pixmapItem = new QGraphicsPixmapItem(pic);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
            QRectF rect = m_pixmapItem->boundingRect();
            setSceneRect(rect);
            scene()->addItem(m_pixmapItem);
            fitWindow();
            m_currentPath = path;
        } else {
            //       App->setStackWidget(0);
        }
        m_FilterImage = image();
    }
}

void ImageView::openFilterImage(QImage img)
{
    if (!img.isNull() && scene()) {
        m_FilterImage = img;
    }
    QPixmap pic = QPixmap::fromImage(img);
    if (!pic.isNull()) {
        scene()->clear();
        m_pixmapItem = new QGraphicsPixmapItem(pic);
        m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
        QRectF rect = m_pixmapItem->boundingRect();
        setSceneRect(rect);
        scene()->addItem(m_pixmapItem);
        fitWindow();
    }


}
qreal ImageView::windowRelativeScale() const
{
    //替换撑满方案
    QRectF bf = sceneRect();
    if (1.0 * width() / height() > 1.0 * bf.width() / bf.height()) {
        return 1.0 * height() / bf.height();
    } else {
        return 1.0 * width() / bf.width();
    }
}
void ImageView::fitWindow()
{
    qreal wrs = windowRelativeScale();
    m_scal = wrs;
    resetTransform();
    scale(wrs, wrs);

    m_isFitImage = false;
    m_isFitWindow = true;

}

void ImageView::fitImage()
{
    resetTransform();
    m_scal = 1.0;
    scale(1, 1);
    m_isFitImage = true;
    m_isFitWindow = false;
}

void ImageView::RotateImage(const int &index)
{
    if (!m_pixmapItem && scene()) return;
    QPixmap pixmap = m_pixmapItem->pixmap();
    QMatrix rotate;
    rotate.rotate(index);

    pixmap = pixmap.transformed(rotate, Qt::FastTransformation);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    scene()->clear();
    resetTransform();
    m_pixmapItem = new QGraphicsPixmapItem(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    // Make sure item show in center of view after reload
    QRectF rect = m_pixmapItem->boundingRect();
    setSceneRect(rect);
    scene()->addItem(m_pixmapItem);

    autoFit();
    m_rotateAngel += index;

    m_FilterImage = image();


}



//void ImageView::openImage(QImage *img)
//{
//    if (!img->isNull() && scene()) {
//        QPixmap pic = QPixmap::fromImage(*img);
//        if (!pic.isNull()) {
//            scene()->clear();
//            m_pixmapItem = new QGraphicsPixmapItem(pic);
//            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
//            QRectF rect = m_pixmapItem->boundingRect();
//            setSceneRect(rect);
//            scene()->addItem(m_pixmapItem);
//            fitWindow();
//        }
//    }
//}

qreal ImageView::imageRelativeScale() const
{
    return transform().m11() / devicePixelRatioF();
}
void ImageView::autoFit()
{
    if (image().isNull())
        return;

    QSize image_size = image().size();

    // change some code in graphicsitem.cpp line100.

    if ((image_size.width() >= width() || image_size.height() >= height() - 150) && width() > 0 &&
            height() > 0) {
        fitWindow();
    } else {
        fitImage();
    }

}

void ImageView::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);
    viewport()->setCursor(Qt::ArrowCursor);
}

void ImageView::mousePressEvent(QMouseEvent *e)
{
    QGraphicsView::mousePressEvent(e);
    viewport()->unsetCursor();
    viewport()->setCursor(Qt::ArrowCursor);
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    //修复鼠标状态不对的问题
    if (!(event->buttons() | Qt::NoButton)) {
        viewport()->setCursor(Qt::ArrowCursor);
    } else {
        QGraphicsView::mouseMoveEvent(event);
        viewport()->setCursor(Qt::ClosedHandCursor);
    }
}

bool ImageView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent *>(event));
    }
    return QGraphicsView::event(event);
}

bool ImageView::gestureEvent(QGestureEvent *event)
{
//    if (QGesture *swipe = event->gesture(Qt::SwipeGesture)) //滑动逻辑
//        swipeTriggered(static_cast<QSwipeGesture *>(swipe));
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))//双指捏合
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    return true;
}


const QImage ImageView::image()
{
    if (m_pixmapItem) {
        return m_pixmapItem->pixmap().toImage();
    } else {
        return QImage();
    }
}

void ImageView::resizeEvent(QResizeEvent *event)
{

    return QGraphicsView::resizeEvent(event);
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    scaleAtPoint(event->pos(), factor);

    event->accept();
}

void ImageView::handleGestureEvent(QGestureEvent *gesture)
{
    qDebug() << "------" << __FUNCTION__ << "";
    if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void ImageView::pinchTriggered(QPinchGesture *gesture)
{
    qDebug() << "------" << __FUNCTION__ << "";
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    //缩放手势
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        QPoint pos = mapFromGlobal(gesture->centerPoint().toPoint());
        if (abs(gesture->scaleFactor() - 1) > 0.006) {
            qDebug() << "scaleFactor" << gesture->scaleFactor();
            scaleAtPoint(pos, gesture->scaleFactor());
        }
    }
}
void ImageView::scaleAtPoint(QPoint pos, qreal factor)
{
    // Remember zoom anchor point.
    const QPointF targetPos = pos;
    const QPointF targetScenePos = mapToScene(targetPos.toPoint());

    // Do the scaling.
    setScaleValue(factor);
    // Restore the zoom anchor point.
    //
    // The Basic idea here is we don't care how the scene is scaled or transformed,
    // we just want to restore the anchor point to the target position we've
    // remembered, in the coordinate of the view/viewport.
    const QPointF curPos = mapFromScene(targetScenePos);
    const QPointF centerPos = QPointF(width() / 2.0, height() / 2.0) + (curPos - targetPos);
    const QPointF centerScenePos = mapToScene(centerPos.toPoint());
    centerOn(static_cast<int>(centerScenePos.x()), static_cast<int>(centerScenePos.y()));
}
void ImageView::setScaleValue(qreal v)
{
    //由于矩阵被旋转，通过矩阵获取缩放因子，计算缩放比例错误，因此记录过程中的缩放因子来判断缩放比例
    m_scal *= v;
    qDebug() << m_scal;
    scale(v, v);
    //const qreal irs = imageRelativeScale() * devicePixelRatioF();
    // Rollback
    if (v < 1 && /*irs <= MIN_SCALE_FACTOR)*/m_scal < 0.03) {
        const qreal minv = MIN_SCALE_FACTOR / m_scal;
        // if (minv < 1.09) return;
        scale(minv, minv);
        m_scal *= minv;
    } else if (v > 1 && /*irs >= MAX_SCALE_FACTOR*/m_scal > 20) {
        const qreal maxv = MAX_SCALE_FACTOR / m_scal;
        scale(maxv, maxv);
        m_scal *= maxv;
    } else {
        m_isFitImage = false;
        m_isFitWindow = false;
    }

//    qreal rescale = imageRelativeScale() * devicePixelRatioF();
    //    if (rescale - 1 > -0.01 && rescale - 1 < 0.01) {
    //        emit checkAdaptImageBtn();
    //    } else {
    //        emit disCheckAdaptImageBtn();
    //    }
    emit scaled(m_scal * 100);
    emit showScaleLabel();

}

