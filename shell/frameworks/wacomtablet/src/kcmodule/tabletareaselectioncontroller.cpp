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

#include "tabletareaselectioncontroller.h"

#include "calibrationdialog.h"
#include "logging.h"
#include "tabletareaselectionview.h"

#include "screensinfo.h"
#include "stringutils.h"
#include "x11wacom.h"

using namespace Wacom;

namespace Wacom
{
class TabletAreaSelectionControllerPrivate
{
public:
    TabletAreaSelectionView *view = nullptr;
    TabletArea tabletGeometry; // the original tablet geometry
    TabletArea tabletGeometryRotated; // the rotated tablet geometry if rotation is active
    QMap<QString, QRect> screenGeometries; // the geometries of all screens which form the desktop
    ScreenSpace currentScreen;
    QString deviceName; // the device this instance is handling
    ScreenMap screenMap; // the current screen mappings
    ScreenRotation tabletRotation = ScreenRotation::NONE; // the tablet rotation
};
}

TabletAreaSelectionController::TabletAreaSelectionController()
    : QObject()
    , d_ptr(new TabletAreaSelectionControllerPrivate)
{
    // nothing to do except for initializing our private class
}

TabletAreaSelectionController::~TabletAreaSelectionController()
{
    delete this->d_ptr;
}

const ScreenMap &TabletAreaSelectionController::getScreenMap()
{
    Q_D(const TabletAreaSelectionController);

    // make sure the current mapping is included
    setMapping(d->currentScreen, d->view->getSelection());

    // return mapping
    return d->screenMap;
}

const ScreenSpace TabletAreaSelectionController::getScreenSpace() const
{
    Q_D(const TabletAreaSelectionController);

    return d->currentScreen;
}

void TabletAreaSelectionController::select(const ScreenSpace &screenSpace)
{
    Q_D(TabletAreaSelectionController);

    if (!hasView()) {
        return;
    }

    setMapping(d->currentScreen, d->view->getSelection());

    d->currentScreen = screenSpace;
    d->view->select(screenSpace.toString(), screenSpace.isDesktop(), getMapping(d->currentScreen));
}

void TabletAreaSelectionController::setView(TabletAreaSelectionView *view)
{
    Q_D(TabletAreaSelectionController);

    // cleanup signal/slot connections if we already have a view
    if (d->view) {
        disconnect(d->view, SIGNAL(signalCalibrateClicked()), this, SLOT(onCalibrateClicked()));
        disconnect(d->view, SIGNAL(signalFullTabletSelection()), this, SLOT(onFullTabletSelected()));
        disconnect(d->view, SIGNAL(signalScreenToggle()), this, SLOT(onScreenToggle()));
        disconnect(d->view, SIGNAL(signalSetScreenProportions()), this, SLOT(onSetScreenProportions()));
        disconnect(d->view, SIGNAL(signalTabletAreaSelection()), this, SLOT(onTabletAreaSelected()));
    }

    // save view and connect signals
    d->view = view;

    if (view) {
        connect(view, SIGNAL(signalCalibrateClicked()), this, SLOT(onCalibrateClicked()));
        connect(view, SIGNAL(signalFullTabletSelection()), this, SLOT(onFullTabletSelected()));
        connect(view, SIGNAL(signalScreenToggle()), this, SLOT(onScreenToggle()));
        connect(view, SIGNAL(signalSetScreenProportions()), this, SLOT(onSetScreenProportions()));
        connect(view, SIGNAL(signalTabletAreaSelection()), this, SLOT(onTabletAreaSelected()));
    }
}

void TabletAreaSelectionController::setupController(const ScreenMap &mappings, const QString &deviceName, const ScreenRotation &rotation)
{
    Q_D(TabletAreaSelectionController);

    if (!hasView()) {
        return;
    }

    d->deviceName = deviceName;
    d->tabletGeometry = X11Wacom::getMaximumTabletArea(deviceName);
    d->screenGeometries = ScreensInfo::getScreenGeometries();
    d->screenMap = mappings;

    if (rotation == ScreenRotation::AUTO) {
        // TODO
        d->tabletRotation = ScreensInfo::getScreenRotation();
        // we have a tablet (not a canvas) viewpoint here, so we need to invert the screen rotation
        d->tabletRotation = d->tabletRotation.invert();

    } else if (rotation == ScreenRotation::AUTO_INVERTED) {
        // TODO
        d->tabletRotation = ScreensInfo::getScreenRotation();

    } else {
        d->tabletRotation = rotation;
    }

    d->tabletGeometryRotated = d->tabletGeometry;

    if (d->tabletRotation == ScreenRotation::CW || d->tabletRotation == ScreenRotation::CCW) {
        d->tabletGeometryRotated.setWidth(d->tabletGeometry.height());
        d->tabletGeometryRotated.setHeight(d->tabletGeometry.width());
    }

    qCDebug(KCM) << "Calling setupScreens and setupTablet from setupController.  ScreenGeometries: " << d->screenGeometries;
    d->view->setupScreens(d->screenGeometries, QSize(200, 200));
    d->view->setupTablet(d->tabletGeometryRotated, QSize(400, 400));

    // make sure we have valid data set
    d->view->select(d->currentScreen.toString(), d->currentScreen.isDesktop(), getMapping(d->currentScreen));
}

void TabletAreaSelectionController::onCalibrateClicked()
{
    Q_D(TabletAreaSelectionController);

    QScopedPointer<CalibrationDialog> calibDialog(new CalibrationDialog(d->deviceName, d->currentScreen.toString()));
    calibDialog->exec();

    setSelection(TabletArea(calibDialog->calibratedArea()));
}

