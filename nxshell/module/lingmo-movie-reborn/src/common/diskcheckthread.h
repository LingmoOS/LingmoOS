// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DISKCHECKTHREAD_H
#define DISKCHECKTHREAD_H

#include <QMap>
#include <QTimer>
#include <QObject>

/**
 * @file 光盘监测线程，监测光盘状态
 */
class Diskcheckthread: public QObject
{
    Q_OBJECT

signals:
    /**
     * @file 光盘移除信号
     * @param 移除的光盘名
     */
    void diskRemove(QString sDiskName);

public:
    Diskcheckthread();
    /**
     * @file 启动计时器循环检测线程
     */
    void start();
    /**
     * @file 停止定时器
     */
    void stop();

protected slots:
    void diskChecking();

private:
    QMap<QString, QString> m_mapDisk2Name; //光盘挂载路径和光盘挂载名的映射
    QTimer m_timerDiskCheck;               //监测定时器
};

#endif // DISKCHECKTHREAD_H
