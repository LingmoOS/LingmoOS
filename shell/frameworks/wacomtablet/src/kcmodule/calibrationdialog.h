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

#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>

namespace Wacom
{

/**
 * @brief
 *
 */
class CalibrationDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the fullscreen window for the calibration process
     *
     * @param toolname name of the tool to calibrate (stylus or touch name)
     * @param targetScreen screen which is going to be used for calibration
     */
    CalibrationDialog(const QString &toolname, const QString &targetScreen);

    /**
     * @brief Returns the new tablet area
     *
     * @return Tablet area after calibration
     */
    QRect calibratedArea();

protected:
    /**
     * @brief Draws the cross for the calibration
     *
     * The user has to hit the center of the cross for the calibration
     *
     * @param event some additional event details
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Catch mousepress events
     *
     * Only events inside the calibration cross are recognized. All other are ignored.
     *
     * @param event mouse press details
     */
    void mousePressEvent(QMouseEvent *event) override;

private:
    /**
     * @brief calculates the new tablet area from all 4 calibration points
     */
    void calculateNewArea();

    int m_drawCross; /**< Which cross should be drawn? 0=topleft, 1=bottomleft, and so on */
    int m_shiftLeft; /**< Where to start the cross from the left */
    int m_shiftTop; /**< Where to start the cross from the top */

    QString m_toolName; /**< Name of the tool to calibrate (stylus name or touch name) */
    QRectF m_originaltabletArea; /**< Original tablet area before calibration */
    QRectF m_newtabletArea; /**< Calibrated tablet area */
    QPointF m_topLeft; /**< Top left clicked point for calibration */
    QPointF m_bottomLeft; /**< Bottom left clicked point for calibration  */
    QPointF m_topRight; /**< Top right clicked point for calibration  */
    QPointF m_bottomRight; /**< Bottom right clicked point for calibration  */
};
}
#endif // CALIBRATIONDIALOG_H
