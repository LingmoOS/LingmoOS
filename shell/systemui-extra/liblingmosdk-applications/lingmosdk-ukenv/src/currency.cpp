/*
 * liblingmosdk-ukenv's Library
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

#include "currency.h"

namespace kdk
{

Currency::Currency() = default;

Currency::~Currency() = default;

QString Currency::getAppName(AppName appName)
{
    switch (appName) {
    case AppName::LingmoCalculator:
        return QString("lingmo-calaulator");
    case AppName::LingmoCalendar:
        return QString("lingmo-calendar");
    case AppName::LingmoCamera:
        return QString("lingmo-camera");
    case AppName::LingmoFontViewer:
        return QString("lingmo-font-viewer");
    case AppName::LingmoGpuController:
        return QString("lingmo-gpu-controller");
    case AppName::LingmoIpmsg:
        return QString("lingmo-ipmsg");
    case AppName::LingmoMusic:
        return QString("lingmo-music");
    case AppName::LingmoPhotoViewer:
        return QString("lingmo-photo-viewer");
    case AppName::LingmoPrinter:
        return QString("lingmo-printer");
    case AppName::LingmoRecorder:
        return QString("lingmo-recorder");
    case AppName::LingmoServiceSupport:
        return QString("lingmo-service-support");
    case AppName::LingmoWeather:
        return QString("lingmo-weather");
    case AppName::LingmoNotebook:
        return QString("lingmo-notebook");
    case AppName::LingmoOsManager:
        return QString("lingmo-os-manager");
    case AppName::LingmoNetworkCheck:
        return QString("lingmo-network-check-tools");
    case AppName::LingmoGallery:
        return QString("lingmo-gallery");
    case AppName::LingmoScanner:
        return QString("lingmo-scanner");
    case AppName::LingmoMobileAssistant:
        return QString("lingmo-mobile-assistant");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

}
