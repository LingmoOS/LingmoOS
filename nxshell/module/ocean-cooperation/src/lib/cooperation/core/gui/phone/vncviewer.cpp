// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vncviewer.h"
#include "qt2keysum.h"

#include <QDebug>
#include <QElapsedTimer>

using namespace cooperation_core;

enum Operation {
    BACK = 0,
    HOME,
    RECENTS
};

enum PhoneMode {
    PORTRAIT= 0,
    LANDSCAPE,
};

VncViewer::VncViewer(QWidget *parent)
    : QWidget(parent),
      m_surfacePixmap(-1, -1),
      m_scale(1.0),
      m_scaled(true),
      m_buttonMask(0)
{
    setFocusPolicy(Qt::StrongFocus);
#ifdef TOUCH_MODE
    setMouseTracking(false);
#else
    setMouseTracking(true);
#endif

    _vncSendThread = new QThread(this);
    _vncSendWorker = new VNCSendWorker();
    connect(this, &VncViewer::sendMouseState, _vncSendWorker, &VNCSendWorker::sendMouseUpdateMsg);
    connect(this, &VncViewer::sendKeyState, _vncSendWorker, &VNCSendWorker::sendKeyUpdateMsg);
    _vncSendWorker->moveToThread(_vncSendThread);

    _vncRecvThread = new VNCRecvThread(this);
    connect(_vncRecvThread, &VNCRecvThread::updateImageSignal, this, &VncViewer::updateImage, Qt::BlockingQueuedConnection);
    connect(_vncRecvThread, &VNCRecvThread::finished, this, &VncViewer::stop);

    m_frameTimer = new QTimer(this);
    m_frameTimer->setTimerType(Qt::CoarseTimer);
    m_frameTimer->setInterval(1000);

    connect(m_frameTimer, SIGNAL(timeout()), this, SLOT(frameTimerTimeout()));
}

VncViewer::~VncViewer()
{
}

void VncViewer::setServes(const std::string &ip, int port, const std::string &pwd)
{
    m_serverIp = ip;
    m_serverPort = port;
    m_serverPwd = pwd;
}

void VncViewer::frameTimerTimeout()
{
    setCurrentFps(frameCounter());
    setFrameCounter(0);

#ifdef QT_DEBUG
    qWarning() << " FPS: " << currentFps();
#endif

    if (!m_connected)
        return;

    // Check the screen has been rotated
    int curentMode = (m_rfbCli->width < m_rfbCli->height) ? PORTRAIT : LANDSCAPE;
    if (curentMode != m_phoneMode) {
        m_phoneMode = curentMode;
        int w = (m_phoneMode == PORTRAIT) ? m_realSize.width() : m_realSize.height();
        const QSize size = {static_cast<int>(w * m_phoneScale), m_rfbCli->height};

        setSurfaceSize(size);
        emit sizeChanged(size);
    }
}

void VncViewer::onShortcutAction(int action)
{
    if (!m_connected)
        return;

    int key = 0;
    switch (action) {
    case BACK:
        key = Qt::Key_Escape;
        break;
    case HOME:
        key = Qt::Key_Home;
        break;
    case RECENTS:
        key = Qt::Key_PageUp;
        break;
    }

    if (key > 0) {
        SendKeyEvent(m_rfbCli, qt2keysym(key), true);
    }
}

void VncViewer::updateSurface()
{
    resizeEvent(0);
    update();
}

void VncViewer::clearSurface()
{
    setCurrentFps(0);
    setFrameCounter(0);

    if (m_surfacePixmap.isNull() && m_connected)
        setSurfaceSize({m_rfbCli->width, m_rfbCli->height});
    else
        setSurfaceSize(m_surfacePixmap.size());
}

int VncViewer::translateMouseButton(Qt::MouseButton button)
{
    switch (button) {
    case Qt::LeftButton:   return rfbButton1Mask;
    case Qt::MiddleButton: return rfbButton2Mask;
    case Qt::RightButton:  return rfbButton3Mask;
    default:               return 0;
    }
}

void VncViewer::setMobileRealSize(const int w, const int h)
{
    // set the phone real resolution by only once
    m_realSize = w < h ? QSize(w, h) : QSize(h, w);
}

void VncViewer::updateImage(const QImage &image)
{
    m_image = image;

    update();
}

