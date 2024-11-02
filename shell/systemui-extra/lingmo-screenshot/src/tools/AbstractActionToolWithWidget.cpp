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
#include "AbstractActionToolWithWidget.h"

AbstractActionToolWithWidget::AbstractActionToolWithWidget(QObject *parent) : CaptureTool(parent) {

}

bool AbstractActionToolWithWidget::isValid() const {
    return true;
}

bool AbstractActionToolWithWidget::isSelectable() const {
    return true;
}

bool AbstractActionToolWithWidget::showMousePreview() const {
    return false;
}

QIcon AbstractActionToolWithWidget::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    return QIcon();
}
#ifdef SUPPORT_LINGMO
QIcon AbstractActionToolWithWidget::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    Q_UNUSED(context);
    return QIcon();
}
#endif
void AbstractActionToolWithWidget::undo(QPixmap &pixmap) {
    Q_UNUSED(pixmap);
}

void AbstractActionToolWithWidget::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    Q_UNUSED(painter);
    Q_UNUSED(pixmap);
    Q_UNUSED(recordUndo);
}

void AbstractActionToolWithWidget::paintMousePreview(
        QPainter &painter, const CaptureContext &context)
{
    Q_UNUSED(painter);
    Q_UNUSED(context);
}

void AbstractActionToolWithWidget::drawEnd(const QPoint &p) {
    Q_UNUSED(p);
}

void AbstractActionToolWithWidget::drawMove(const QPoint &p) {
    Q_UNUSED(p);
}

void AbstractActionToolWithWidget::drawStart(const CaptureContext &context) {
    Q_UNUSED(context);
}

void AbstractActionToolWithWidget::colorChanged(const QColor &c) {
    Q_UNUSED(c);
}

void AbstractActionToolWithWidget::thicknessChanged(const int th) {
    Q_UNUSED(th);
}
void AbstractActionToolWithWidget::textthicknessChanged(const int th) {
    Q_UNUSED(th);
}

void AbstractActionToolWithWidget::textChanged(const CaptureContext &context) {
    Q_UNUSED(context);
}
