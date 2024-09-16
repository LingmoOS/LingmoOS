// SPDX-FileCopyrightText: 2023-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "manager/sessionmanager.h"
#include "common/commonutils.h"

//#include <unistd.h>
#include <iostream>
#include <QStringList>
#include <QApplication>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QThread>

#define COO_SESSION_PORT 51588
#define COO_HARD_PIN "515616"

#define COO_WEB_PORT 51568

using namespace deepin_cross;

int main(int argc, char** argv)
{
    CommonUitls::initLog();

    ExtenMessageHandler msg_cb([](int32_t mask, const picojson::value &json_value, std::string *res_msg) -> bool {
        LOG << "Main >> " << mask <<" msg_cb, json_msg: " << json_value;
        return false;
    });
    auto sessionManager = new SessionManager();
    sessionManager->setSessionExtCallback(msg_cb);
    sessionManager->updatePin(COO_HARD_PIN);
    sessionManager->sessionListen(COO_SESSION_PORT);

    // 获取下载目录
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    sessionManager->setStorageRoot(downloadDir);
    sessionManager->updateSaveFolder("launcher");

    LOG << "start listen........" << COO_SESSION_PORT;

    qputenv("QT_LOGGING_RULES", "cooperation-launcher.debug=true");

#if 0
    QString ip = "10.8.11.98";
    LOG << "connect remote " << ip.toStdString();
    sessionManager->sessionConnect(ip, COO_SESSION_PORT, COO_HARD_PIN);
    QThread::msleep(100);
    LOG << "sending file.........." << ip.toStdString();
    QStringList fileList;
    fileList << "/home/doll/Downloads/launcher/Fuck floder";
    fileList << "/home/doll/Downloads/launcher/【成研】跨端冒烟测试1.xlsx";
    sessionManager->sendFiles(ip, COO_WEB_PORT, fileList);
#endif
    QApplication app(argc, argv);
    app.exec();

    LOG << "Done!";

    return 0;
}
