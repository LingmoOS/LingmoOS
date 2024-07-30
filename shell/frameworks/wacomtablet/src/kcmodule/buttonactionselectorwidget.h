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

#ifndef BUTTONACTIONSELECTORWIDGET_H
#define BUTTONACTIONSELECTORWIDGET_H

#include <QWidget>

#include "buttonshortcut.h"

namespace Wacom
{

class ButtonActionSelectorWidgetPrivate;

class ButtonActionSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonActionSelectorWidget(QWidget *parent = nullptr);

    ~ButtonActionSelectorWidget() override;

    const ButtonShortcut &getShortcut() const;

    void setShortcut(const ButtonShortcut &shortcut);

signals:

    void buttonActionChanged(const ButtonShortcut &shortcut);

private slots:

    void onButtonActionSelectorClicked();

    void onLineEditSelectionChanged();

private:
    void setupUi();

    void updateActionName(const ButtonShortcut &shortcut);

    void updateSelectorButton(const ButtonShortcut &shortcut);

    Q_DECLARE_PRIVATE(ButtonActionSelectorWidget)
    ButtonActionSelectorWidgetPrivate *const d_ptr; //!< The D-Pointer of this class.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
