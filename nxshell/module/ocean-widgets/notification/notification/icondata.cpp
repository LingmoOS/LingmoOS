// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icondata.h"
#include <QDebug>

QDebug operator<<(QDebug arg, const IconData &data)
{
    arg << "width = " << data.width << endl;
    arg << "height = " << data.height << endl;
    arg << "rowstride = " << data.rowstride << endl;
    arg << "alpha = " << data.alpha << endl;
    arg << "per = " << data.bit << endl;
    arg << "cancel = " << data.cannel << endl;
    arg << "array = " << data.array << endl;
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
