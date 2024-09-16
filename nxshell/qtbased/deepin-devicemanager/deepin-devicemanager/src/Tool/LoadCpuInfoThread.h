// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADCPUINFOTHREAD_H
#define LOADCPUINFOTHREAD_H

#include <QThread>

class LoadCpuInfoThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadCpuInfoThread();
    /**
     * @brief run
     */
    virtual void run() override;

signals:

public slots:

private:
    /**
     * @brief runCmd
     * @param info
     * @param cmd
     */
    void runCmd(QString &info, const QString &cmd);

    /**
     * @brief loadXrandrInfo:加载xrandr获取的信息
     * @param lstMap
     * @param key:命令
     */
    void loadCpuInfo(QMap<QString, QString> &mapInfo, const QString &cmd);

    /**
     * @brief getMapInfoFromCmd:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getCpuInfoFromLscpu:根据lscpu获取CPU信息
     */
    void getCpuInfoFromLscpu();
};

#endif // LOADCPUINFOTHREAD_H
