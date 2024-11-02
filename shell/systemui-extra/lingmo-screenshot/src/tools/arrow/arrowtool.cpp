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

#include <QPainterPath>
#include "arrowtool.h"

namespace {

#define PADDING_VALUE 2

const int ArrowWidth = 10;
const int ArrowHeight = 18;

QPainterPath getArrowHead(QPoint p1, QPoint p2, const int thickness) {
    QLineF base(p1, p2);
    // Create the vector for the position of the base  of the arrowhead
    QLineF temp(QPoint(0,0), p2-p1);
    int val = ArrowHeight + thickness*4;
    if (base.length() < val) {
        val = (base.length() + thickness*2);
    }
    temp.setLength(base.length() + thickness*2 - val);
    // Move across the line up to the head
    QPointF bottonTranslation(temp.p2());

    // Rotate base of the arrowhead
    base.setLength(ArrowWidth + thickness*2);
    base.setAngle(base.angle() + 90);
    // Move to the correct point
    QPointF temp2 = p1 - base.p2();
    // Center it
    QPointF centerTranslation((temp2.x()/2), (temp2.y()/2));

    base.translate(bottonTranslation);
    base.translate(centerTranslation);

    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(base.p1());
    path.lineTo(base.p2());
    path.lineTo(p2);
    return path;
}

// gets a shorter line to prevent overlap in the point of the arrow
QLine getShorterLine(QPoint p1, QPoint p2, const int thickness) {
    QLineF l(p1, p2);
    int val = ArrowHeight + thickness*4;
    if (l.length() < val) {
        val = (l.length() + thickness*2);
    }
    l.setLength(l.length() + thickness*2 - val);
    return l.toLine();
}

} // unnamed namespace

ArrowTool::ArrowTool(QObject *parent) : AbstractTwoPointTool(parent) {
    m_padding = ArrowWidth / 2;
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon ArrowTool::icon(const QColor &background, bool inEditor) const {
    //Q_UNUSED(inEditor);
   // return QIcon(iconPath(background) + "arrow-bottom-left.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "arrow-bottom-left.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "arrow-bottom-left.svg");

}
#ifdef SUPPORT_LINGMO
QIcon ArrowTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const {
    //Q_UNUSED(inEditor);
   // return QIcon(iconPath(background) + "arrow-bottom-left.svg");
    Q_UNUSED(background);
    if((context.style_name.compare("lingmo-dark")==0) || (context.style_name.compare("lingmo-black")==0)){
         return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "arrow-bottom-left.svg") :
                  QIcon(QStringLiteral(":/img/material/dark-theme/") + "arrow-bottom-left.png");
     }
    //if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
    else{
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "arrow-bottom-left.svg") :
                     QIcon(QStringLiteral(":/img/material/white/") + "arrow-bottom-left.svg");
    }
}
#endif
QString ArrowTool::name() const {
    return tr("Arrow");
}

QString ArrowTool::nameID() {
    return QLatin1String("");
}

QString ArrowTool::description() const {
    return tr("Set the Arrow as the paint tool");
}

CaptureTool* ArrowTool::copy(QObject *parent) {
    return new ArrowTool(parent);
}

void ArrowTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    // 绘制箭头工具 限定区域为框选区域
    if (rect.contains(m_points.first))
    {
             painter.drawLine(getShorterLine(
			      QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                     qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness)),
                              QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
                                     qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness)),
			      m_thickness));
	     painter.fillPath(getArrowHead(QPoint(qBound(rect.x()+m_thickness, m_points.first.x(), rect.x()+ rect.width()-m_thickness),
                                     qBound(rect.y()+m_thickness, m_points.first.y(), rect.y()+rect.height()-m_thickness)),
                              QPoint(qBound(rect.x()+m_thickness, m_points.second.x(), rect.x()+ rect.width()-m_thickness),
                                     qBound(rect.y()+m_thickness, m_points.second.y(), rect.y()+rect.height()-m_thickness))
			      , m_thickness), QBrush(m_color));
    }

}

void ArrowTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void ArrowTool::drawStart(const CaptureContext &context) {
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void ArrowTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
