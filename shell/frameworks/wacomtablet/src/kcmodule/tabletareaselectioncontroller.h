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

#ifndef TABLETAREASELECTIONCONTROLLER_H
#define TABLETAREASELECTIONCONTROLLER_H

#include "screenmap.h"
#include "screenrotation.h"
#include "screenspace.h"
#include "tabletarea.h"

#include <QObject>
#include <QString>

namespace Wacom
{

class TabletAreaSelectionControllerPrivate;
class TabletAreaSelectionView;

class TabletAreaSelectionController : public QObject
{
    Q_OBJECT

public:
    TabletAreaSelectionController();
    ~TabletAreaSelectionController() override;

    /**
     * Get the screen space mapping.
     *
     * @return The current screen space mapping.
     */
    const ScreenMap &getScreenMap();

    /**
     * @return The current screen space which was selected.
     */
    const ScreenSpace getScreenSpace() const;

    /**
     * Shows the selection for the given screen space.
     */
    void select(const ScreenSpace &screenSpace);

    /**
     * Sets the view. When a view has been set, the controller
     * needs to be set up again.
     *
     * @param view The view to control.
     */
    void setView(TabletAreaSelectionView *view);

    /**
     * Initializes the controller. This method has to be called before the widget
     * can be used.
     *
     * @param mappings The screen mappings of the device we are handling.
     * @param deviceName The X11 xinput device name of the device we ware handling.
     * @param rotation The currently selected tablet rotation.
     */
    void setupController(const ScreenMap &mappings, const QString &deviceName, const ScreenRotation &rotation);

public slots:

    /**
     * Called by the view when the user wants to calibrate the tablet.
     */
    void onCalibrateClicked();

    /**
     * Called by the view when the user selected the full tablet area.
     */
    void onFullTabletSelected();

    /**
     * Called by the view when the user wants to toggle the screen.
     */
    void onScreenToggle();

    /**
     * Called by the view when the user wants to set screen proportions on his tablet.
     */
    void onSetScreenProportions();

    /**
     * Called by the view when the user selected a tablet area.
     */
    void onTabletAreaSelected();

private:
    void checkConfigurationForTrackingModeProblems();

    const TabletArea convertAreaFromRotation(const TabletArea &tablet, const TabletArea &area, const ScreenRotation &rotation) const;

    const TabletArea convertAreaToRotation(const TabletArea &tablet, const TabletArea &area, const ScreenRotation &rotation) const;

    const QRect getScreenGeometry(QString output) const;

    const TabletArea getMapping(ScreenSpace screenSpace) const;

    void setMapping(ScreenSpace screenSpace, const TabletArea &mapping);

    /**
     * @return True if this controller has a view, else false.
     */
    bool hasView() const;

    /**
     * Sets a selection on the view. This also checks for invalid selections
     * and automatically determines if the selection is a full tablet selection
     * or a part of the tablet.
     *
     * @param selection The selection to set.
     */
    void setSelection(const TabletArea &selection);

    Q_DECLARE_PRIVATE(TabletAreaSelectionController)
    TabletAreaSelectionControllerPrivate *const d_ptr; //!< D-Pointer for this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
