// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTIMEDLOOP_H
#define DTIMEDLOOP_H
#include <dtkcore_global.h>
#include <DObject>

#include <QEventLoop>

DCORE_BEGIN_NAMESPACE

class DObject;
class DTimedLoopPrivate;
class DTimedLoop : public QEventLoop,  public DObject {
    Q_OBJECT
public:
    explicit DTimedLoop() noexcept;
    explicit DTimedLoop(QObject *parent) noexcept;

    ~DTimedLoop();

    // 如果是 isRunning 则返回从开始到现在的 exec 执行时间，否则返回上次运行的时间
    int runningTime();
    void setTimeDump(bool flag = true);

    void exit(int returnCode = 0);

    // 方式1：不传定时时间，如果不退出就一直执行，配合 exit 使用
    // 方式2：传入durationMs 参数的是定时执行的，也能调用 exit 提前退出
    // 如果传入了 executionName 就会为本次执行设置一个名字，会输出到 log
    // 在执行结束将会打印 exec 的执行时间，可以用 setTimeDump 控制其是否打印
    int exec(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    int exec(int durationMs, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    int exec(const QString &executionName, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    int exec(int durationMs, const QString &executionName, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);

private:
    Q_DISABLE_COPY(DTimedLoop)
    D_DECLARE_PRIVATE(DTimedLoop)
};

DCORE_END_NAMESPACE

#endif // DTIMEDLOOP_H
