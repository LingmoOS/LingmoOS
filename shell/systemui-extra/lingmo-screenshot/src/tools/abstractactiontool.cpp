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

#include "abstractactiontool.h"

AbstractActionTool::AbstractActionTool(QObject *parent) : CaptureTool(parent) {

}

bool AbstractActionTool::isValid() const {
    return true;
}

bool AbstractActionTool::isSelectable() const {
    return false;
}

bool AbstractActionTool::showMousePreview() const {
    return false;
}

QIcon AbstractActionTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    return QIcon();
}
#ifdef SUPPORT_LINGMO
QIcon AbstractActionTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    Q_UNUSED(context);
    return QIcon();
}
#endif
void AbstractActionTool::undo(QPixmap &pixmap) {
    Q_UNUSED(pixmap);
}

void AbstractActionTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    Q_UNUSED(painter);
    Q_UNUSED(pixmap);
    Q_UNUSED(recordUndo);
}

void AbstractActionTool::paintMousePreview(
        QPainter &painter, const CaptureContext &context)
{
    Q_UNUSED(painter);
    Q_UNUSED(context);
}

void AbstractActionTool::drawEnd(const QPoint &p) {
    Q_UNUSED(p);
}

void AbstractActionTool::drawMove(const QPoint &p) {
    Q_UNUSED(p);
}

void AbstractActionTool::drawStart(const CaptureContext &context) {
    Q_UNUSED(context);
}

void AbstractActionTool::colorChanged(const QColor &c) {
    Q_UNUSED(c);
}

void AbstractActionTool::thicknessChanged(const int th) {
    Q_UNUSED(th);
}
void AbstractActionTool::textthicknessChanged(const int th) {
    Q_UNUSED(th);
}

void AbstractActionTool::textChanged(const CaptureContext &context) {
    Q_UNUSED(context);
}
