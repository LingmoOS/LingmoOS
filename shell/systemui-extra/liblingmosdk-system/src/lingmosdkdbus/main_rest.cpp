/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include "lingmosdkrest.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDBusConnection con = QDBusConnection::sessionBus();
    KySdkRest res;
    if(!con.registerService("com.lingmo.lingmosdk.service"))
    {
        qDebug() << "com.lingmo.lingmosdk.service 服务注册失败";
        return -1;
    }

    if(!con.registerObject("/com/lingmo/lingmosdk/resolution", &res,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/lingmo/lingmosdk/resolution 对象注册失败";
        return -1;
    }

    return a.exec();
}
