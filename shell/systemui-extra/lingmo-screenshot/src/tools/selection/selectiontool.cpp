/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*
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
#include "selectiontool.h"
#include <QPainter>

namespace {
#define PADDING_VALUE 2
}

SelectionTool::SelectionTool(QObject *parent) : AbstractTwoPointTool(parent) {
    m_supportsDiagonalAdj = true;
}

bool SelectionTool::closeOnButtonPressed() const {
    return false;
}

QIcon SelectionTool::icon(const QColor &background, bool inEditor) const {
   // Q_UNUSED(inEditor);
   // return QIcon(iconPath(background) + "square-outline.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "square.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "square.svg");

}
#ifdef SUPPORT_LINGMO
QIcon SelectionTool::icon(const QColor &background, bool inEditor, const CaptureContext &context ) const {
    Q_UNUSED(background);
    if((context.style_name.compare("lingmo-dark")==0) || (context.style_name.compare("lingmo-black")==0)){
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "square.svg") :
                           QIcon(QStringLiteral(":/img/material/dark-theme/") + "square.png");
    }
    //if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
    else{
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "square.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "square.svg");
    }
}
#endif
QString SelectionTool::name() const {
    return tr("Rectangular Selection");
}

QString SelectionTool::nameID() {
    return QLatin1String("");
}

QString SelectionTool::description() const {
    return tr("Set the rectangle as the paint tool");
}

CaptureTool* SelectionTool::copy(QObject *parent) {
    return new SelectionTool(parent);
}

void SelectionTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    //绘制矩形  以框选区域为边界
    if (rect.contains(m_points.first))
    {    painter.drawLine(QLine(QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness)),
			        QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
				       qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness))));

         painter.drawLine(QLine(QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness)),
                                QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness))));

	 painter.drawLine(QLine(QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness)),
                                QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness))));

	 painter.drawLine(QLine(QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness)),
                                QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
                                       qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness))));
    }
}

void SelectionTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void SelectionTool::drawStart(const CaptureContext &context) {
    pixelRatio = context.origScreenshot.devicePixelRatio();
    rect = context.selection;
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void SelectionTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
