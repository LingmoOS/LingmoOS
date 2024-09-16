// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "func.h"
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <dpinyin.h>
#include <sys/resource.h>

bool Func::isNetConnect()
{
    QProcess process;
    QStringList list;
    list << "-i"
         << "www.baidu.com";
    process.start("curl", list);
    process.waitForFinished(3000);
    if(!process.exitCode()){
        return true;
    }
    return isLanConnect();
}

bool Func::isHTTPConnect()
{
    //    if (!isBt()) {
    //        return false;
    //    }
    return isNetConnect();
}

bool Func::isIPV6Connect()
{
    QProcess process;
    process.start("ifconfig");
    process.waitForFinished(-1);
    QString str = process.readAllStandardOutput();
    QStringList strList = str.split("\n");
    for(int i = 0 ;i < strList.size(); i++){
        if(strList[i].contains("inet6 fe80::")){
            return true;
        }
    }
    return false;
}
        

bool Func::setMimeappsValue(QString key, QString value)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/mimeapps.list";
    QFile readFile(path);
    if (!readFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error";
        return false;
    }
    QTextStream data(&readFile);
    bool isDefault = false;
    bool isAdded = false;
    QStringList DefaultList;
    QStringList AddedList;
    //找到 [Default Applications] 和[Added Associations] 下面中的归类，放入list中
    while (!data.atEnd()) {
        QString sLine = data.readLine();
        if (sLine == "[Default Applications]") {
            isDefault = true;
            isAdded = false;
        } else if (sLine == "[Added Associations]") {
            isDefault = false;
            isAdded = true;
        }
        if (isDefault) {
            DefaultList.append(sLine);
        }
        if (isAdded) {
            AddedList.append(sLine);
        }
    }
    //设置key和value,如果key已经存在，将value设置。  如果key不存在，在后追加一行
    if (!DefaultList.isEmpty()) {
        for (int i = 0; i < DefaultList.size(); i++) {
            if (DefaultList[i].contains(key)) {
                if(DefaultList[i] != key+ "=" + value){
                    DefaultList[i] = key+ "=" + value;
                }
                break;
            }
            if (i == DefaultList.size() - 1 && !(DefaultList[i].contains(key))) {
                DefaultList.append(key+ "=" + value);
                break;
            }
        }
    }
    //设置key和value,如果key已经存在，将value设置。  如果key不存在，在后追加一行
    if (!AddedList.isEmpty()) {
        for (int i = 0; i < AddedList.size(); i++) {
            if (AddedList[i].contains(key)) {
                if(AddedList[i] != key+ "=" + value){
                    AddedList[i] = key+ "=" + value;
                }
                break;
            }
            if (i == AddedList.size() - 1 && !(AddedList[i].contains("application/x-bittorrent"))) {
                AddedList.append(key+ "=" + value);
                break;
            }
        }
    }
    readFile.close();

    //将替换以后的字符串，重新写入到文件中去
    QFile writerFile(path);
    DefaultList << AddedList;
    if (writerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    }
    QTextStream writeData(&writerFile);

    for (int i = 0; i < DefaultList.size(); i++) {
        writeData << DefaultList[i] << endl;
    }
    writeData.flush();
    writerFile.close();
    return true;
}

QString Func::pathToMD5(QString path)
{
    QFile theFile(path);
    theFile.open(QIODevice::ReadOnly);
    QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
    theFile.close();
    return  ba.toHex().constData();
}

//QString Func::getIniConfigValue(QString path, QString group, QString key)
//{
//    QFile f(path);
//    if (!f.open(QIODevice::ReadOnly)) {
//        qDebug() << "error";
//        return "";
//    }
//     QTextStream data(&f);
//     group = QString("[%1]").arg(group);
//     bool isGroup = false;
//     while (!data.atEnd()) {
//         QString sLine = data.readLine();
//         if(sLine.isNull()){
//             continue;
//         }
//         if(sLine == group){
//            isGroup = true;
//            continue;
//         }
//         if(isGroup){
//             if(sLine.startsWith('[') && sLine.endsWith(']')){
//                 isGroup = false;
//                 continue;
//             }
//             if(sLine.split('=')[0] == key){
//                 return sLine.split('=')[1];
//             }
//         }
//     }
//}

//bool Func::setIniConfigValue(QString path, QString group, QString key, QString value)
//{
//    QFile f(path);
//    if (!f.open(QIODevice::ReadWrite)) {
//        qDebug() << "error";
//        return false;
//    }
//    QTextStream data(&f);
//    QStringList dataList = data.readAll().split("\n");
//    group = QString("[%1]").arg(group);
//    bool isGroup = false;
//    for (int i = 0; i < dataList.size(); i++) {
//        if(dataList[i].isNull()){
//            continue;
//        }
//        if(dataList[i] == group){
//            isGroup = true;
//            continue;
//        }
//        if(isGroup){
//            if(dataList[i].startsWith('[') && dataList[i].endsWith(']')){
//                isGroup = false;
//                continue;
//            }
//            if(dataList[i].split('=')[0] == key){
//                dataList[i] = dataList[i].split('=')[0] + "=" + value;

