// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include <QThread>

namespace dmr {

// Virtual button codes that are not defined by X11.
#define Button1         1
#define Button2         2
#define Button3         3
#define WheelUp         4
#define WheelDown       5
#define WheelLeft       6
#define WheelRight      7
#define XButton1        8
#define XButton2        9
/**
 * @file x11事件过滤器线程
 */
class EventMonitor : public QThread
{
    Q_OBJECT

public:
    explicit EventMonitor(QObject *parent = nullptr);
    /**
     * @brief 事件处理函数，处理点击事件
     */
    void handleRecordEvent(void *);
    /**
     * @brief 应用进入激活状态
     */
    void resumeRecording();
    /**
     * @brief 应用失去选中状态
     */
    void suspendRecording();

signals:
    /**
     * @brief 鼠标按下信号
     * @param 按下的横坐标
     * @param 纵坐标
     */
    void buttonedPress(int nPosX, int nPosY);
    /**
     * @brief 鼠标拖动信号
     * @param 拖动的横坐标
     * @param 纵坐标
     */
    void buttonedDrag(int nPosX, int nPosY);
    /**
     * @brief 鼠标释放信号
     * @param 按下的横坐标
     * @param 纵坐标
     */
    void buttonedRelease(int nPosX, int nPosY);

//cppcheck 误报
protected:
    void run();

private:
    bool m_bIsPress;            ///记录鼠标是否按下标志位
    QAtomicInt m_recording {1}; ///记录应用是否是激活状态
};

}
#endif
