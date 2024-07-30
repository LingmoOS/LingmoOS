/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AREASELECTIONWIDGET_H
#define AREASELECTIONWIDGET_H

#include <QFont>
#include <QRectF>
#include <QSize>
#include <QStringList>
#include <QWidget>

class QBrush;
class QPainter;

namespace Wacom
{

class AreaSelectionWidgetPrivate;

class AreaSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    bool paintBelow{false};

    explicit AreaSelectionWidget(QWidget *parent = nullptr);

    ~AreaSelectionWidget() override;

    /**
     * Resets the selection back to the full size.
     */
    void clearSelection();

    /**
     * Gets the currently selected area.
     *
     * @return The currently selected area.
     */
    const QRect getSelection() const;

    /**
     * Returns the current selection as string.
     * Format is: "x y width height"
     *
     * @return The current selection as string.
     */
    const QString getSelectionAsString() const;

    /**
     * @return The virtual area rectangle which is the union of all areas currently set.
     */
    const QRect &getVirtualArea() const;

    /**
     * Sets the area which should be scaled and displayed to the user.
     *
     * @param area The area to scale and present to the user.
     * @param caption An optional caption which is drawn in the center of the area.
     */
    void setArea(const QRect &area, const QString &caption);

    /**
     * Sets the areas which should be scaled and displayed to the user.
     * All areas will be united and displayed as one big area to the user.
     * Therefore the areas should be adjacent or overlapping.
     *
     * @param areas   The areas to display to the user.
     * @param areaCaptions Optional captions to draw in the center of each area.
     */
    void setAreas(const QMap<QString, QRect> &areas, const QStringList &areaCaptions);

    /**
     * Enables or disables drawing of area captions.
     *
     * @param value True to draw area captions, false to not draw them.
     */
    void setDrawAreaCaptions(bool value);

    /**
     * Enables or disables drawing of the selection area caption.
     *
     * @param value True to draw caption, false to not draw it.
     */
    void setDrawSelectionCaption(bool value);

    /**
     * Sets the font used to draw all captions.
     *
     * @param font The font used for captions.
     */
    void setFont(const QFont &font);

    /**
     * Sets the out of bounds margin in pixels or as a percentage.
     * The out of bounds margin is the amount of pixels, the user is
     * allowed to drag the selected area outside the bounds of the
     * displayed area. The margin has to be given as a percentage value
     * or as a positive value in real size pixels.
     *
     * @param margin A percentage (0. - 1.0) or a positive value in pixels.
     */
    void setOutOfBoundsMargin(qreal margin);

    /**
     * Selects an area.
     *
     * @param selection The area to select.
     * @param emitUpdate Whether to emit the update signal.
     */
    void setSelection(const QRect &selection, bool emitUpdate);

    /**
     * Selects an area by area index. This is the index from the list
     * the widget was initialized with.
     *
     * @param output The list index of the sub-area to select.
     */
    void setSelection(QString output);

    /**
     * Sets the widget's target size. This is only a hint for the widget
     * to determine the approximate size it should target for when scaling the
     * areas. As long as the ratio does not fit to the ratio of the displayed
     * area, only one value will be used. The other value will be scaled up
     * or down to fit the ratio of the displayed area.
     *
     * @param size The widget's target size (Default: 400x400)
     */
    void setWidgetTargetSize(const QSize &size);

    void lockProportions(bool enable);

signals:

    /**
     * Emitted whenever the selected area changes.
     */
    void selectionChanged();

protected:
    /**
     * Overridden QWidget method which captures mouse movements.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * Overridden QWidget method which captures mouse button press events.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * Overridden QWidget method which captures mouse button release events.
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * Overridden QWidget method which paints the widget.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * The current dragging mode which determines if the
     * user is dragging one of the border handles or the
     * whole area with the mouse.
     */
    enum class DragMode {
        DragNone, //!< The user is currently not dragging.
        DragSelectedArea, //!< The user drags the whole selection area.
        DragTopHandle, //!< The user drags the top border handle.
        DragRightHandle, //!< The user drags the right border handle.
        DragBottomHandle, //!< The user drags the bottom border handle.
        DragLeftHandle //!< The user drags the left border handle.
    };

    /**
     * Calculates the size and position of the area which is displayed
     * to the user according to the given virtual area, a scale factor
     * and an optional margin.
     *
     * @param virtualArea The virtual area to calculate the display area for.
     * @param scaleFactor The scale factor which is applied to the virtual area.
     * @param totalDisplayAreaMargin The total display area margin.
     */
    const QRectF calculateDisplayArea(const QRect &virtualArea, qreal scaleFactor, qreal totalDisplayAreaMargin) const;

    /**
     * Calculates the size and position of the display sub-areas which
     * make up the whole display area.
     *
     * @param areas       The display sub-areas.
     * @param scaleFactor The scale factor which is applied to the areas.
     * @param totalDisplayAreaMargin The total display area margin.
     */
    const QList<QRectF> calculateDisplayAreas(const QMap<QString, QRect> areas, qreal scaleFactor, qreal totalDisplayAreaMargin) const;

