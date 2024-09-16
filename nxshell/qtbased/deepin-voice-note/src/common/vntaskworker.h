// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNTASKWORKER_H
#define VNTASKWORKER_H

#include <QThread>
#include <QVector>
#include <QMutex>
#include <QWaitCondition>

class VNTask;

class VNTaskWorker : public QThread
{
    Q_OBJECT
public:
    explicit VNTaskWorker(QObject *parent = nullptr);
    //添加任务
    void addTask(VNTask *task);
    //设置任务名称
    void setWorkerName(const QString &worker);
    //退出，结束线程
    void quitWorker();
signals:

public slots:
protected:
    virtual void run() override;

protected:
    QVector<VNTask *> m_safetyTaskQueue;
    QMutex m_taskLock;
    QWaitCondition m_taskCondition;
    QString m_workerName {"VNTaskWoker"};
    volatile bool m_fQuit {false};
};

#endif // VNTASKWORKER_H