void VncViewer::paintEvent(QPaintEvent *event)
{
    if (m_connected) {
        m_painter.begin(&m_surfacePixmap);
        // if (m_image.hasAlphaChannel()) {
        //     // 设置合成模式为源模式
        //     m_painter.setCompositionMode(QPainter::CompositionMode_Source);
        //     m_painter.fillRect(this->rect(), Qt::transparent); // 用透明填充
        // } else {
        //     // 如果没有 alpha 通道，保留原来的内容
        //     m_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        // }
        m_painter.drawImage(rect().topLeft(), m_image);
        m_painter.end();


        m_painter.begin(this);
        m_painter.setRenderHints(QPainter::SmoothPixmapTransform);
        m_painter.fillRect(rect(), m_backgroundBrush);
        if (scaled()) {
            m_surfaceRect.moveCenter(rect().center());
            m_painter.scale(m_scale, m_scale);
            m_painter.drawPixmap(m_surfaceRect.x() / m_scale, m_surfaceRect.y() / m_scale, m_surfacePixmap);
        } else {
            m_painter.scale(1.0, 1.0);
            m_painter.drawPixmap((width() - m_surfacePixmap.width()) / 2, (height() - m_surfacePixmap.height()) / 2, m_surfacePixmap);
        }
        m_painter.end();
    } else {
        m_painter.begin(this);
        m_painter.fillRect(rect(), backgroundBrush());
        m_painter.setPen(Qt::black);
        m_painter.setFont(font());
        QRect m_textBoundingRect = m_painter.boundingRect(rect(), Qt::AlignHCenter | Qt::AlignVCenter, tr("Disconnected"));
        m_textBoundingRect.moveCenter(rect().center());
        m_painter.drawText(m_textBoundingRect, tr("Disconnected"));
        m_painter.end();
    }

    incFrameCounter();
}

void VncViewer::setSurfaceSize(QSize surfaceSize)
{
    m_surfacePixmap = QPixmap(surfaceSize);
    m_surfacePixmap.fill(backgroundBrush().color());
    m_surfaceRect = m_surfacePixmap.rect();
    m_surfaceRect.setWidth(m_surfaceRect.width() * m_scale);
    m_surfaceRect.setHeight(m_surfaceRect.height() * m_scale);
    m_transform = QTransform::fromScale(1.0 / m_scale, 1.0 / m_scale);

    QTimer::singleShot(0, this, SLOT(updateSurface()));
}

void VncViewer::resizeEvent(QResizeEvent *e)
{
    if (!m_surfacePixmap.isNull()) {
        if (scaled()) {
            // 计算保持比例的缩放因子
            qreal widthScale = (qreal)width() / (qreal)m_surfacePixmap.width();
            qreal heightScale = (qreal)height() / (qreal)m_surfacePixmap.height();

            // 选择较小的缩放因子以保持比例
            m_scale = qMin(widthScale, heightScale);
        }

        m_surfaceRect = m_surfacePixmap.rect();
        m_surfaceRect.setWidth(m_surfaceRect.width() * m_scale);
        m_surfaceRect.setHeight(m_surfaceRect.height() * m_scale);
        m_surfaceRect.moveCenter(rect().center());
        m_transform = QTransform::fromScale(1.0 / m_scale, 1.0 / m_scale);
        m_transform.translate(-m_surfaceRect.x(), -m_surfaceRect.y());
    }
    if (e)
        QWidget::resizeEvent(e);
}

