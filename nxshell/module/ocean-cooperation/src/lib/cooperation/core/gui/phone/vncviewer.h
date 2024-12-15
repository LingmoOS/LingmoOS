// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNCVIEWER_H
#define VNCVIEWER_H

#include "rfb/rfbclient.h"

#include "vncrecvthread.h"
#include "vncsendworker.h"

#include <iostream>
#include <thread>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>
#include <QTimer>

namespace cooperation_core {

class VncViewer : public QWidget
{
    Q_OBJECT

public:
    VncViewer(QWidget *parent = nullptr);
    virtual ~VncViewer();

    void setServes(const std::string &ip, int port, const std::string &pwd);
    void start();
    void stop();

    void setMobileRealSize(const int w, const int h);
    void updateImage(const QImage& image);

    std::thread *vncThread() const;
    void paintEvent(QPaintEvent *event) override;

    void setBackgroundBrush(QBrush brush) { m_backgroundBrush = brush; }
    QBrush backgroundBrush() { return m_backgroundBrush; }
    bool scaled() { return m_scaled; }
    void setScaled(bool scaled) { m_scaled = scaled; }

Q_SIGNALS:
    void sizeChanged(const QSize &size);
    void sendMouseState(rfbClient* cl, int x, int y, int button);
    void sendKeyState(rfbClient *cl, int key, bool down);
    void fullWindowCloseSignal();

public slots:
    void frameTimerTimeout();
    void onShortcutAction(int action);

    void updateSurface();

private:
    void setSurfaceSize(QSize surfaceSize);
    void clearSurface();

protected:
    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *e);

    // capture the mouse move events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent* event) override;

    int currentFps() { return m_currentFps; }
    void setCurrentFps(int fps) { m_currentFps = fps; }

    int frameCounter() { return m_frameCounter; }
    void incFrameCounter() { m_frameCounter++; }
    void setFrameCounter(int counter) { m_frameCounter = counter; }

    int translateMouseButton(Qt::MouseButton button);

private:
    std::string m_serverIp;
    std::string m_serverPwd;
    int m_serverPort;

    bool m_connected;
    QImage m_image;
    rfbClient *m_rfbCli { nullptr };
    QPainter m_painter;

    QThread* _vncSendThread;
    VNCSendWorker* _vncSendWorker;
    VNCRecvThread* _vncRecvThread;

    QBrush m_backgroundBrush;
    QPixmap m_surfacePixmap;
    QRect m_surfaceRect;
    QTransform m_transform;
    int m_buttonMask;
    bool m_scaled;
    qreal m_scale;

    QSize m_realSize; // record the phone real size
    qreal m_phoneScale; // the image scale on phone side
    int m_phoneMode;  // phone showing mode

    QTimer *m_frameTimer;
    uint m_frameCounter;
    uint m_currentFps;
};

}   // namespace cooperation_core

#endif   // VNCVIEWER_H
