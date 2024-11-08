/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "screencapexit.h"

ScreenCapExit::ScreenCapExit(QObject *parent) : CaptureToolCap(parent)
{

}
QString ScreenCapExit::name() const
{
    return tr("Options");
}
QString ScreenCapExit::nameID()
{
     return QLatin1String("");
}
QString ScreenCapExit::description() const
{
    return tr("options tool");
    //return tr("图片保存配置");
}
bool ScreenCapExit::isValid() const
{
    return true;
}
