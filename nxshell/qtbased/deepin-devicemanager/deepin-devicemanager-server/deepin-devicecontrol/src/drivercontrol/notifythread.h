// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFYTHREAD_H
#define NOTIFYTHREAD_H

#include <QObject>
#include <QThread>


class NotifyThread : public QThread
{
    Q_OBJECT
public:
    explicit NotifyThread(QObject *parent = nullptr);
    void run() override;

signals:

public slots:
};

#endif // NOTIFYTHREAD_H
