// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "booster-desktop.h"
#include "daemon.h"
#include "search.h"

#include <QApplication>
#include <QWidget>
#include <QImageReader>

#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

const string DesktopBooster::m_boosterType  = "desktop";

const string & DesktopBooster::boosterType() const
{
    return m_boosterType;
}

int DesktopBooster::run(SocketManager * socketManager) {
    std::ifstream infile(m_appData->fileName());
    std::string line;
    std::string exec;
    std::string name;

    while (std::getline(infile, line)) {
        if (!std::strncmp(line.c_str(), "Exec=", 5)) {
            exec = std::move(line);
            continue;
        }

        if (!std::strncmp(line.c_str(), "Name=", 5)) {
            name = std::move(line);
            continue;
        }

        if (!exec.empty() && !name.empty()) {
            break;
        }
    }

    // remove Exec=
    exec = exec.substr(5);
    name = name.substr(5);

    std::vector<std::string> sv;
    std::istringstream iss(exec);
    std::string tmp;
    while (std::getline(iss, tmp, ' ')) {
        sv.emplace_back(std::move(tmp));
    }

    m_appData->setFileName(search_program(sv[0].c_str()));
    m_appData->setAppName(name.c_str());

    //NOTE: 需要管理内存
    char** newargv = new char*[sv.size()];
    for (size_t i = 0; i < sv.size(); ++i) {
        const std::string &str = sv[i];
        newargv[i] = new char[str.length()];
        std::strcpy(newargv[i], str.c_str());
    }

    m_appData->setArgc(sv.size());
    m_appData->setArgv(const_cast<const char**>(newargv));

    return Booster::run(socketManager);
}

void DesktopBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket, int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    static int argc = initialArgc;
    Q_UNUSED(new QApplication(argc, initialArgv));

    Booster::initialize(initialArgc, initialArgv, boosterLauncherSocket, socketFd, singleInstance, bootMode);
}

bool DesktopBooster::preload()
{
    // 初始化QWidget，减少程序启动后第一次显示QWidget时的时间占用
    // 在龙芯和申威上，时间主要消耗在xcb插件中加载glx相关库（libdri600等）
    QWidget widget;

    widget.setWindowFlags(Qt::BypassWindowManagerHint
                          | Qt::WindowStaysOnBottomHint
                          | Qt::WindowTransparentForInput
                          | Qt::WindowDoesNotAcceptFocus);
    widget.setFixedSize(1, 1);
    widget.createWinId();
//    widget.show();

    // 初始化图片解码插件，在龙芯和申威上，Qt程序冷加载图片解码插件几乎耗时1s
    Q_UNUSED(QImageReader::supportedImageFormats());

    return true;
}

int main(int argc, char **argv)
{
    DesktopBooster *booster = new DesktopBooster;

    Daemon d(argc, argv);
    d.run(booster);

    return 0;
}
