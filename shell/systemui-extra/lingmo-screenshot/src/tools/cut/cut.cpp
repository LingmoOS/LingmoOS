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
#include "cut.h"

Cut::Cut(QObject *parent) : AbstractActionTool(parent) {


}
bool Cut::closeOnButtonPressed() const
{
     return false;
}

QIcon Cut::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "cut.svg") :
                      QIcon(QStringLiteral(":/img/material/black/") + "cut.svg");
}
#ifdef SUPPORT_LINGMO
QIcon Cut::icon(const QColor &background, bool inEditor , const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "cut.svg") :
                      QIcon(QStringLiteral(":/img/material/black/") + "cut.svg");
}
#endif
QString Cut::name() const
{
    return tr("Cut");
}
QString  Cut::nameID()
{
     return QLatin1String("");
}
QString Cut::description() const
{
    return tr("cut");
}

CaptureTool* Cut::copy(QObject *parent)
{
     return new Cut(parent);
}


void Cut::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_CUT);
}
