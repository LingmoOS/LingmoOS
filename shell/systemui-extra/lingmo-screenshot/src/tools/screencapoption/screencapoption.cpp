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
#include "screencapoption.h"

ScreenCapOption::ScreenCapOption(QObject *parent) : CaptureToolCap(parent)
{

}
QString ScreenCapOption::name() const
{
    return tr("Options");
}
QString ScreenCapOption::nameID()
{
     return QLatin1String("");
}
QString ScreenCapOption::description() const
{
    return tr("options tool");
    //return tr("图片保存配置");
}
bool ScreenCapOption::isValid() const
{
    return true;
}
