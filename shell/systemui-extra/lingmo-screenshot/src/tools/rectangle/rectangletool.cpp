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

#include "rectangletool.h"
#include <QPainter>
#include <QDebug>
namespace {
#define PADDING_VALUE 2
}

RectangleTool::RectangleTool(QObject *parent) : AbstractTwoPointTool(parent) {
    m_supportsDiagonalAdj = true;
}

QIcon RectangleTool::icon(const QColor &background, bool inEditor) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "square.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "square.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "square.svg");
}
#ifdef SUPPORT_LINGMO
QIcon RectangleTool::icon(const QColor &background, bool inEditor , const CaptureContext &context) const {
    Q_UNUSED(background);
    if((context.style_name.compare("lingmo-dark")==0) || (context.style_name.compare("lingmo-black")==0)){
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "square.svg") :
                           QIcon(QStringLiteral(":/img/material/dark-theme/") + "square.png");
    }
    //if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0)|| (context.style_name.compare("lingmo-light")==0)){
    else{
        return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "square.svg") :
                      QIcon(QStringLiteral(":/img/material/dark-theme/") + "square.png");
    }
}
#endif
QString RectangleTool::name() const {
    return tr("Rectangle");
}

QString RectangleTool::nameID() {
    return QLatin1String("");
}

QString RectangleTool::description() const {
    return tr("Set the Rectangle as the paint tool");
}

CaptureTool* RectangleTool::copy(QObject *parent) {
    return new RectangleTool(parent);
}

void RectangleTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.setBrush(QBrush(m_color));
    painter.drawRect(QRect(m_points.first, m_points.second));
}

void RectangleTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    rect = context.selection;
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void RectangleTool::drawStart(const CaptureContext &context) {
    rect = context.selection;
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void RectangleTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