    /**
     * Calculates the out of bounds margin for the virtual area.
     *
     * @param virtualArea      The virtual area to calculate the oob margin for.
     * @param outOfBoundMargin A percentage (0-1.0) or a value in pixels (>1).
     *
     * @return The out ouf bounds margin in virtual area pixels.
     */
    qreal calculateOutOfBoundsVirtualAreaMargin(const QRect &virtualArea, qreal outOfBoundsMargin) const;

    /**
     * Calculates a scaling factor based on a widget target size and the
     * given virtual area.
     *
     * @param targetSize  The target size of this widget.
     * @param virtualArea The virtual area to calculate the scaling factor for.
     * @param virtualAreaOutOfBoundsMargin The out of bounds margin of the virtual area.
     * @param displayAreaExtraMargin       An extra margin for the display area.
     *
     * @return The scaling factor for the given target size.
     */
    qreal calculateScaleFactor(const QSize &targetSize, const QRect &virtualArea, qreal virtualAreaOutOfBoundsMargin, qreal displayAreaExtraMargin) const;

    /**
     * Calculates a scaled version of the given area.
     *
     * @param area        The area to scale.
     * @param scaleFactor The scale factor which is applied to the area.
     * @param margin      An optional margin around the display area.
     *
     * @return The scaled area.
     */
    const QRectF calculateScaledArea(const QRect &area, qreal scaleFactor, qreal totalDisplayAreaMargin) const;

    /**
     * Unscales a scaled area.
     *
     * @param area        The area to unscale.
     * @param scaleFactor The scale factor which was applied to the area.
     * @param margin      An optional margin around the display area.
     *
     * @return The unscaled area.
     */
    const QRect calculateUnscaledArea(const QRectF &area, qreal scaleFactor, qreal totalDisplayAreaMargin) const;

    /**
     * Calculates the size of the virtual area.
     *
     * @param areas A list of areas to calculate the virtual area for.
     *
     * @return The size of the virtual area as rectangle.
     */
    const QRect calculateVirtualArea(const QMap<QString, QRect> &areas) const;

    /**
     * Returns the total margin of the display area. This is the out of bounds
     * margin plus any additional margin which might be applied around the widget.
     *
     * @return The total display area margin.
     */
    qreal getTotalDisplayAreaMargin() const;

    /**
     * Determines if the user is currently dragging a handle or
     * the selected area with the mouse.
     *
     * @return True if user is dragging, else false.
     */
    bool isUserDragging() const;

    /**
     * Paints the captions of the display areas.
     *
     * @param painter The painter to use.
     */
    void paintDisplayAreaCaptions(QPainter &painter);

    /**
     * Paints the display area and all of its sub-areas.
     *
     * @param painter     The painter to use.
     * @param outlineOnly A flag if only the outline should be painted.
     */
    void paintDisplayAreas(QPainter &painter, bool outlineOnly);

    /**
     * Paints the drag handles.
     *
     * @param painter The painter to use.
     */
    void paintDragHandles(QPainter &painter);

    /**
     * Paints the selected area.
     *
     * @param painter The painter to use.
     */
    void paintSelectedArea(QPainter &painter, bool outlineOnly);

    /**
     * Paints the size of the selected area as string.
     *
     * @param painter The painter to sue.
     */
    void paintSelectedAreaCaption(QPainter &painter);

    /**
     * Sets up the widget. Can be called anytime to recalculate the display area size.
     */
    void setupWidget();

    /**
     * Recalculates the positions of the drag handles.
     */
    void updateDragHandles();

    /**
     * Updates the mouse cursor according to the mouse's position.
     *
     * @param mousePosition The current position of the mouse.
     */
    void updateMouseCursor(const QPoint &mousePosition);

    /**
     * Updates the selected area when the user drags it.
     */
    void updateSelectedAreaOnDrag(const QPoint &mousePosition);

    /**
     * Updates the selected area when the user is moving it.
     *
     * @param mousePosition The current mouse position where the user wants to move the selection to.
     */
    void updateSelectedAreaOnDragArea(const QPoint &mousePosition);

    /**
     * Updates the selected area when the user is dragging with the bottom handle.
     *
     * @param mousePosition The current mouse position where the user wants to have the new border set.
     */
    void updateSelectedAreaOnDragBottom(const QPoint &mousePosition);

    /**
     * Updates the selected area when the user is dragging with the left handle.
     *
     * @param mousePosition The current mouse position where the user wants to have the new border set.
     */
    void updateSelectedAreaOnDragLeft(const QPoint &mousePosition);

    /**
     * Updates the selected area when the user is dragging with the right handle.
     *
     * @param mousePosition The current mouse position where the user wants to have the new border set.
     */
    void updateSelectedAreaOnDragRight(const QPoint &mousePosition);

    /**
     * Updates the selected area when the user is dragging with the top handle.
     *
     * @param mousePosition The current mouse position where the user wants to have the new border set.
     */
    void updateSelectedAreaOnDragTop(const QPoint &mousePosition);

    /**
     * Asserts that the selected area is not wider or higher than the display area.
     * Updates the selected area accordingly.
     *
     * @param fixPositionInsteadOfSize Fix x and y value instead of the width and height of the selection.
     */
    void updateSelectedAreaSize(bool fixPositionInsteadOfSize = false);

    Q_DECLARE_PRIVATE(AreaSelectionWidget)
    AreaSelectionWidgetPrivate *const d_ptr; //!< D-Pointer for this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