void TabletAreaSelectionController::onFullTabletSelected()
{
    checkConfigurationForTrackingModeProblems();
}

void TabletAreaSelectionController::onScreenToggle()
{
    Q_D(TabletAreaSelectionController);

    select(d->currentScreen.next());
}

void TabletAreaSelectionController::onSetScreenProportions()
{
    Q_D(TabletAreaSelectionController);

    QRect tabletGeometry = d->tabletGeometryRotated;
    QRect screenSelection = getScreenGeometry(d->currentScreen.toString());

    if (screenSelection.isEmpty()) {
        return;
    }

    // calculate new height and width of the selection
    qreal screenAreaSelectionRatio = (float)screenSelection.width() / screenSelection.height();
    qreal newWidth, newHeight;

    if (screenSelection.width() > screenSelection.height()) {
        newWidth = tabletGeometry.width();
        newHeight = newWidth / screenAreaSelectionRatio;

        if (newHeight > tabletGeometry.height()) {
            newHeight = tabletGeometry.height();
            newWidth = newHeight * screenAreaSelectionRatio;
        }

    } else {
        newHeight = tabletGeometry.height();
        newWidth = newHeight * screenAreaSelectionRatio;

        if (newWidth > tabletGeometry.width()) {
            newWidth = tabletGeometry.width();
            newHeight = newWidth / screenAreaSelectionRatio;
        }
    }

    // calculate x and y to center the selection
    int newX = tabletGeometry.x() + (int)((tabletGeometry.width() - newWidth) / 2.);
    int newY = tabletGeometry.y() + (int)((tabletGeometry.height() - newHeight) / 2.);

    setSelection(TabletArea(QRect(newX, newY, qRound(newWidth), qRound(newHeight))));
}

void TabletAreaSelectionController::onTabletAreaSelected()
{
    checkConfigurationForTrackingModeProblems();
}

bool TabletAreaSelectionController::hasView() const
{
    Q_D(const TabletAreaSelectionController);

    return (d->view != nullptr);
}

void TabletAreaSelectionController::checkConfigurationForTrackingModeProblems()
{
    Q_D(TabletAreaSelectionController);

    // a device can not be mapped to a single screen in relative mode
    if (d->currentScreen.isMonitor()) {
        d->view->setTrackingModeWarning(true);
    } else {
        d->view->setTrackingModeWarning(false);
    }
}

const TabletArea TabletAreaSelectionController::convertAreaFromRotation(const TabletArea &tablet, const TabletArea &area, const ScreenRotation &rotation) const
{
    TabletArea result = area;

    if (rotation == ScreenRotation::CW) {
        result.setX(area.y());
        result.setY(tablet.height() - area.x() - area.width());
        result.setWidth(area.height());
        result.setHeight(area.width());

    } else if (rotation == ScreenRotation::CCW) {
        result.setX(tablet.width() - area.y() - area.height());
        result.setY(area.x());
        result.setWidth(area.height());
        result.setHeight(area.width());

    } else if (rotation == ScreenRotation::HALF) {
        result.setX(tablet.width() - area.width() - area.x());
        result.setY(tablet.height() - area.height() - area.y());
        result.setWidth(area.width());
        result.setHeight(area.height());
    }

    return result;
}

const TabletArea TabletAreaSelectionController::convertAreaToRotation(const TabletArea &tablet, const TabletArea &area, const ScreenRotation &rotation) const
{
    TabletArea result = area;

    if (rotation == ScreenRotation::CW) {
        result.setX(tablet.height() - area.height() - area.y());
        result.setY(area.x());
        result.setWidth(area.height());
        result.setHeight(area.width());

    } else if (rotation == ScreenRotation::CCW) {
        result.setX(area.y());
        result.setY(tablet.width() - area.width() - area.x());
        result.setWidth(area.height());
        result.setHeight(area.width());

    } else if (rotation == ScreenRotation::HALF) {
        result.setX(tablet.width() - area.width() - area.x());
        result.setY(tablet.height() - area.height() - area.y());
        result.setWidth(area.width());
        result.setHeight(area.height());
    }

    return result;
}

const QRect TabletAreaSelectionController::getScreenGeometry(QString output) const
{
    Q_D(const TabletAreaSelectionController);

    return d->screenGeometries.value(output, ScreensInfo::getUnifiedDisplayGeometry());
}

const TabletArea TabletAreaSelectionController::getMapping(ScreenSpace screenSpace) const
{
    Q_D(const TabletAreaSelectionController);

    return convertAreaToRotation(d->tabletGeometry, d->screenMap.getMapping(screenSpace), d->tabletRotation);
}

void TabletAreaSelectionController::setMapping(ScreenSpace screenSpace, const TabletArea &mapping)
{
    Q_D(TabletAreaSelectionController);

    TabletArea area = convertAreaFromRotation(d->tabletGeometry, mapping, d->tabletRotation);
    d->screenMap.setMapping(screenSpace, area);
}

void TabletAreaSelectionController::setSelection(const TabletArea &selection)
{
    Q_D(TabletAreaSelectionController);

    if (!hasView()) {
        return;
    }

    if (selection.isEmpty() || selection == d->tabletGeometryRotated) {
        d->view->selectFullTablet();
    } else {
        d->view->selectPartOfTablet(selection);
    }
}

#include "moc_tabletareaselectioncontroller.cpp"
