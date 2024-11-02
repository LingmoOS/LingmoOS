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

#include "circletool.h"
#include <QPainter>
namespace {
#define PADDING_VALUE 2
}

CircleTool::CircleTool(QObject *parent) : AbstractTwoPointTool(parent)
{
    m_supportsDiagonalAdj = true;
}

QIcon CircleTool::icon(const QColor &background, bool inEditor) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "circle-outline.svg");
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "circle-outline.svg")
           : QIcon(QStringLiteral(":/img/material/white/") + "circle-outline.svg");
}

#ifdef SUPPORT_LINGMO
QIcon CircleTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    if ((context.style_name.compare("lingmo-dark") == 0)
        || (context.style_name.compare("lingmo-black") == 0)) {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "circle-outline.svg")
               : QIcon(QStringLiteral(":/img/material/dark-theme/") + "circle-outline.png");
    }
    // if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
    else {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "circle-outline.svg")
               : QIcon(QStringLiteral(":/img/material/white/") + "circle-outline.svg");
    }
}

#endif
QString CircleTool::name() const
{
    return tr("Circle");
}

QString CircleTool::nameID()
{
    return QLatin1String("");
}

QString CircleTool::description() const
{
    return tr("Set the Circle as the paint tool");
}

CaptureTool *CircleTool::copy(QObject *parent)
{
    return new CircleTool(parent);
}

void CircleTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo)
{
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    // 绘制椭圆  以框选区域为边界
    if (rect.contains(m_points.first)) {
        painter.drawEllipse(QRect(QPoint(qBound(rect.x()+m_thickness, m_points.first.x(),
                                                rect.x()+ rect.width()-m_thickness),
                                         qBound(rect.y()+m_thickness, m_points.first.y(),
                                                rect.y()+rect.height()-m_thickness)),
                                  QPoint(qBound(rect.x()+m_thickness, m_points.second.x(),
                                                rect.x()+ rect.width()-m_thickness),
                                         qBound(rect.y()+m_thickness, m_points.second.y(),
                                                rect.y()+rect.height()-m_thickness))));
    }
}

void CircleTool::paintMousePreview(QPainter &painter, const CaptureContext &context)
{
    pixelRatio = context.origScreenshot.devicePixelRatio();
    rect = context.selection;
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void CircleTool::drawStart(const CaptureContext &context)
{
    pixelRatio = context.origScreenshot.devicePixelRatio();
    rect = context.selection;
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void CircleTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
}
