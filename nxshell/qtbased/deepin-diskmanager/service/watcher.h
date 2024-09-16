// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef WATCHER_H
#define WATCHER_H

#include <QObject>
#include <QThread>

namespace DiskManager {

/**
 * @class ProbeThread
 * @brief 硬件信息刷新类
 */
class Watcher :public QThread
{
    Q_OBJECT
public:

    /**
     * @brief 监测是前端是否启动又退出
     */
    void run();

    void executCmd(const QString &strCmd, QString &outPut, QString &error);

public Q_SLOTS:
    void exit();

private:
    std::atomic_bool stoped { false };
};
}
#endif // WATCHER_H
