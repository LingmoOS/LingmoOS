// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vncrecvthread.h"


VNCRecvThread::VNCRecvThread(QObject *parent): QThread(parent)
{

}

void VNCRecvThread::startRun(rfbClient *cl)
{
    if (_runFlag)
        return;

    _cl = cl;
    _cl->FinishedFrameBufferUpdate = frameBufferUpdated;
    rfbClientSetClientData(_cl, nullptr, this);
    _runFlag = true;

    this->start();
}

void VNCRecvThread::stopRun()
{
    if (!_runFlag)
        return;

    _runFlag = false;
    if (_cl) {
        rfbClientSetClientData(_cl, nullptr, nullptr);
        _cl->FinishedFrameBufferUpdate = nullptr;
    }
}


void VNCRecvThread::run()
{
    while (_runFlag && _cl) {
        int i = WaitForMessage(_cl, 500);
        if (i < 0) {
            break;
        }

        if (i && !HandleRFBServerMessage(_cl)) {
            break;
        }
    };
}

void VNCRecvThread::frameBufferUpdated(rfbClient *cl)
{
    VNCRecvThread *vncRecvThread = static_cast<VNCRecvThread *>(rfbClientGetClientData(cl, nullptr));

    emit vncRecvThread->updateImageSignal(QImage(cl->frameBuffer, cl->width, cl->height, QImage::Format_RGBA8888));
}
