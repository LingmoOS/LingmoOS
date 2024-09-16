// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camerawidget.h"
#include "v4l2_core.h"

#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>
#include <QDir>
#include <QBitmap>
#include <QtConcurrent>

#include "../camera/LPF_V4L2.h"
#include "../camera/majorimageprocessingthread.h"

CameraWidget::CameraWidget(DWidget *parent) : DWidget(parent)
{
}

CameraWidget::~CameraWidget()
{

    if (m_imgPrcThread) {
        m_imgPrcThread->stop();
        m_imgPrcThread->deleteLater();
        m_imgPrcThread = nullptr;
    }
    qDebug() << "~CameraWidget()";
}


void CameraWidget::setRecordRect(int x, int y, int width, int height)
{
    this->recordX = x;
    this->recordY = y;
    this->recordWidth = width;
    this->recordHeight = height;
}

void CameraWidget::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();
    move(pos.x(), pos.y());
}

int CameraWidget::getRecordX()
{
    return recordX;
}
int CameraWidget::getRecordY()
{
    return recordY;
}
int CameraWidget::getRecordWidth()
{
    return recordWidth;
}
int CameraWidget::getRecordHeight()
{
    return recordHeight;
}

void CameraWidget::initUI()
{
    qDebug() << "正在初始化摄像头界面...";
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
    m_cameraUI = new DLabel(this);

    QHBoxLayout *t_hlayout = new QHBoxLayout(this);
    t_hlayout->addWidget(m_cameraUI);
    this->setLayout(t_hlayout);

//    qDebug() << "第一次打开摄像头界面通过QCamera获取摄像头名称:" << QCameraInfo::defaultCamera().isNull();
//    //第一次打开摄像头界面通过QCamera获取摄像头名称
//    while (QCameraInfo::defaultCamera().isNull()) {
//        qDebug() << "正在通过QCamera获取摄像头名称:" << QCameraInfo::defaultCamera().isNull();
//        QEventLoop loop;
//        QTimer::singleShot(200, &loop, SLOT(quit()));
//        loop.exec();
//    }
//    m_deviceName = QCameraInfo::defaultCamera().deviceName();
    qDebug() << "摄像头界面初始化已完成";

}

void CameraWidget::cameraStart()
{
    if (!m_isInitCamera) {
        qInfo() << "第一次初始化摄像头...";
        if (m_imgPrcThread == nullptr) {
            m_imgPrcThread = new MajorImageProcessingThread;
            m_imgPrcThread->setParent(this);
            m_imgPrcThread->setObjectName("MajorThread");
            connect(m_imgPrcThread, SIGNAL(SendMajorImageProcessing(QImage)),
                    this, SLOT(onReceiveMajorImage(QImage)));
        }
        m_isInitCamera = true;
        startCameraV4l2(m_deviceName.toLatin1());
        qInfo() << "第一次初始化摄像头已完成！";
    } else {
        qInfo() << "重新初始化摄像头...";
        restartDevices();
        qInfo() << "重新初始化摄像头已完成！";
    }
}
int CameraWidget::startCameraV4l2(const char *device)
{
    qInfo() << "正在初始化摄像头(" << device << ")...";
    int ret = camInit(device);
    v4l2_dev_t *vd =  get_v4l2_device_handler();

    if (vd == nullptr) {
        qWarning() << "camInit failed! not found camera device!";
        return -1;
    }
    qDebug() << "videoDevice->videodevice: " << vd->videodevice;
    if (ret == E_OK) {
        //m_imgPrcThread->setCameraDevice(vd);
        m_imgPrcThread->start();
    } else {
        qWarning() << "camInit failed";
        camUnInit();
    }
    qInfo() << "摄像头已初始化(" << device << ")";
    return ret;
}

void CameraWidget::restartDevices()
{
    //获取当前的摄像头设备
    v4l2_dev_t *devicehandler =  get_v4l2_device_handler();

    //停止采集摄像头画面的线程
    if (m_imgPrcThread != nullptr && m_imgPrcThread->isRunning())
        m_imgPrcThread->stop();

    while (m_imgPrcThread->isRunning());
    QString str;

    if (devicehandler != nullptr) {
        str = QString(devicehandler->videodevice);
        close_v4l2_device_handler();
    }

    v4l2_device_list_t *devlist = get_device_list();
    if (devlist->num_devices == 2) {
        for (int i = 0 ; i < devlist->num_devices; i++) {
            QString str1 = QString(devlist->list_devices[i].device);
            if (str != str1) {
                if (E_OK == startCameraV4l2(devlist->list_devices[i].device)) {
                    break;
                }
            }
        }
    } else {
        for (int i = 0 ; i < devlist->num_devices; i++) {
            QString str1 = QString(devlist->list_devices[i].device);
            if (str == str1) {
                if (i == devlist->num_devices - 1) {
                    startCameraV4l2(devlist->list_devices[0].device);
                } else {
                    startCameraV4l2(devlist->list_devices[i + 1].device);
                }
                break;
            }
            if (str.isEmpty()) {
                startCameraV4l2(devlist->list_devices[0].device);
                break;
            }
        }
    }
}

