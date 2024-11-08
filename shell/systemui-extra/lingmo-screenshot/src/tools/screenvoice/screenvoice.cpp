/*
 *
 * Copyright:  2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@cs2c.com.cn.com>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#include "screenvoice.h"

ScreenVoice::ScreenVoice(QObject *parent) : CaptureToolCap(parent)
{

}
QString ScreenVoice::name() const
{
    return tr("Options");
}
QString ScreenVoice::nameID()
{
     return QLatin1String("");
}
QString ScreenVoice::description() const
{
    return tr("options tool");
    //return tr("图片保存配置");
}
bool ScreenVoice::isValid() const
{
    return true;
}
