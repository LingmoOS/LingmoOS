// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>

class MainJob : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.ddeprinter")

public:
    explicit MainJob(QObject *parent = nullptr);
};

#endif // SERVICE_H
