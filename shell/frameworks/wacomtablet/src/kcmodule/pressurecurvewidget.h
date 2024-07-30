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

#ifndef PRESSURECURVEWIDGET_H
#define PRESSURECURVEWIDGET_H

// Qt includes
#include <QColor>
#include <QPoint>
#include <QSize>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;

namespace Wacom
{
/**
 * This widget implements a visual way to adjust the press curve for wacom tablets
 * It shows the beziercurve and two control points to adjust the curve
 * Furthermore if Qt detects the tablet device the pressure is animated
 * as filled area below the curve to give a better feel what the changes to the press curve mean
 * This step is not necessary to change the press curve settings, but a nice visual help.
 * Qt relies on the xorg.conf to detect the tablet correctly because it use hardcoded names for it
 * name them "stylus", "pen", "eraser" to get it working.
 *
 * @see https://doc.qt.io/qt-5/qtabletevent.html
 */
class PressureCurveWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * default constructor
     *
     * @param parent the parent widget that holds this one
     */
    explicit PressureCurveWidget(QWidget *parent = nullptr);

    /**
     * Sets the start values for the bezier presscurve via this control points
     *
     * @bug if this widget has no fixed size, the width()/height() return the
     *      the values as specified in the .ui file or when the widget is created
     *      The automatic layout changes took place before somehow and thus
     *      leads to a wrong placement of the control points
     * @param p1 x value of the first point
     * @param p2 y value of the first point
     * @param p3 x value of the second point
     * @param p4 y value of the second point
     */
    void setControlPoints(qreal p1, qreal p2, qreal p3, qreal p4);

protected:
    /**
     * Called whenever the mouse is pressed in the widget
     * Selects the control point if the mouse click was on it
     *
     * @param event the mouse event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * Drags the control points around
     * based on the active selected control point specified in the mousePressEvent
     * the selected control point is moved around and changes the presscurve directly
     *
     * @param event the mouse event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * Dragging the controlpoint stopped
     * Resets the selection of the active control point to move around
     *
     * @param event the mouse event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * Changes the position of the controlpoints with the same ratio than the widget resize
     * This way the presscurve stays the same all the time
     *
     * @param event the resize event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * If the tablet was detected by Qt this animates the pressure as filled area below the presscurve
     * Qt detects the tablet only if it was configured via the xorg.conf. Otherwise it does not know about it
     * Furthermore the tablet input devices must have the standard names "stylus" and "eraser"
     *
     * @param event the Tablet event
     */
    void tabletEvent(QTabletEvent *event) override;

    /**
     * Draws the background grid, controlpoints and the presscurve on the widget
     * Does it with antialias support
     *
     * @param event the paint event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * Finds the nearest controlpoint the uses clicks on
     * Will be called for mouse and tabletevent and sets m_activePoint
     * with the number of the clicked point.
     *
     * @param pos the position of the mouse/pen click
     */
    void setNearestPoint(const QPointF &pos);

    /**
     * Moves both control points according to the user interaction
     * Both points will be changed even if only one is selected
     * The reason lies deep in the Linux Wacom driver that expect the beziercurve
     * to be this way. So point 2 is always point 1 with mirrored x/y coordinates
     *
     * @param pos the new position
     */
    void moveControlPoint(const QPointF &pos);

signals:
    /**
     * This signal will be fired if the position of the control points change
     * Used to inform the parent widget of this change to be able to display the new values
     * Furthermore the changed presscurve will be set via the xsetwacom driver immediately thus
     * allow the user to see the changes in action right now
     *
     * @param points the presscurve points in a format expected by the xsetwacom driver (like "0 100 0 100")
     */
    void controlPointsChanged(const QString &points);

private:
    QPointF m_cP1; /**< Control point 1 */
    QPointF m_cP2; /**< Control point 2 */
    int m_pointSize = 10; /**< Size of the control point */
    int m_activePoint = 0; /**< The point that is dragged around */
    QColor m_pointColor = Qt::red; /**< Color of the points */
    QColor m_curveColor = Qt::black; /**< Color of the curve */
    qreal m_pressure = 0; /**< Buffers the current stylus pressure. (0.0 - 1.0) used to animate the pressure */
    QColor m_pressAreaColor = Qt::blue; /**< Color of the press indication area */
};

}

#endif // PRESSURECURVEWIDGET_H
