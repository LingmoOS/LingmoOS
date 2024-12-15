// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THREADEXECXRANDR_H
#define THREADEXECXRANDR_H

#include <QThread>

class ThreadExecXrandr : public QThread
{
public:
    ThreadExecXrandr(bool gpu, bool isDXcbPlatform);

    /**
     * @brief run
     */
    virtual void run() override;
    /**
     * @brief getMonitorNumber
     */
    int getMonitorNumber() { return m_monitorLst.size(); }

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
     * @param cmd:调试文件名
     */
    void loadXrandrInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief loadXrandrVerboseInfo:加载xrandr --verbose获取的信息
     * @param lstMap
     * @param cmd:调试文件名
     */
    void loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief getRefreshRateFromDBus:通过dbus获取刷新率
     * @param lstMap
     */
    void getRefreshRateFromDBus(QList<QMap<QString, QString>> &lstMap);

    /**
     * @brief getResolutionFromDBus:通过dbus获取分辨率
     * @param lstMap
     */
    void getResolutionFromDBus(QMap<QString, QString> &lstMap);

    /**
     * @brief getResolutionRateFromDBus:通过dbus获取分辨率和刷新率
     * @param lstMap
     */
    void getResolutionRateFromDBus(QList<QMap<QString, QString> > &lstMap);

    /**
     * @brief getMonitorInfoFromXrandrVerbose:从xrandr --verbose获取显示设备信息
     */
    void getMonitorInfoFromXrandrVerbose();

    /**
     * @brief getGpuInfoFromXrandr:从xrandr获取显卡信息
     */
    void getGpuInfoFromXrandr();


private:
    bool m_Gpu;                //<!  判断是否是gpu
    bool m_isDXcbPlatform;     //<!  判断是否是DXcbPlatform
    QStringList m_monitorLst;
};

#endif // THREADEXECXRANDR_H
