// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERF_H
#define PERF_H
#include <QObject>
#include <QMap>
#include <QString>

/**
 * @brief The PointInfo struct
 */
struct PointInfo {
    QString desc;
    qint64 time;
};

class DebugTimeManager
{
public:
    /**
     * @brief getInstance : get signal instance
     * @return : the signal instance
     */
    static DebugTimeManager *getInstance()
    {
        if (!s_Instance) {
            s_Instance = new DebugTimeManager();
        }
        return s_Instance;
    }

    /**
     * @brief clear : clear data
     */
    void clear();

    /**
     * @brief beginPointLinux : 打点开始,Linux
     * @param point : 所打的点的名称，固定格式，在打点文档中查看 -- POINT-XX POINT-01
     * @param status : 性能测试的状态，比如测试时文件的大小
     */
    void beginPointLinux(const QString &point, const QString &status = "");

    /**
     * @brief endPointLinux : 结束打点,Linux
     * @param point : 需要结束的点
     */
    void endPointLinux(const QString &point);

protected:
    DebugTimeManager();

private:
    static DebugTimeManager *s_Instance; //<! singal install
    QMap<QString, PointInfo> m_MapPoint; //<! 保存所打的点
};

//默认开启时间打印
#define PERF_ON
#ifdef PERF_ON
#define PERF_PRINT_BEGIN(printStr, Description) \
    DebugTimeManager::getInstance()->beginPointLinux(printStr, Description)
#define PERF_PRINT_END(printStr) \
    DebugTimeManager::getInstance()->endPointLinux(printStr)
#else
#define PERF_PRINT_BEGIN(printStr, Description)
#define PERF_PRINT_END(printStr)
#endif

#endif // PERF_H
