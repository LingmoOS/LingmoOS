// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WRITEFRAMETHREAD_H
#define WRITEFRAMETHREAD_H
#include <QMutex>
#include <QObject>
#include <QThread>
#include "waylandintegration_p.h"

class WriteFrameThread : public QThread
{
    Q_OBJECT

public:
    explicit WriteFrameThread(WaylandIntegration::WaylandIntegrationPrivate* context,QObject *parent = nullptr);
    void run();

    bool bWriteFrame();
    void setBWriteFrame(bool bWriteFrame);

private:
    WaylandIntegration::WaylandIntegrationPrivate * m_context;
    //是否写视频帧
    bool m_bWriteFrame;
    QMutex m_writeFrameMutex;
};

#endif // WRITEFRAMETHREAD_H
