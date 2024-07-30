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

#ifndef BUTTONACTIONSELECTIONDIALOG_H
#define BUTTONACTIONSELECTIONDIALOG_H

#include <QDialog>

namespace Wacom
{

class ButtonShortcut;
class ButtonActionSelectionDialogPrivate;

/**
 * A dialog which displays the button action selection widget.
 */
class ButtonActionSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ButtonActionSelectionDialog(QWidget *parent = nullptr);

    ~ButtonActionSelectionDialog() override;

    /**
     * Gets the shortcut which was selected by the user. If the
     * user canceled the dialog, the last shortcut which was set
     * using \a setShortcut(const ButtonShortcut&) will be returned.
     *
     * @return The selected button shortcut.
     */
    const ButtonShortcut &getShortcut() const;

    /**
     * Sets a button shortcut.
     *
     * @param shortcut The shortcut to set.
     */
    void setShortcut(const ButtonShortcut &shortcut);

private slots:

    /**
     * Called when the user confirms the dialog with the OK button.
     */
    void onOkClicked();

private:
    Q_DECLARE_PRIVATE(ButtonActionSelectionDialog)
    ButtonActionSelectionDialogPrivate *const d_ptr; //!< D-Pointer for this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
