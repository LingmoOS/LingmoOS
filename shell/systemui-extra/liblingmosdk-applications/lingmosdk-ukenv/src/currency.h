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

#ifndef LINGMOSDK_UKENV_CURRENCY_H_
#define LINGMOSDK_UKENV_CURRENCY_H_

#include <QString>

namespace kdk
{

enum AppName {
    LingmoIpmsg = 0,       /* 传书 */
    LingmoFontViewer,      /* 字体管理器 */
    LingmoCalculator,      /* 灵墨计算器 */
    LingmoGpuController,   /* 显卡控制器 */
    LingmoMusic,           /* 音乐 */
    LingmoWeather,         /* 天气 */
    LingmoPhotoViewer,     /* 看图 */
    LingmoServiceSupport,  /* 服务与支持 */
    LingmoPrinter,         /* 灵墨打印 */
    LingmoCalendar,        /* 日历 */
    LingmoRecorder,        /* 录音 */
    LingmoCamera,          /* 摄像头 */
    LingmoNotebook,        /* 便签 */
    LingmoOsManager,       /* 灵墨管家 */
    LingmoNetworkCheck,    /* 网络检测工具 */
    LingmoGallery,         /* 相册 */
    LingmoScanner,         /* 扫描 */
    LingmoMobileAssistant, /* 多端协同 */
    LingmoTest             /* 测试预留 */
};

class Currency
{
public:
    Currency();
    ~Currency();

    static QString getAppName(AppName appName);
};

} // namespace kdk

#endif
