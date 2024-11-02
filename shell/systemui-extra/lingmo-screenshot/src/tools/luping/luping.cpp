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
#include "luping.h"

LuPing::LuPing(QObject *parent) : AbstractActionTool(parent)
{

}
bool LuPing::closeOnButtonPressed() const
{
     return false;
}

QIcon LuPing:: icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "luping.svg");
}
#ifdef SUPPORT_LINGMO
QIcon LuPing:: icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "luping.svg");
}
#endif
QString LuPing::name() const
{
    return tr("LuPing");
}
 QString LuPing:: nameID()
{
     return QLatin1String("");
}
QString LuPing::description() const
{
    return tr("luping");
}
CaptureTool* LuPing::copy(QObject *parent)
{
     return new LuPing(parent);
}


void LuPing::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_LUPING);
}
