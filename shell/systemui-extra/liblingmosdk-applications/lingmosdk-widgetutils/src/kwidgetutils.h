/*
 * liblingmosdk-widgetutils's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KWIDGETUTILS_H
#define KWIDGETUTILS_H

#include "lingmosdk-widgetutils_global.h"
namespace kdk
{
class LINGMOSDKWIDGETUTILS_EXPORT KWidgetUtils
{
public:
    KWidgetUtils();

    /**
     * @brief 适配高分屏，需要在定义QApplication对象前，调用该函数
     *     KWidgetUtils::highDpiScaling();
     *     QApplication a(argc, argv);
     */
    static void highDpiScaling();


    /**
     * @brief 判断合成器是否开启
     *
     * @param 无
     *
     * @retval true 开启
     * @retval false 没开启
     */
    static bool checkCompositorRunning(void);
};
}

#endif // KWIDGETUTILS_H
