// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H
#include <QObject>

class Utils :public QObject
{
public:
    static bool isWaylandMode;
};

#endif // UTILS_H
