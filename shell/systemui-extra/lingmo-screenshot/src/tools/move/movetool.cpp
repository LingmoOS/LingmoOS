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

#include "movetool.h"
#include <QPainter>

MoveTool::MoveTool(QObject *parent) : AbstractActionTool(parent) {

}

bool MoveTool::closeOnButtonPressed() const {
    return false;
}

QIcon MoveTool::icon(const QColor &background, bool inEditor) const {
   // Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "cursor-move.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "cursor-move.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "cursor-move.svg");
}
#ifdef SUPPORT_LINGMO
QIcon MoveTool::icon(const QColor &background, bool inEditor, const CaptureContext &context ) const {
   // Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "cursor-move.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "cursor-move.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "cursor-move.svg");
}
#endif
QString MoveTool::name() const {
    return tr("Move");
}

QString MoveTool::nameID() {
    return QLatin1String("");
}

QString MoveTool::description() const {
    return tr("Move the selection area");
}

CaptureTool* MoveTool::copy(QObject *parent) {
    return new MoveTool(parent);
}

void MoveTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
    emit requestAction(REQ_MOVE_MODE);
}
