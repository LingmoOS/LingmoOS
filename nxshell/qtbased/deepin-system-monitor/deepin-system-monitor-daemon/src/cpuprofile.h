// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPUPROFILE_H
#define CPUPROFILE_H

#include <QObject>
#include <QMap>

class CpuProfile : public QObject
{
    Q_OBJECT
public:
    explicit CpuProfile(QObject *parent = nullptr);

signals:

public slots:

public:
    /*!
     * 更新CPU占用率
     */
    double updateSystemCpuUsage();
    /*!
     * 获取CPU占用率
     */
    double getCpuUsage();
    /*!
     * 获取当前CPU状态
     */
    QMap<QString, int> cpuStat();

private:
    QMap<QString, int> mLastCpuStat;
    double mCpuUsage;
};

#endif // CPUPROFILE_H
