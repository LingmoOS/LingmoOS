// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icondata.h"
#include <QDebug>

QDebug operator<<(QDebug arg, const IconData &data)
{
    arg << "width = " << data.width << Qt::endl;
    arg << "height = " << data.height << Qt::endl;
    arg << "rowstride = " << data.rowstride << Qt::endl;
    arg << "alpha = " << data.alpha << Qt::endl;
    arg << "per = " << data.bit << Qt::endl;
    arg << "cancel = " << data.cannel << Qt::endl;
    arg << "array = " << data.array << Qt::endl;
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const IconData &data)
{
    arg.beginStructure();
    arg << data.width << data.height << data.rowstride << data.alpha << data.bit << data.cannel << data.array;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, IconData &data)
{
    arg.beginStructure();
    arg >> data.width >> data.height >> data.rowstride >> data.alpha >> data.bit >> data.cannel >> data.array;
    arg.endStructure();

    return arg;
}