void CameraWidget::cameraStop()
{
    m_cameraUI->clear();

    if (m_imgPrcThread != nullptr) {
        m_imgPrcThread->stop();
    }
    camUnInit();
}

void CameraWidget::onReceiveMajorImage(QImage image)
{
    QImage t_image = image.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    QPixmap pixmap = QPixmap::fromImage(t_image);
//    pixmap.save(QString("/home/wangcong/Desktop/test/test_%1.png").arg(QDateTime::currentMSecsSinceEpoch()));
    if (m_cameraUI)
        m_cameraUI->setPixmap(pixmap);
    //qInfo() << "接收当前摄像头画面！";
}

void CameraWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qDebug() << "CameraWidget enterEvent";
}

void CameraWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
//    QPainter painter(this);
//    painter.setBrush(QColor(0, 0, 0, 255));
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.drawRect(rect());

    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
//    p.fillRect(this->rect(), Qt::transparent);
//    p.setBrush(Qt::transparent);
//    p.drawRoundedRect(bmp.rect(), 30, 45, Qt::RelativeSize);
    p.drawRoundedRect(12, 12, width() - 25, height() - 25, 8, 8);
    setMask(bmp);



//    DWidget::paintEvent(e);
}
void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton) {
        qDebug() << "CameraWidget mousePressEvent";
        m_move = true;
        //记录鼠标的世界坐标.
        m_startPoint = event->globalPos();
        //记录窗体的世界坐标.
        m_windowTopLeftPoint = this->frameGeometry().topLeft();
        m_windowTopRightPoint = this->frameGeometry().topRight();
        m_windowBottomLeftPoint = this->frameGeometry().bottomLeft();
    }
}

void CameraWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && m_Immovable == false) {
//        //移动中的鼠标位置相对于初始位置的相对位置.
//        QPoint relativePos = event->globalPos() - m_startPoint;
//        QRect recordRect = QRect(recordX, recordY, recordWidth, recordHeight);
//        QPoint removePos = m_windowTopLeftPoint + relativePos;
//        QPoint removeTopRightPos = m_windowTopRightPoint + relativePos;
//        QPoint removeBottomLeftPos = m_windowBottomLeftPoint + relativePos;
//        //然后移动窗体即可.
//        if (recordRect.contains(removePos) &&
//                recordRect.contains(removeTopRightPos) &&
//                recordRect.contains(removeBottomLeftPos) ) {
//            this->move(removePos);
//        }

//        else {

//        }
        QSize size = this->size();
        QSize parentSize(recordWidth, recordHeight);
        QPoint curPos = QPoint(event->globalX() - size.width() / 2,
                               event->globalY() - size.height() / 2);
        if (curPos.x() < recordX) //left
            curPos.setX(recordX);

        if (curPos.y() < recordY) //top
            curPos.setY(recordY);

        if ((curPos.x() + size.width()) > (recordX + parentSize.width()))  //right
            curPos.setX(recordX + parentSize.width() - size.width());

        if ((curPos.y() + size.height()) > (recordY + parentSize.height()))  //bottom
            curPos.setY(recordY + parentSize.height() - size.height());

//        QPoint resultPos = QPoint(curPos.x() - size.width() / 2,
//                                  curPos.y() - size.height() / 2);
        this->move(curPos);
    }
    DWidget::mouseMoveEvent(event);
}

void CameraWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        //改变移动状态.
        m_move = false;
        int centerX = this->x() + this->width() / 2;
        int centerY = this->y() + this->height() / 2;
        if (centerX - recordX < recordWidth / 2) {
            if (centerY - recordY < recordHeight / 2) {
                this->move(recordX, recordY);
            } else {
                this->move(recordX, recordY + recordHeight - this->height());
            }
        } else {
            if (centerY - recordY < recordHeight / 2) {
                this->move(recordX + recordWidth - this->width(), recordY);
            } else {
                this->move(recordX + recordWidth - this->width(), recordY + recordHeight - this->height());
            }
        }
    }
}

CameraWidget::Position CameraWidget::postion()
{
    if (this->x() == recordX) {
        if (this->y() == recordY) {
            return leftTop;
        } else {
            return leftBottom;
        }
    } else {
        if (this->y() == recordY) {
            return rightTop;
        } else {
            return rightBottom;
        }
    }
}

// 设置CameraWidget是否可以拖动
void CameraWidget::setCameraWidgetImmovable(bool immovable)
{
    m_Immovable = immovable;
}

int CameraWidget::getCameraStatus()
{
    if (m_imgPrcThread)
        return m_imgPrcThread->getStatus();
    return 0;
}

void CameraWidget::setDevcieName(const QString &devcieName)
{
    m_deviceName = devcieName;
}
