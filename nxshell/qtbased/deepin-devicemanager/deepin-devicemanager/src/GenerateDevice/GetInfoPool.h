// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef READFILEPOOL_H
#define READFILEPOOL_H

#include <QObject>
#include <QThreadPool>

class GetInfoPool;

/**
 * @brief The CmdTask class
 * 线程池的任务类,执行命令的任务类
 */
class CmdTask: public QObject, public QRunnable
{
    Q_OBJECT
public:
    CmdTask(QString key, QString file, QString info, GetInfoPool *parent);
    ~CmdTask();
protected:
    void run() override;

private:
    QString m_Key;
    QString m_File;
    QString m_Info;
    GetInfoPool *mp_Parent;
};


class GetInfoPool : public QThreadPool
{
    Q_OBJECT
public:
    GetInfoPool();

    /**
     * @brief getAllInfo : 加载设备信息
     */
    void getAllInfo();

    /**
     * @brief finishedCmd
     * @param info
     * @param cmdInfo
     */
    void finishedCmd(const QString &info, const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo);
    /**
     * @brief setFramework：设置架构
     * @param arch:架构
     */
    void setFramework(const QString &arch);

signals:
    void finishedAll(const QString &info);

private:
    /**
     * @brief initCmd : 初始化命令列表
     */
    void initCmd();

private:
    QString                      m_Arch;
    QList<QStringList>           m_CmdList;
    int                          m_FinishedNum;
};

#endif // READFILEPOOL_H
