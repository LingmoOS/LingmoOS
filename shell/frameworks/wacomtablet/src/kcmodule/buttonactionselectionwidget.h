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

#ifndef BUTTONACTIONSELECTIONWIDGET_H
#define BUTTONACTIONSELECTIONWIDGET_H

#include <QWidget>

class QCheckBox;
class QKeySequence;

namespace Ui
{
class ButtonActionSelectionWidget;
}

namespace Wacom
{

class ButtonShortcut;
class ButtonActionSelectionWidgetPrivate;

class ButtonActionSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonActionSelectionWidget(QWidget *parent = nullptr);

    ~ButtonActionSelectionWidget() override;

    /**
     * @return The currently selected shortcut.
     */
    const ButtonShortcut &getShortcut() const;

    /**
     * Sets a shortcut.
     *
     * @param shortcut The shortcut to set.
     */
    void setShortcut(const ButtonShortcut &shortcut);

private slots:

    /**
     * Called when one of the clear buttons is clicked.
     */
    void onClearButtonClicked(bool checked);

    /**
     * Called when the keyboard shortcut sequence is modified.
     */
    void onShortcutChanged(QKeySequence sequence);

    /**
     * Called when the user tries to select the action text.
     */
    void onActionLineEditSelectionChanged();

    /**
     * Called when the state of a modifier checkbox changes.
     */
    void onModifierChanged(int state);

    /**
     * Called when the mouse button selection was changed.
     */
    void onMouseSelectionChanged(int index);

private:
    /**
     * Sets up the user interface. This should only be called once by the constructor.
     */
    void setupUi();

    /**
     * Determines a name for the current shortcut and updates the
     * name widget. This will also take into account global shortcuts.
     *
     * @param shortcut The current shortcut.
     */
    void updateCurrentActionName(const ButtonShortcut &shortcut);

    /**
     * Updates the modifier selection widgets for the current shortcut.
     * While the update is in progress all signals will be blocked.
     *
     * @param shortcut The current shortcut.
     */
    void updateModifierWidgets(const ButtonShortcut &shortcut);

    /**
     * Updates the mouse button selection for the current shortcut.
     * While the update is in progress all signals will be blocked.
     *
     * @param shortcut The current shortcut.
     */
    void updateMouseButtonSeletion(const ButtonShortcut &shortcut);

    /**
     * Updates the checked state of a  QCheckBox. While the
     * update is in progress all signals will be blocked.
     *
     * @param checkbox The checkbox to update.
     * @param isChecked A flag to signal if the checkbox is checked.
     */
    void updateQCheckBox(QCheckBox &checkbox, bool isChecked) const;

    /**
     * Updates the keyboard shortcut widget according to the current shortcut.
     * While the update is in progress all shortcut widgets will be blocked.
     *
     * @param shortcut The current shortcut.
     */
    void updateShortcutWidgets(const ButtonShortcut &shortcut);

    Q_DECLARE_PRIVATE(ButtonActionSelectionWidget)
    ButtonActionSelectionWidgetPrivate *const d_ptr; //!< D-Pointer for this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
