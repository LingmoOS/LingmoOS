// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QDir>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.process(app);
    const QStringList fileList = parser.positionalArguments();
    if (fileList.count() < 1) {
        return 0;
    }
    QStringList arg;
    if (fileList[0] == "dmesg") {
        arg << "-c"
            << "dmesg -C";
    } else {
        arg << "-c" << QString("truncate -s 0 %1").arg(fileList[0]);
    }

    QProcess proc;
    proc.start("/bin/bash", arg);
    proc.waitForFinished(-1);
    //直接
    QByteArray byte =   proc.readAllStandardOutput();
    //    QTextStream stream(&byte);
    //    QByteArray encode;
    //    stream.setCodec(encode);
    //    QString str = stream.readAll();
    //必须要replace \u0000,不然QByteArray会忽略这以后的内容
    std::cout << byte.replace('\u0000', "").data();
    proc.close();
    return 0;
}
