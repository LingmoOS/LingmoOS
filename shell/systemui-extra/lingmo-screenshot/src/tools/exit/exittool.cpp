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

#include "exittool.h"
#include <QPainter>

ExitTool::ExitTool(QObject *parent) : AbstractActionTool(parent) {

}

bool ExitTool::closeOnButtonPressed() const {
    return true;
}

QIcon ExitTool::icon(const QColor &background, bool inEditor) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "close.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "close.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "close.svg");
}

#ifdef SUPPORT_LINGMO
QIcon ExitTool::icon(const QColor &background, bool inEditor,const CaptureContext &context ) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "close.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "close.svg") :
                       QIcon(QStringLiteral(":/img/material/white/") + "close.svg");
}
#endif
QString ExitTool::name() const {
    return tr("Exit");
}

QString ExitTool::nameID() {
    return QLatin1String("");
}

QString ExitTool::description() const {
    return tr("Leave the capture screen");
}

CaptureTool* ExitTool::copy(QObject *parent) {
    return new ExitTool(parent);
}

void ExitTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
    emit requestAction(REQ_CLOSE_GUI);
}
