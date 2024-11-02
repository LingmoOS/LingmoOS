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
#include "toolfactorycap.h"

ToolFactoryCap::ToolFactoryCap(QObject *parent) : QObject(parent)
{

}
CaptureToolCap* ToolFactoryCap::CreateTool(CaptureButtonCap::ButtonType t, QObject *parent)
{
    CaptureToolCap *tool;
    switch (t) {
     case CaptureButtonCap::TYPE_SCREENCUT:
        tool = new ScreenCut(parent);
        break;
     case CaptureButtonCap::TYPE_SCREENCAP:
        tool = new ScreenCap(parent);
        break;
     case CaptureButtonCap::TYPE_SCREENMOUSECURSOR:
        tool = new ScreenMouseCursor(parent);
        break;
    case CaptureButtonCap::TYPE_SCREENVOICE:
        tool = new ScreenVoice(parent);
        break;
    case CaptureButtonCap::TYPE_SCREENMOUSE:
        tool = new ScreenMouse(parent);
        break;
    case CaptureButtonCap::TYPE_SCREENCAPOPTION:
        tool = new ScreenCapOption(parent);
        break;
    case CaptureButtonCap::TYPE_SCREENCAPEXIT:
        tool = new ScreenCapExit(parent);
        break;
    case CaptureButtonCap::TYPE_SCREENCAPSTART:
        tool = new ScreenCapStart(parent);
        break;
    default:
        tool = nullptr;
        break;
    }
    return tool;
}
