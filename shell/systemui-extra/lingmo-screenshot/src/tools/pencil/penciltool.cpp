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

#include "penciltool.h"
#include <QPainter>

PencilTool::PencilTool(QObject *parent) : AbstractPathTool(parent) {

}

QIcon PencilTool::icon(const QColor &background, bool inEditor) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "pencil.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "pencil.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "pencil.svg");
}
#ifdef SUPPORT_LINGMO
QIcon PencilTool::icon(const QColor &background, bool inEditor , const CaptureContext &context) const {
    Q_UNUSED(background);
    if((context.style_name.compare("lingmo-dark")==0) || (context.style_name.compare("lingmo-black")==0)){
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "pencil.svg") :
                          QIcon(QStringLiteral(":/img/material/dark-theme/") + "pencil.png");
    }
    //if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
    else{
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "pencil.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "pencil.svg");
    }
}
#endif
QString PencilTool::name() const {
    return tr("Pencil");
}

QString PencilTool::nameID() {
    return QLatin1String("");
}

QString PencilTool::description() const {
    return tr("Set the Pencil as the paint tool");
}

CaptureTool* PencilTool::copy(QObject *parent) {
    return new PencilTool(parent);
}

void PencilTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    //int date = m_points.data();
    // 绘制画笔工具 限定区域为框选区域
    for (int i = 0 ; i < m_points.size(); i++)
    {
	 if (!rect.contains(m_points.at(i)))
	 {
	     m_points.remove(i);
	 }
    }
    painter.drawPolyline(m_points.data(), m_points.size());
}

void PencilTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, context.thickness + 2));
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    painter.drawLine(context.mousePos, context.mousePos);
}

void PencilTool::drawStart(const CaptureContext &context) {
    rect = context.selection;
    pixelRatio = context.origScreenshot.devicePixelRatio();
    m_color = context.color;
    m_thickness = context.thickness + 2;
    m_points.append(context.mousePos);
    m_backupArea.setTopLeft(context.mousePos);
    m_backupArea.setBottomRight(context.mousePos);
}

void PencilTool::pressed(const CaptureContext &context) {
    rect = context.selection;
     //Q_UNUSED(context);
}
