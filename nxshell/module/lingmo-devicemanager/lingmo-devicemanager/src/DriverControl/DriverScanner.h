// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERSCANNER_H
#define DRIVERSCANNER_H

#include "MacroDefinition.h"

#include <QThread>

class DriverScanner : public QThread
{
    Q_OBJECT
public:
    explicit DriverScanner(QObject *parent = nullptr);

    /**
     * @brief run
     */
    void run();

    /**
     * @brief setDriverList
     * @param lstInfo
     */
    void setDriverList(QList<DriverInfo *> lstInfo);

signals:
    void scanInfo(const QString &info, int progress);
    void scanFinished(ScanResult sr);

private:
    QList<DriverInfo *>     m_ListDriverInfo;
    bool m_IsStop;
};

#endif // DRIVERSCANNER_H
