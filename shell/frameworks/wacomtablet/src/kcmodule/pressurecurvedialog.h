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

#ifndef PRESSURECURVEDIALOG_H
#define PRESSURECURVEDIALOG_H

#include "devicetype.h"

#include <QDialog>

namespace Ui
{
class PressureCurveDialog;
}

class QDBusInterface;

namespace Wacom
{
class PressureCurveWidget;

/**
 * This dialog shows the PressureCurveWidget and lets the uses change it on the fly
 * The pressure is animated when the suer uses his pen inside the widget
 */
class PressureCurveDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * default constructor
     *
     * @param initialValue initial value of the pressure
     * @param tabletId tablet identifier
     * @param deviceType device type, @see DeviceType
     * @param parent parent widget
     */
    PressureCurveDialog(const QString &initialValue, const QString &tabletId, const DeviceType &deviceType, QWidget *parent = nullptr);

    /**
     * default destructor
     */
    ~PressureCurveDialog() override;

    /**
     * Returns the changed presscurve control points in the format needed by the xsetwacom settings
     *
     * @return string version of both control points separated by spaces
     */
    QString getControllPoints();

public slots:
    /**
     * Updates the pen presscurve via the xsetwacom settings and shows the values in the dialog
     *
     * @param points string version of both control points separated by spaces
     */
    void updateControlPoints(const QString &points);

    /**
     * called when the user accept his changes
     */
    void accept() override;

    /**
     * called when the user discards his changes
     */
    void reject() override;

private:
    /**
     * Sets the controllpoints to some starting values
     *
     * @param points starting values in the format as expected by the xssetwacom settings
     */
    void setControllPoints(const QString &points);

private:
    Ui::PressureCurveDialog *m_ui = nullptr; /**< Handler to the presscurvedialog.ui file */
    PressureCurveWidget *m_pressCurve = nullptr; /**< Widget that shows the presscurve and lets him change it */

    QString _initialValue;
    QString _tabletId;
    QString _deviceType;
};
}
#endif // PRESSURECURVEDIALOG_H
