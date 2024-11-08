/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*               2020 KylinSoft Co., Ltd.
* This file is part of Lingmo-Screenshot.
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

#include "undotool.h"
#include <QPainter>

UndoTool::UndoTool(QObject *parent) : AbstractActionTool(parent)
{
}

bool UndoTool::closeOnButtonPressed() const
{
    return false;
}

QIcon UndoTool::icon(const QColor &background, bool inEditor) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "undo-variant.svg");
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "undo-variant.svg")
           : QIcon(QStringLiteral(":/img/material/white/") + "undo-variant.svg");
}

#ifdef SUPPORT_LINGMO
QIcon UndoTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    if ((context.style_name.compare("lingmo-dark") == 0)
        || (context.style_name.compare("lingmo-black") == 0)) {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "undo-variant.svg")
               : QIcon(QStringLiteral(":/img/material/dark-theme/") + "undo-variant.png");
    } else {
        // if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "undo-variant.svg")
               : QIcon(QStringLiteral(":/img/material/white/") + "undo-variant.svg");
    }
}

#endif
QString UndoTool::name() const
{
    return tr("undo");
}

QString UndoTool::nameID()
{
    return QLatin1String("");
}

QString UndoTool::description() const
{
    return tr("Undo the last modification");
}

CaptureTool *UndoTool::copy(QObject *parent)
{
    return new UndoTool(parent);
}

void UndoTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_UNDO_MODIFICATION);
}
