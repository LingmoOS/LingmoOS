// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNCSENDWORKER_H
#define VNCSENDWORKER_H

#include <QObject>

#include "rfb/rfbclient.h"

class VNCSendWorker : public QObject
{
    Q_OBJECT
public:
    explicit VNCSendWorker(QObject *parent = nullptr);

signals:

public slots:
    void sendMouseUpdateMsg(rfbClient *cl, int x, int y, int button);
    void sendKeyUpdateMsg(rfbClient *cl, int key, bool down);

};

#endif // VNCSENDWORKER_H