//                //将替换以后的字符串，重新写入到文件中去
//                QFile writerFile(path);
//                if (writerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
//                }
//                QTextStream writeData(&writerFile);
//                for (int i = 0; i < dataList.size(); i++) {
//                    writeData << dataList[i] <<endl;
//                }
//                writeData.flush();
//                writerFile.close();
//                return true;
//            }
//        }
//    }
//    return false;
//}

double Func::formatSpeed(QString str)
{
    QString number = str;
    if (str.contains("GB/s")) {
        str.remove("GB/s");
    }if (str.contains("KB/s")) {
        str.remove("KB/s");
    } else if (str.contains("MB/s")) {
        str.remove("MB/s");
    } else if (str.contains("B/s")) {
        str.remove("B/s");
    }
    double num = str.toDouble();
    if (number.contains("KB")) {
        num = num * 1024;
    } else if (number.contains("MB")) {
        num = num * 1024 * 1024;
    } else if (number.contains("GB")) {
        num = num * 1024 * 1024 * 1024;
    }
    return num;
}

double Func::formatFileSize(QString str)
{
    double num = -1;
    QString number = str.left(str.length() - 2);
    num = number.toDouble();
    if (str.contains("KB")) {
        num = num * 1024;
    } else if (str.contains("MB")) {
        num = num * 1024 * 1024;
    } else if (str.contains("GB")) {
        num = num * 1024 * 1024 * 1024;
    }
    return num;
}

QString Func::chineseToPinyin(QString input)
{
    if (input.isEmpty()) {
        return "";
    }
    QString value = input;
    for(int i = input.size() - 1; i >= 0; i--) {
        QString ch = input.at(i);
        if(ch.contains(QRegExp("[\\x4e00-\\x9fa5]+"))){
            QString pinyin = removeDigital(DTK_NAMESPACE::Core::Chinese2Pinyin(ch));
            value.replace(ch, pinyin);
        }
    }
    return value;
}

QString Func::removeDigital(QString input)
{
    if ("" == input) {
        return "";
    }
    QString value = "";
    QByteArray ba = input.toLocal8Bit();
    char *data = nullptr;
    data = ba.data();
    while (*data) {
        if (!(*data >= '0' && *data <= '9')) {
            value += *data;
        }
        data++;
    }
    return value;
}

bool Func::isLanConnect()
{
    QString geteWay;
    QString str;
    {
        QProcess netStatProcess;
        QStringList netArgv;
        netArgv << "-r";
        netStatProcess.start("netstat", netArgv);
        netStatProcess.waitForFinished();
        str = netStatProcess.readAllStandardOutput();
        netStatProcess.kill();
        netStatProcess.close();
        netStatProcess.deleteLater();
        QStringList netstatInfo = str.split('\n');
        QStringList geteWayList;
//        QStringList::iterator it;
        for(int i = 2; i < netstatInfo.size()-1; i++){
            geteWayList = netstatInfo[i].split(' ');
            geteWayList.removeAll(QString(""));
//            it = geteWayList.begin();
//            for ( ; it != geteWayList.end(); it++) {
//                QString a = *it;
//                if(it->isEmpty()){
//                    geteWayList.erase(it);
//                }
//            }
            if(geteWayList[1] != "0.0.0.0")
            {
                geteWay = geteWayList[1];
                break;
            }
        }
    }
    if(geteWay.isEmpty()){
        return false;
    } else {
        return true;
    }

//    QStringList nmapInfo;
//    QStringList lanInfo;
//    {
//        QProcess nmapProcess;
//        QStringList nmapArgv;
//        nmapArgv << "-sP" << QString("%1\/24").arg(geteWay);
//        nmapProcess.start("nmap", nmapArgv);
//        nmapProcess.waitForFinished();
//        str = nmapProcess.readAllStandardOutput();
//        nmapProcess.kill();
//        nmapProcess.close();
//        nmapInfo = str.split('\n');
//        if(nmapInfo.size() < 4){
//            return false;
//        }
//        lanInfo = nmapInfo[nmapInfo.size()-4].split(" ");
//    }

//    {
//        if(lanInfo[lanInfo.size()-1] != geteWay){
//            QProcess *lanProcess = new QProcess();
//            QStringList pingArgv;
//            pingArgv<< "-W" <<  "1" << "-c" << "1" << lanInfo[lanInfo.size()-1];
//            lanProcess->start("ping", pingArgv);
//            lanProcess->waitForFinished();
//            str = lanProcess->readAllStandardOutput();
//            lanProcess->kill();
//            lanProcess->close();
//            lanProcess->deleteLater();
//            delete lanProcess;
//            QStringList lanPingInfo = str.split("\n");
//            if(lanPingInfo[lanPingInfo.size() -3].contains("0% packet loss")){
//                return true;
//            }
//        }
//    }
//    return false;
}




