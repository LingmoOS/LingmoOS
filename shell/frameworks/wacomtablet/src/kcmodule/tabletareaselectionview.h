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

#ifndef TABLETAREASELECTIONVIEW_H
#define TABLETAREASELECTIONVIEW_H

#include "tabletarea.h"

#include <QObject>
#include <QRect>
#include <QString>
#include <QWidget>

namespace Wacom
{

class TabletAreaSelectionViewPrivate;

/**
 * A widget which displays the current screen area selection and lets
 * the user map this selection to a tablet area.
 */
class TabletAreaSelectionView : public QWidget
{
    Q_OBJECT

public:
    explicit TabletAreaSelectionView(QWidget *parent = nullptr);

    ~TabletAreaSelectionView() override;

    /**
     * @return The current selection as rectangle.
     */
    const TabletArea getSelection() const;

    /**
     * Selects all of the tablet.
     */
    void selectFullTablet();

    /**
     * Selects part of the tablet.
     *
     * @param selection The part to select, may not be empty.
     */
    void selectPartOfTablet(const TabletArea &selection);

    /**
     * Switches to the given screen and selects the given tablet region.
     *
     * @param output The name of the screen to switch to.
     * @param isDesktop Whether the screen is a desktop screen.
     * @param tabletSelection The selection to set on the tablet.
     */
    void select(QString output, bool isDesktop, const TabletArea &tabletSelection);

    /**
     * Shows or hides a warning that the current selection is only
     * available in absolute tracking mode.
     *
     * @param doShow If true the warning is displayed, else it is hidden.
     */
    void setTrackingModeWarning(bool doShow);

    /**
     * Sets up the screen area widget.
     *
     * @param screenGeometries The X11 geometries of the connected screens.
     * @param widgetTargetSize The target size of the screen area widget.
     */
    void setupScreens(const QMap<QString, QRect> &screenGeometries, const QSize &widgetTargetSize);

    /**
     * Sets up the tablet area widget.
     *
     * @param geometry The geometry of the tablet.
     * @param widgetTargetSize The target size of the tablet area widget.
     */
    void setupTablet(const TabletArea &geometry, const QSize &widgetTargetSize);

public slots:

    /**
     * Called by the UI when the user wants to calibrate the current device.
     */
    void onCalibrateClicked();

    /**
     * Called by the UI when the user wants to adjust the selection to the screen proportions.
     */
    void onForceProportionsClicked();

    /**
     * Called by the UI when the user wants to select the full tablet area.
     */
    void onFullTabletSelected(bool checked);

    /**
     * Called by the UI when the user wants to toggle the screen.
     */
    void onScreenToggle();

    /**
     * Called by the UI when the user wants to select an area of the tablet.
     */
    void onTabletAreaSelected(bool checked);

    /**
     * @brief onLockProportionsToggled
     * @param enabled
     */
    void onLockProportionsToggled(bool enabled);

    void onSelectionChanged();

    void onFineTuneValuesChanged(QString);

signals:

    /**
     * Signals the controller that the user wants to calibrate the tablet.
     */
    void signalCalibrateClicked();

    /**
     * Signals the controller that the user selected the full tablet area.
     */
    void signalFullTabletSelection();

    /**
     * Signals the controller that the user wants to toggle the screen.
     */
    void signalScreenToggle();

    /**
     * Signals the controller that the user wants to set screen proportions.
     */
    void signalSetScreenProportions();

    /**
     * Signals the controller that the user selected a tablet area.
     */
    void signalTabletAreaSelection();

protected:
    /**
     * The available mapping types for the tablet.
     */
    enum TabletAreaType {
        FullTabletArea, //!< Enables full desktop selection.
        PartialTabletArea //!< Enables area selection.
    };

    /**
     * Sets a selection on the tablet based on the given geometry.
     * This does not update any other widgets. It only tells the
     * area widget to select the specified area. However a check
     * is done if the selection is valid. If it is invalid, the full
     * area will be selected.
     *
     * @param selection The geometry of the new selection.
     */
    void setSelection(const TabletArea &selection);

    /**
     * Sets the given mapping type and updates the widgets
     * and the selection.
     *
     * @param type The new mapping type.
     */
    void setTabletAreaType(TabletAreaType type);

private:
    /**
     * Checks if the given selection selects the full tablet area.
     *
     * @param selection The selection to check.
     *
     * @return True if the given selection selects the full tablet area, else false.
     */
    bool isFullAreaSelection(const TabletArea &selection) const;

    /**
     * Sets up this widget. Must only be called once by the constructor.
     */
    void setupUi();

    Q_DECLARE_PRIVATE(TabletAreaSelectionView)
    TabletAreaSelectionViewPrivate *const d_ptr; //!< D-Pointer for this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
