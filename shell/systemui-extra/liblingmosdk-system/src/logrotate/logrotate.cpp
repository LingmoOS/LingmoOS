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

#include "logrotate.h"
#include <QFile>
#include <QDir>
#include <QDebug>

#define DefaultRule "{\n\tdaily\n\trotate 5\n\tcompress\n\tdateext\n\tmissingok\n\tnotifempty\n\tcopytruncate\n\tsu root root\n}\n"

LogRotate::LogRotate(QObject *parent) : QObject(parent)
{
}

int LogRotate::setConfig(QString path)
{
    qDebug() << "begin";
    int ret = -1;
    QString logfile = path.mid(path.lastIndexOf('/') + 1);
    if(!logfile.endsWith(".log"))
        return ret;

    QString fileName = logfile.left(logfile.length() - 4);
    QDir dir("/etc/logrotate.d/kylog-rotate");
    if (!dir.exists())
        dir.mkdir("/etc/logrotate.d/kylog-rotate");
    QFile file(QString("/etc/logrotate.d/kylog-rotate/%1").arg(fileName));
    do
    {

        bool exist = file.exists();

        if(!file.open(QFile::ReadWrite | QFile::Text))
        {
            qDebug() << "打开文件失败";
            break;
        }

        QString conf;
        if(exist)
        {
            conf = file.readAll();
            if(!conf.contains(path))
                conf = QString("%1\n%2").arg(path).arg(conf).toStdString().c_str();
        }
        else
            conf = QString("%1\n%2").arg(path).arg(DefaultRule).toStdString().c_str();
        file.reset();
        file.write(conf.toLatin1());
        ret = 0;
    } while (0);

    file.close();
    qDebug() << "end";
    return ret;
}
