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

#include "linetool.h"
#include <QPainter>

namespace {

#define PADDING_VALUE 2

}

LineTool::LineTool(QObject *parent) : AbstractTwoPointTool(parent) {
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon LineTool::icon(const QColor &background, bool inEditor) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "line.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "line.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "line.svg");
}
#ifdef SUPPORT_LINGMO
QIcon LineTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const {
    Q_UNUSED(background);
    if((context.style_name.compare("lingmo-dark")==0) || (context.style_name.compare("lingmo-black")==0)){
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "line.svg") :
                          QIcon(QStringLiteral(":/img/material/dark-theme/") + "line.png");
    }
    //if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
    else{
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "line.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "line.svg");
    }
}
#endif
QString LineTool::name() const {
    return tr("Line");
}

QString LineTool::nameID() {
    return QLatin1String("");
}

QString LineTool::description() const {
    return tr("Set the Line as the paint tool");
}

CaptureTool* LineTool::copy(QObject *parent) {
    return new LineTool(parent);
}

void LineTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    //绘制直线  以框选区域为边界
    if (rect.contains(m_points.first))
    {
        painter.drawLine(QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness)),
                         QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
                                qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness)));
    }

}

void LineTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void LineTool::drawStart(const CaptureContext &context) {
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void LineTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
