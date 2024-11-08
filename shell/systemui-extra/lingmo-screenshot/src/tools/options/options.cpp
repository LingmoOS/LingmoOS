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
#include "options.h"

Options::Options(QObject *parent) : AbstractActionToolWithWidget(parent)
{
}
bool Options::closeOnButtonPressed() const
{
     return false;
}

QIcon Options::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
    return QIcon();
}
#ifdef SUPPORT_LINGMO
QIcon Options::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
    return QIcon();
}
#endif
QString Options::name() const
{
    return tr("Options");
}
QString Options::nameID()
{
     return QLatin1String("");
}
QString Options::description() const
{
    return tr("options tool");
    //return tr("图片保存配置");
}

CaptureTool* Options::copy(QObject *parent)
{
     return new Options(parent);
}

void Options::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_OPTIONS);
}
