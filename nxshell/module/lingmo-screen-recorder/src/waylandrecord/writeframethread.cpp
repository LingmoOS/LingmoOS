// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "writeframethread.h"
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include <qdebug.h>
#include <qimage.h>
#include "recordadmin.h"

WriteFrameThread::WriteFrameThread(WaylandIntegration::WaylandIntegrationPrivate* context, QObject *parent) :
    QThread(parent),
    m_bWriteFrame(false)
{
    m_context = context;
}

//int test = 0;
void WriteFrameThread::run()
{
    if(nullptr == m_context)
        return;

    m_context->m_recordAdmin->m_cacheMutex.lock();
    WaylandIntegration::WaylandIntegrationPrivate::waylandFrame frame;
    while (m_context->isWriteVideo()) {
        if (m_context->getFrame(frame)) {
            m_context->m_recordAdmin->m_pOutputStream->writeVideoFrame(frame);
        }
    }
    m_context->m_recordAdmin->m_cacheMutex.unlock();
}

bool WriteFrameThread::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    return m_bWriteFrame;
}

void WriteFrameThread::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
}
