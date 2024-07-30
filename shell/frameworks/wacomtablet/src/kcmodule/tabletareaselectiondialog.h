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

#ifndef TABLETAREASELECTIONDIALOG_H
#define TABLETAREASELECTIONDIALOG_H

#include "screenmap.h"
#include "screenrotation.h"
#include "screenspace.h"

#include <QDialog>
#include <QList>
#include <QRect>

namespace Wacom
{

class TabletAreaSelectionDialogPrivate;

/**
 * A dialog which displays the current screen area selection and lets
 * the user map this selection to a tablet area.
 */
class TabletAreaSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TabletAreaSelectionDialog();

    ~TabletAreaSelectionDialog() override;

    /**
     * Gets the current selection in profile format.
     * See TabletAreaSelectionWidget::getSelection() for a list of possible return values.
     *
     * @return The current selection in profile format.
     */
    const ScreenMap &getScreenMap();

    const ScreenSpace getScreenSpace() const;

    void select(QString output);

    void select(const ScreenSpace &screenSpace);

    void setupWidget(const ScreenMap &mappings, const QString &deviceName, const ScreenRotation &rotation);

private:
    /**
     * Sets up the dialog. Must only be called once by a constructor.
     */
    void setupUi();

    Q_DECLARE_PRIVATE(TabletAreaSelectionDialog)
    TabletAreaSelectionDialogPrivate *const d_ptr; //!< D-Pointer for this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
