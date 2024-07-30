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

#ifndef KEYSEQUENCEINPUTBUTTON_H
#define KEYSEQUENCEINPUTBUTTON_H

#include <QPushButton>
#include <QScopedPointer>
#include <QtGlobal>

// forward declarations
class QKeySequence;

namespace Wacom
{

// forward declarations
class KeySequenceInputButtonPrivate;

/**
 * @class KeySequenceInputButton
 *
 * @brief A copy of KKeySequenceWidget which returns all modifiers.
 *
 * This code is mostly copied from KKeySequenceWidget but it allows us to store
 * all key codes of the sequence. While the original implementation would convert
 * "Shift+1" to "!", this implementation will actually return "Shift+!". The wacom
 * driver can not convert shifted keys back to its original key sequence as it
 * treats every key as a keycode. Therefore we have to make sure we return every
 * key which was pressed.
 */
class KeySequenceInputButton : public QPushButton
{
    Q_OBJECT

public:
    //! Default Constructor
    KeySequenceInputButton(QWidget *parent = nullptr);

    //! Destructor
    ~KeySequenceInputButton() override;

    //! Clears the current sequence.
    void clearSequence();

    //! Gets the current sequence.
    const QKeySequence &getSequence() const;

    //! Sets the current sequence.
    void setSequence(const QKeySequence &sequence);

Q_SIGNALS:

    //! Emitted when the key sequences changes.
    void keySequenceChanged(const QKeySequence &sequence);

protected:
    //! Cancels shortcut recording.
    void cancelRecording();

    // overloaded from QPushButton
    bool event(QEvent *event) override;

    // overloaded from QPushButton
    void keyPressEvent(QKeyEvent *event) override;

    // overloaded from QPushButton
    void keyReleaseEvent(QKeyEvent *event) override;

    //! Starts shortcut recording.
    void startRecording();

    //! Finish shortcut recording.
    void stopRecording();

private Q_SLOTS:

    void onButtonClicked();

private:
    //! Hidden Copy Constructor
    KeySequenceInputButton(const KeySequenceInputButton &other) = delete;

    //! Hidden Copy Operator
    KeySequenceInputButton &operator=(const KeySequenceInputButton &other) = delete;

    void recordKey(uint modifierKeys, int keyQt);

    void setupUi();

    void updateShortcutDisplay();

    Q_DECLARE_PRIVATE(KeySequenceInputButton)
    const QScopedPointer<KeySequenceInputButtonPrivate> d_ptr; //!< D-Pointer to private members.

}; // CLASS KeySequenceInputButton
} // NAMESPACE Wacom
#endif // KEYSEQUENCEINPUTBUTTON_H
