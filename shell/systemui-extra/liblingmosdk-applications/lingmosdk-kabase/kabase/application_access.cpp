/*
 * liblingmosdk-base's Library
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include "application_access.hpp"
#include "dbus.hpp"
#include "log.hpp"

namespace kdk
{
namespace kabase
{

ApplicationAccess::ApplicationAccess() = default;

ApplicationAccess::~ApplicationAccess() = default;

bool ApplicationAccess::exportFunc(AppName appName, QObject *object)
{
    if (!DBus::registerService(getServiceName(appName))) {
        error << "kabase : register service fail!";
        return false;
    }

    if (!DBus::registerObject(getObjectPath(appName), object)) {
        error << "kabase : register object fail!";
        return false;
    }

    return true;
}

QList<QVariant> ApplicationAccess::callMethod(AppName appName, QString methodName, QList<QVariant> args)
{
    return DBus::callMethod(getServiceName(appName), getObjectPath(appName), getInterfaceName(appName), methodName,
                            args);
}

QString ApplicationAccess::getServiceName(AppName appName)
{
    switch (appName) {
    case AppName::LingmoCalculator:
        return QString("org.lingmo.calculator");
    case AppName::LingmoCalendar:
        return QString("org.lingmo.calendar");
    case AppName::LingmoCamera:
        return QString("org.lingmo.camera");
    case AppName::LingmoFontViewer:
        return QString("org.lingmo.font.viewer");
    case AppName::LingmoGpuController:
        return QString("org.lingmo.gpu.controller");
    case AppName::LingmoIpmsg:
        return QString("org.lingmo.ipmsg");
    case AppName::LingmoMusic:
        return QString("org.lingmo.music");
    case AppName::LingmoPhotoViewer:
        return QString("org.lingmo.photo.viewer");
    case AppName::LingmoPrinter:
        return QString("org.lingmo.printer");
    case AppName::LingmoRecorder:
        return QString("org.lingmo.recorder");
    case AppName::LingmoServiceSupport:
        return QString("org.lingmo.service.support");
    case AppName::LingmoWeather:
        return QString("org.lingmo.weather");
    case AppName::LingmoNotebook:
        return QString("org.lingmo.notebook");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

QString ApplicationAccess::getObjectPath(AppName appName)
{
    switch (appName) {
    case AppName::LingmoCalculator:
        return QString("/org/lingmo/calculator");
    case AppName::LingmoCalendar:
        return QString("/org/lingmo/calendar");
    case AppName::LingmoCamera:
        return QString("/org/lingmo/camera");
    case AppName::LingmoFontViewer:
        return QString("/org/lingmo/font/viewer");
    case AppName::LingmoGpuController:
        return QString("/org/lingmo/gpu/controller");
    case AppName::LingmoIpmsg:
        return QString("/org/lingmo/ipmsg");
    case AppName::LingmoMusic:
        return QString("/org/lingmo/music");
    case AppName::LingmoPhotoViewer:
        return QString("/org/lingmo/photo/viewer");
    case AppName::LingmoPrinter:
        return QString("/org/lingmo/printer");
    case AppName::LingmoRecorder:
        return QString("/org/lingmo/recorder");
    case AppName::LingmoServiceSupport:
        return QString("/org/lingmo/service/support");
    case AppName::LingmoWeather:
        return QString("/org/lingmo/weather");
    case AppName::LingmoNotebook:
        return QString("/org/lingmo/notebook");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

QString ApplicationAccess::getInterfaceName(AppName appName)
{
    switch (appName) {
    case AppName::LingmoCalculator:
        return QString("org.lingmo.calculator.interface");
    case AppName::LingmoCalendar:
        return QString("org.lingmo.calendar.interface");
    case AppName::LingmoCamera:
        return QString("org.lingmo.camera.interface");
    case AppName::LingmoFontViewer:
        return QString("org.lingmo.font.viewer.interface");
    case AppName::LingmoGpuController:
        return QString("org.lingmo.gpu.controller.interface");
    case AppName::LingmoIpmsg:
        return QString("org.lingmo.ipmsg.interface");
    case AppName::LingmoMusic:
        return QString("org.lingmo.music.interface");
    case AppName::LingmoPhotoViewer:
        return QString("org.lingmo.photo.viewer.interface");
    case AppName::LingmoPrinter:
        return QString("org.lingmo.printer.interface");
    case AppName::LingmoRecorder:
        return QString("org.lingmo.recorder.interface");
    case AppName::LingmoServiceSupport:
        return QString("org.lingmo.service.support.interface");
    case AppName::LingmoWeather:
        return QString("org.lingmo.weather.interface");
    case AppName::LingmoNotebook:
        return QString("org.lingmo.notebook.interface");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

} /* namespace kabase */
} /* namespace kdk */
