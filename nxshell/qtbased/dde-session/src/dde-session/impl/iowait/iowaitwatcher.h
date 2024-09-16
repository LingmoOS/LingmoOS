// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IOWAITWATCHER_H
#define IOWAITWATCHER_H

#include <QObject>

class QTimer;
class IOWaitWatcher : public QObject
{
    Q_OBJECT
public:
    explicit IOWaitWatcher(QObject *parent = nullptr);

    void start();

private Q_SLOTS:
    void onTimeOut();

private:
    void init();
    void xcLeftPtrToWatch(bool enabled);

private:
    float m_maxStep;
    QTimer *m_timer;
};

#endif // IOWAITWATCHER_H
