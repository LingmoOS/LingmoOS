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

#include "pressurecurvedialog.h"
#include "ui_pressurecurvedialog.h"

#include "dbustabletinterface.h"
#include "logging.h"
#include "pressurecurvewidget.h"

// Qt includes
#include <QDBusInterface>
#include <QDBusReply>

using namespace Wacom;

PressureCurveDialog::PressureCurveDialog(const QString &initialValue, const QString &tabletId, const DeviceType &deviceType, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::PressureCurveDialog)
    , _initialValue(initialValue)
    , _tabletId(tabletId)
    , _deviceType(deviceType.key())
{
    m_ui->setupUi(this);

    connect(m_ui->pc_Widget, SIGNAL(controlPointsChanged(QString)), SLOT(updateControlPoints(QString)));

    setControllPoints(initialValue);
}

PressureCurveDialog::~PressureCurveDialog()
{
    delete m_ui;
}

void PressureCurveDialog::setControllPoints(const QString &points)
{
    QStringList splitPoints = points.split(QLatin1Char(' '));

    if (splitPoints.count() != 4) {
        qCDebug(KCM) << "Invalid number of control points, using defaults";
        splitPoints.insert(0, QLatin1String("0"));
        splitPoints.insert(1, QLatin1String("0"));
        splitPoints.insert(2, QLatin1String("100"));
        splitPoints.insert(3, QLatin1String("100"));
    }

    qreal p1 = splitPoints.at(0).toDouble();
    qreal p2 = splitPoints.at(1).toDouble();
    qreal p3 = splitPoints.at(2).toDouble();
    qreal p4 = splitPoints.at(3).toDouble();
    m_ui->pc_Widget->setControlPoints(p1, p2, p3, p4);
    m_ui->pc_Values->setText(QString::fromLatin1("%1 %2 %3 %4").arg(p1).arg(p2).arg(p3).arg(p4));
}

QString PressureCurveDialog::getControllPoints()
{
    return m_ui->pc_Values->text();
}

void PressureCurveDialog::updateControlPoints(const QString &points)
{
    m_ui->pc_Values->setText(points);
    DBusTabletInterface::instance().setProperty(_tabletId, _deviceType, Property::PressureCurve.key(), points);
}

void PressureCurveDialog::accept()
{
    done(QDialog::Accepted);
}
void PressureCurveDialog::reject()
{
    // Reset pressure curve to initial values
    DBusTabletInterface::instance().setProperty(_tabletId, _deviceType, Property::PressureCurve.key(), _initialValue);

    done(QDialog::Rejected);
}

#include "moc_pressurecurvedialog.cpp"
