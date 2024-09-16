// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONUTILS_H
#define COMMONUTILS_H

#include <QString>

#include <co/flag.h>
#include <co/log.h>

namespace deepin_cross {
class CommonUitls
{
public:
    static std::string getFirstIp();

    static bool isPortInUse(int port);

    static void loadTranslator();

    static void initLog();

    static QString elidedText(const QString &text, Qt::TextElideMode mode, int maxLength);

    static void manageDaemonProcess(const QString &side);

    static bool isFirstStart();

    static QString tipConfPath();

private:
    static QString logDir();
    static bool detailLog();
    static bool isProcessRunning(const QString &processName);
};
}

#endif   // COMMONUTILS_H
