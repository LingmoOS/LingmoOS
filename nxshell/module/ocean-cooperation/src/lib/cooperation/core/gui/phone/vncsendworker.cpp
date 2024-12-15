// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vncsendworker.h"

#include <QTimer>
#include <QDebug>

VNCSendWorker::VNCSendWorker(QObject *parent): QObject(parent)
{
}

void VNCSendWorker::sendMouseUpdateMsg(rfbClient *cl, int x, int y, int button = 0)
{
    SendPointerEvent(cl, x, y, button);
    SendIncrementalFramebufferUpdateRequest(cl);
}

void VNCSendWorker::sendKeyUpdateMsg(rfbClient *cl, int key, bool down)
{
    SendKeyEvent(cl, key, down);
    SendIncrementalFramebufferUpdateRequest(cl);
}
