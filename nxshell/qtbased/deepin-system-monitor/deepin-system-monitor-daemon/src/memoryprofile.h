// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMORYPROFILE_H
#define MEMORYPROFILE_H

#include <QObject>

class MemoryProfile : public QObject
{
    Q_OBJECT
public:
    explicit MemoryProfile(QObject *parent = nullptr);

signals:

public slots:

public:
    /*!
     * 更新内存占用率
     */
    double updateSystemMemoryUsage();
    /*!
     * 获取内存占用率
     */
    double getMemUsage();

private:
    double mMemUsage;
};

#endif // MEMORYPROFILE_H
