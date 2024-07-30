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

#ifndef KEYSEQUENCEINPUTWIDGET_H
#define KEYSEQUENCEINPUTWIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QtGlobal>

// forward declarations
class QKeySequence;

namespace Wacom
{

// forward declarations
class KeySequenceInputWidgetPrivate;

/**
 * @class KeySequenceInputWidget
 *
 * @brief A keyboard shortcut input widget similar to KKeySequence.
 *
 * This class is mostly copied from KKeySequence. However this implementation
 * only supports one shortcut and it will not suppress the shift modifier.
 * To set a key sequence on a wacom tablet the full sequence is required. The
 * original KKeySequence implementation suppresses the shift modifier which
 * makes the shortcut useless for xsetwacom.
 */
class KeySequenceInputWidget : public QWidget
{
    Q_OBJECT

public:
    //! Default Constructor
    KeySequenceInputWidget(QWidget *parent = nullptr);

    //! Destructor
    ~KeySequenceInputWidget() override;

    //! Returns the current key sequence.
    const QKeySequence &keySequence() const;

public Q_SLOTS:

    //! Clears the current key sequence.
    void clearKeySequence();

    //! Sets the key sequence.
    void setKeySequence(const QKeySequence &sequence);

Q_SIGNALS:

    //! Emitted when the key sequence changes.
    void keySequenceChanged(const QKeySequence &sequence);

private Q_SLOTS:

    void onKeySequenceChanged(const QKeySequence &sequence);

private:
    //! Copy Constructor
    KeySequenceInputWidget(const KeySequenceInputWidget &other) = delete;

    //! Copy Operator
    KeySequenceInputWidget &operator=(const KeySequenceInputWidget &other) = delete;

    Q_DECLARE_PRIVATE(KeySequenceInputWidget)
    const QScopedPointer<KeySequenceInputWidgetPrivate> d_ptr; //!< D-Pointer to private members.

}; // CLASS KeySequenceInputWidget
} // NAMESPACE Wacom
#endif // KEYSEQUENCEINPUTWIDGET_H
