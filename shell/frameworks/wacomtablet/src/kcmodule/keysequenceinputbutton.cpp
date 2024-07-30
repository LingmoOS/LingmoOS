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

#include "keysequenceinputbutton.h"

#include <KKeyServer>
#include <KLocalizedString>

#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QString>

using namespace Wacom;

namespace Wacom
{
class KeySequenceInputButtonPrivate
{
public:
    KeySequenceInputButtonPrivate()
        : isRecording(false)
        , modifierKeys(0)
    {
    }
    ~KeySequenceInputButtonPrivate()
    {
    }

    bool isRecording;
    QKeySequence keySequence;
    QKeySequence oldSequence;
    uint modifierKeys;
};
}

KeySequenceInputButton::KeySequenceInputButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KeySequenceInputButtonPrivate)
{
    setupUi();
}

KeySequenceInputButton::~KeySequenceInputButton()
{
    // destructor must exist in cpp file, even if not used!
}

void KeySequenceInputButton::clearSequence()
{
    Q_D(KeySequenceInputButton);

    if (d->isRecording) {
        cancelRecording();
    }

    d->keySequence = QKeySequence();
    updateShortcutDisplay();
}

const QKeySequence &KeySequenceInputButton::getSequence() const
{
    Q_D(const KeySequenceInputButton);

    return d->isRecording ? d->oldSequence : d->keySequence;
}

void KeySequenceInputButton::setSequence(const QKeySequence &sequence)
{
    Q_D(KeySequenceInputButton);

    if (d->isRecording) {
        cancelRecording();
    }

    d->keySequence = sequence;
    updateShortcutDisplay();
}

void KeySequenceInputButton::cancelRecording()
{
    Q_D(KeySequenceInputButton);

    d->keySequence = d->oldSequence;
    stopRecording();
}

bool KeySequenceInputButton::event(QEvent *event)
{
    Q_D(KeySequenceInputButton);

    if (d->isRecording && event->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent *>(event));
        return true;
    }

    // prevent local shortcut overrides which could close our dialog
    if (d->isRecording && event->type() == QEvent::ShortcutOverride) {
        event->accept();
        return true;
    }

    return QPushButton::event(event);
}

void KeySequenceInputButton::keyPressEvent(QKeyEvent *event)
{
    Q_D(KeySequenceInputButton);

    int keyQt = event->key();

    // if qt can not resolve a key it returns -1
    if (keyQt == -1) {
        // TODO display red background to inform the user that shortcut recording failed
        return cancelRecording();
    }

    // we only care about modifiers which we can actually handle
    uint newModifierKeys = event->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

    // start recording when return or space is pressed but do not record these buttons
    if (!d->isRecording && (keyQt == Qt::Key_Return || keyQt == Qt::Key_Space)) {
        startRecording();
        recordKey(newModifierKeys, 0);
        updateShortcutDisplay();
        return;
    }

    // ignore everything if we are not recording
    if (!d->isRecording) {
        return QPushButton::keyPressEvent(event);
    }

    // accept the event and record the key
    event->accept();
    recordKey(newModifierKeys, keyQt);
    updateShortcutDisplay();
}

void KeySequenceInputButton::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(KeySequenceInputButton);

    if (event->key() == -1) {
        // ignore unknown keys
        return;
    }

    if (!d->isRecording) {
        return QPushButton::keyReleaseEvent(event);
    }

    event->accept();

    // we only care about modifiers which we can actually handle
    uint newModifierKeys = event->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

    // remove modifier if it was part of our shortcut
    if ((newModifierKeys & d->modifierKeys) < d->modifierKeys) {
        d->modifierKeys = newModifierKeys;
        // TODO control timeout
        updateShortcutDisplay();
    }
}

void KeySequenceInputButton::startRecording()
{
    Q_D(KeySequenceInputButton);

    if (d->isRecording) {
        return;
    }

    d->modifierKeys = 0;
    d->oldSequence = d->keySequence;
    d->keySequence = QKeySequence();
    d->isRecording = true;

    this->grabKeyboard();
    this->setDown(true);

    updateShortcutDisplay();
}

void KeySequenceInputButton::stopRecording()
{
    Q_D(KeySequenceInputButton);

    if (!d->isRecording) {
        return;
    }

    d->isRecording = false;

    this->releaseKeyboard();
    this->setDown(false);

    updateShortcutDisplay();

    if (d->keySequence == d->oldSequence) {
        return;
    }

    emit keySequenceChanged(d->keySequence);
}

void KeySequenceInputButton::onButtonClicked()
{
    Q_D(KeySequenceInputButton);

    if (d->isRecording) {
        return;
    }

    startRecording();
}

void KeySequenceInputButton::recordKey(uint modifierKeys, int keyQt)
{
    Q_D(KeySequenceInputButton);

    if (!d->isRecording) {
        return;
    }

    d->modifierKeys = modifierKeys;

    keyQt &= ~Qt::KeyboardModifierMask;

    switch (keyQt) {
    case Qt::Key_AltGr: // TODO check if xsetwacom supports this
        break;

    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
        // TODO control timeout
        break;

    default:
        if (keyQt) {
            if ((keyQt == Qt::Key_Backtab) && (d->modifierKeys & Qt::SHIFT)) {
                keyQt = Qt::Key_Tab | d->modifierKeys;
            } else {
                keyQt |= d->modifierKeys;
            }

            d->keySequence = QKeySequence(keyQt);

            stopRecording();
            // TODO control timeout
        }
    }
}

void KeySequenceInputButton::setupUi()
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    this->setToolTip(i18n("Click on the button, then enter the shortcut like you would in the program.\nExample for Ctrl+A: hold the Ctrl key and press A."));

    connect(this, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

    updateShortcutDisplay();
}

void KeySequenceInputButton::updateShortcutDisplay()
{
    Q_D(KeySequenceInputButton);

    QString keySequenceString = d->keySequence.toString(QKeySequence::NativeText);
    keySequenceString.replace(QChar::fromLatin1('&'), QLatin1String("&&"));

    if (d->isRecording) {
        if (d->modifierKeys) {
            if (d->modifierKeys & Qt::META) {
                keySequenceString += KKeyServer::modToStringUser(Qt::META) + QChar::fromLatin1('+');
            }

            if (d->modifierKeys & Qt::CTRL) {
                keySequenceString += KKeyServer::modToStringUser(Qt::CTRL) + QChar::fromLatin1('+');
            }

            if (d->modifierKeys & Qt::ALT) {
                keySequenceString += KKeyServer::modToStringUser(Qt::ALT) + QChar::fromLatin1('+');
            }

            if (d->modifierKeys & Qt::SHIFT) {
                keySequenceString += KKeyServer::modToStringUser(Qt::SHIFT) + QChar::fromLatin1('+');
            }

        } else if (keySequenceString.isEmpty()) {
            keySequenceString = i18nc("What the user inputs now will be taken as the new shortcut", "Input");
        }

        keySequenceString.append(QLatin1String(" ..."));
    }

    if (keySequenceString.isEmpty()) {
        keySequenceString = i18nc("No shortcut defined", "None");
    }

    keySequenceString.prepend(QChar::fromLatin1(' '));
    keySequenceString.append(QChar::fromLatin1(' '));

    this->setText(keySequenceString);
}

#include "moc_keysequenceinputbutton.cpp"
