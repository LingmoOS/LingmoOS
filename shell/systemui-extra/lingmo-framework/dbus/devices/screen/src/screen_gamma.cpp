/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screen_gamma.h"


ScreenGamma::ScreenGamma(QObject *parent)
{
    qDebug() << "init screenBrightNode";
}

ScreenGamma::~ScreenGamma()
{

}

bool ScreenGamma::setScreenBrightness(QString name, uint screenBrightness)
{
    QDBusReply<int> reply = callGamma("setScreenBrightness", MY_NAME, name, screenBrightness);
    if (false == reply.isValid()) {
        return false;
    }

    return reply.value() >= 0;
}

uint ScreenGamma::getScreenBrightness(QString name)
{
    QDBusReply<OutputGammaInfoList> reply = callGamma("getScreensGammaList", MY_NAME);

    if (false == reply.isValid()) {
        return false;
    }

    Q_FOREACH(const OutputGammaInfo info, reply.value()) {
        if (info.outputName == name) {
            return info.brignthess;
        }
    }
    return -1;
}

bool ScreenGamma::setScreensBrightness(OutputGammaInfoList outputsInfo)
{
    Q_FOREACH(const OutputGammaInfo &info, outputsInfo) {
        callGamma("setScreenBrightness", MY_NAME, info.outputName, info.brignthess);
    }

    return true;
}

bool ScreenGamma::setAllScreenSameBrightness(uint brighenss)
{
    QDBusReply<bool> reply = callGamma("setAllScreenBrightness", MY_NAME, (int)brighenss);
    if (false == reply.isValid()) {
        qDebug() << reply.error();
        return false;
    }

    return reply.value();
}

OutputGammaInfoList ScreenGamma::getAllScreenBrightness()
{
    OutputGammaInfoList ret;

    QDBusReply<OutputGammaInfoList>  reply = callGamma("getScreensGammaList", MY_NAME);

    if (false == reply.isValid()) {
        return ret;
    }
    return reply.value();
}

QDBusMessage ScreenGamma::callGamma(const QString &method, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4)
{
    QDBusInterface globalManagerInterface("org.lingmo.SettingsDaemon",
                                  "/org/lingmo/SettingsDaemon/GammaManager",
                                  "org.lingmo.SettingsDaemon.GammaManager");

    return globalManagerInterface.call(method,arg1,arg2,arg3,arg4);
}