bool VncViewer::event(QEvent *e)
{
    if (m_connected) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
            m_surfaceRect.moveCenter(rect().center()); // fall through!
        case QEvent::MouseMove: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            QPoint mappedPos = m_transform.map(mouseEvent->pos());
            int button = translateMouseButton(mouseEvent->button());

            switch (e->type()) {
            case QEvent::MouseButtonPress:
                m_buttonMask |= button;
                break;

            case QEvent::MouseButtonRelease:
                m_buttonMask &= ~button;
                break;

            case QEvent::MouseButtonDblClick:
                emit sendMouseState(m_rfbCli, mappedPos.x(), mappedPos.y(), m_buttonMask | button);
                emit sendMouseState(m_rfbCli, mappedPos.x(), mappedPos.y(), m_buttonMask);
                emit sendMouseState(m_rfbCli, mappedPos.x(), mappedPos.y(), m_buttonMask | button);
                break;

            default:
                break;
            }

            emit sendMouseState(m_rfbCli, mappedPos.x(), mappedPos.y(), m_buttonMask);
            break;
        }

        case QEvent::Wheel: { // 处理滚轮事件
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(e);
            QPoint mappedPos = m_transform.map(wheelEvent->pos());

            // 定义一个处理滚轮的辅助函数
            auto processWheelEvent = [&](int delta, int positiveButtonMask, int negativeButtonMask) {
                if (delta != 0) {
                    int steps = delta / 120; // 每120个单位视为一步
                    int buttonMask = (steps > 0) ? positiveButtonMask : negativeButtonMask;
                    for (int i = 0; i < abs(steps); ++i) {
                        emit sendMouseState(m_rfbCli, mappedPos.x(), mappedPos.y(), buttonMask);
                        emit sendMouseState(m_rfbCli, mappedPos.x(), mappedPos.y(), 0); // release event
                    }
                }
            };

            // 处理纵向和横向滚轮事件
            processWheelEvent(wheelEvent->angleDelta().y(), rfbButton4Mask, rfbButton5Mask);
            processWheelEvent(wheelEvent->angleDelta().x(), 0b01000000, 0b00100000);

            break;
        }

        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
            emit sendKeyState(m_rfbCli, qt2keysym(keyEvent->key()), e->type() == QEvent::KeyPress);
            if (keyEvent->key() == Qt::Key_Alt)
                setFocus(); // avoid losing focus
            return true; // prevent futher processing of event
        }

        default:
            break;
        }
    }

    return QWidget::event(e);
}

void VncViewer::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void VncViewer::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void VncViewer::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

void VncViewer ::closeEvent(QCloseEvent *event)
{
    emit fullWindowCloseSignal();
    QWidget::closeEvent(event);
}

void VncViewer::start()
{
    m_rfbCli = rfbGetClient(8, 3, 4);
    m_rfbCli->format.depth = 32;
    m_rfbCli->serverHost = strdup(m_serverIp.c_str());
    m_rfbCli->serverPort = m_serverPort;
    // m_rfbCli->appData.compressLevel = 9;
    // m_rfbCli->appData.qualityLevel = 9;
    // m_rfbCli->appData.scaleSetting = 1;
    m_rfbCli->appData.forceTrueColour = TRUE;
    m_rfbCli->appData.useRemoteCursor = FALSE;
    m_rfbCli->appData.encodingsString = "tight ultra";

    rfbClientSetClientData(m_rfbCli, nullptr, this);

    if (rfbInitClient(m_rfbCli, 0, nullptr)) {
        m_connected = true;
    } else {
        std::cout << "[INFO] disconnected" << std::endl;
        m_connected = false;
        return;
    }

    std::cout << "[INFO] vnc screen: " << m_rfbCli->width << " x " << m_rfbCli->height << std::endl;
#ifdef HIDED_CURSOR
    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::transparent);
    setCursor(QCursor(pixmap));
#endif
    // 启动帧率计时器
    m_frameTimer->start();

    int viewWidth;
    m_phoneMode = (m_rfbCli->width < m_rfbCli->height) ? PORTRAIT : LANDSCAPE;
    if (PORTRAIT == m_phoneMode) {
        m_phoneScale = static_cast<qreal>(m_rfbCli->height) / static_cast<qreal>(m_realSize.height());
        viewWidth = static_cast<int>(m_realSize.width() * m_phoneScale);
    } else {
        m_phoneScale = static_cast<qreal>(m_rfbCli->height) / static_cast<qreal>(m_realSize.width());
        viewWidth = static_cast<int>(m_realSize.height() * m_phoneScale);
    }
    const QSize size = {viewWidth, m_rfbCli->height};

    // qWarning() << "Phone mode: " << m_phoneMode << " Phone scale: " << m_phoneScale;
    // qWarning() << "Real size: " << m_realSize << " show size: " << size;

    setSurfaceSize(size);
    emit sizeChanged(size);

    _vncRecvThread->startRun(m_rfbCli);
    _vncSendThread->start();
}


void VncViewer::stop()
{
    if (!m_connected)
        return;

    m_frameTimer->stop();
    m_connected = false;

    _vncRecvThread->stopRun();
    _vncRecvThread->quit();
    _vncRecvThread->wait();

    _vncSendThread->quit();
    _vncSendThread->wait();

    if (m_rfbCli) {
        rfbClientCleanup(m_rfbCli);
        m_rfbCli = nullptr;
    }
}
