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

#include "buttonshortcut.h"

#include <QKeySequence>
#include <QRegularExpression>

#include <KGlobalAccel>
#include <KLocalizedString>

using namespace Wacom;

/*
 * When mapping multiple keys to the same name, the last entry
 * will be the default one. This is because QMap.find() returns
 * the most recently added value as default.
 *
 * Example:
 *
 * super_l = meta
 * super   = meta  <-- default entry when searching for "meta"
 *
 */
const char *ButtonShortcut::CONVERT_KEY_MAP_DATA[][2] = {
    // clang-format off
    {"alt_l",        "alt"},
    {"alt_r",        "alt"},
    {"alt",          "alt"},  // "alt" default
    {"ampersand",    "&"},
    {"apostrophe",   "'"},
    {"asciicircum",  "^"},
    {"asciitilde",   "~"},
    {"asterisk",     "*"},
    {"at",           "@"},
    {"backslash",    "\\"},
    {"bar",          "|"},
    {"braceleft",    "{"},
    {"braceright",   "}"},
    {"bracketleft",  "["},
    {"bracketright", "]"},
    {"colon",        ":"},
    {"comma",        ","},
    {"ctrl_l",       "ctrl"},
    {"ctrl_r",       "ctrl"},
    {"ctrl",         "ctrl"}, // "ctrl" default
    {"dollar",       "$"},
    {"equal",        "="},
    {"exclam",       "!"},
    {"greater",      ">"},
    {"less",         "<"},
    {"pgdn",         "pgdown"},
    {"numbersign",   "#"},
    {"period",       "."},
    {"plus",         "+"},
    {"minus",        "-"},
    {"parenleft",    "("},
    {"parenright",   ")"},
    {"percent",      "%"},
    {"question",     "?"},
    {"quotedbl",     "\""},
    {"quoteleft",    "`"},
    {"semicolon",    ";"},
    {"slash",        "/"},
    {"super_l",      "meta"},
    {"super_r",      "meta"},
    {"super",        "meta"}, // "super" default
    {"underscore",   "_"},
    {nullptr, nullptr},
    // clang-format on
};

QString buttonNumberToDescription(int buttonNr)
{
    switch (buttonNr) {
    case 1:
        return i18nc("Tablet button triggers a left mouse button click.", "Left Mouse Button Click");
    case 2:
        return i18nc("Tablet button triggers a middle mouse button click.", "Middle Mouse Button Click");
    case 3:
        return i18nc("Tablet button triggers a right mouse button click.", "Right Mouse Button Click");
    case 4:
        return i18nc("Tablet button triggers mouse wheel up.", "Mouse Wheel Up");
    case 5:
        return i18nc("Tablet button triggers mouse wheel down.", "Mouse Wheel Down");
    case 6:
        return i18nc("Tablet button triggers mouse wheel left.", "Mouse Wheel Left");
    case 7:
        return i18nc("Tablet button triggers mouse wheel right.", "Mouse Wheel Right");
    default:
        return i18nc("Tablet button triggers a click of mouse button with number #", "Mouse Button %1 Click", buttonNr);
    }
}

namespace Wacom
{
class ButtonShortcutPrivate
{
public:
    ButtonShortcutPrivate()
    {
        type = ButtonShortcut::ShortcutType::NONE;
        button = 0;
    }

    ButtonShortcut::ShortcutType type;
    QString sequence; //!< The key or modifier sequence.
    int button; //!< The button number.
};
}

ButtonShortcut::ButtonShortcut()
    : d_ptr(new ButtonShortcutPrivate)
{
}

ButtonShortcut::ButtonShortcut(const ButtonShortcut &that)
    : d_ptr(new ButtonShortcutPrivate)
{
    operator=(that);
}

ButtonShortcut::ButtonShortcut(const QString &shortcut)
    : d_ptr(new ButtonShortcutPrivate)
{
    set(shortcut);
}

ButtonShortcut::ButtonShortcut(int buttonNumber)
    : d_ptr(new ButtonShortcutPrivate)
{
    setButton(buttonNumber);
}

ButtonShortcut::~ButtonShortcut()
{
    delete this->d_ptr;
}

ButtonShortcut &ButtonShortcut::operator=(const ButtonShortcut &that)
{
    Q_D(ButtonShortcut);

    *d = *(that.d_ptr);
    return *this;
}

ButtonShortcut &ButtonShortcut::operator=(const QString &shortcut)
{
    set(shortcut);
    return *this;
}

bool ButtonShortcut::operator==(const ButtonShortcut &that) const
{
    Q_D(const ButtonShortcut);

    if (d->type != that.d_ptr->type) {
        return false;
    }

    if (d->button != that.d_ptr->button) {
        return false;
    }

    if (d->sequence.compare(that.d_ptr->sequence, Qt::CaseInsensitive) != 0) {
        return false;
    }

    return true;
}

bool ButtonShortcut::operator!=(const ButtonShortcut &that) const
{
    return (!operator==(that));
}

void ButtonShortcut::clear()
{
    Q_D(ButtonShortcut);

    d->type = ShortcutType::NONE;
    d->button = 0;
    d->sequence.clear();
}

int ButtonShortcut::getButton() const
{
    Q_D(const ButtonShortcut);
    return d->button;
}

ButtonShortcut::ShortcutType ButtonShortcut::getType() const
{
    Q_D(const ButtonShortcut);
    return d->type;
}

bool ButtonShortcut::isButton() const
{
    return (getType() == ShortcutType::BUTTON);
}

bool ButtonShortcut::isKeystroke() const
{
    return (getType() == ShortcutType::KEYSTROKE);
}

bool ButtonShortcut::isModifier() const
{
    return (getType() == ShortcutType::MODIFIER);
}

bool ButtonShortcut::isSet() const
{
    return (getType() != ShortcutType::NONE);
}

bool ButtonShortcut::setButton(int buttonNumber)
{
    Q_D(ButtonShortcut);

    clear();

    if (buttonNumber > 0 && buttonNumber <= 32) {
        d->type = ShortcutType::BUTTON;
        d->button = buttonNumber;
        return true;
    }

    return false;
}

bool ButtonShortcut::set(const QString &sequence)
{
    clear();

    QString seq = sequence.trimmed();

    if (seq.isEmpty()) {
        return true;
    }

    static const QRegularExpression modifierRx(QLatin1String("^(?:key )?(?:\\s*\\+?(?:alt|ctrl|meta|shift|super))+$"),
                                               QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression buttonRx(QLatin1String("^(?:button\\s+)?\\+?\\d+$"), QRegularExpression::CaseInsensitiveOption);

    if (seq.contains(buttonRx)) {
        // this is a button
        return setButtonSequence(seq);

    } else if (seq.contains(modifierRx)) {
        // this is a modifier sequence
        return setModifierSequence(seq);
    }

    // this is probably a key sequence
    return setKeySequence(seq);
}

const QString ButtonShortcut::toDisplayString() const
{
    Q_D(const ButtonShortcut);

    QList<KGlobalShortcutInfo> globalShortcutList;
    QString displayString;
    int buttonNr = getButton();

    switch (d->type) {
    case ShortcutType::BUTTON:
        displayString = buttonNumberToDescription(buttonNr);
        break;

    case ShortcutType::MODIFIER:
        displayString = d->sequence;
        convertKeySequenceToQKeySequenceFormat(displayString);
        break;

    case ShortcutType::KEYSTROKE:
        displayString = d->sequence;
        convertKeySequenceToQKeySequenceFormat(displayString);

        // check if a global shortcut is assigned to this sequence
        globalShortcutList = KGlobalAccel::globalShortcutsByKey(QKeySequence(displayString));

        if (!globalShortcutList.isEmpty()) {
            displayString = globalShortcutList.at(0).uniqueName();
        }
        break;

    case ShortcutType::NONE:
        break;
    }

    return displayString;
}

const QString ButtonShortcut::toQKeySequenceString() const
{
    Q_D(const ButtonShortcut);

    QString keySequence;

    if (d->type == ShortcutType::KEYSTROKE) {
        keySequence = d->sequence;
        convertKeySequenceToQKeySequenceFormat(keySequence);
    }

    return keySequence;
}

const QString ButtonShortcut::toString() const
{
    Q_D(const ButtonShortcut);

    QString shortcutString = QLatin1String("0");

    switch (d->type) {
    case ShortcutType::BUTTON:
        shortcutString = QString::number(d->button);
        break;

    case ShortcutType::MODIFIER:
    case ShortcutType::KEYSTROKE:
        shortcutString = QString::fromLatin1("key %2").arg(d->sequence);
        break;

    case ShortcutType::NONE:
        break;
    }

    return shortcutString.toLower();
}

bool ButtonShortcut::convertKey(QString &key, bool fromStorage) const
{
    QMap<QString, QString>::ConstIterator iter;
    QMap<QString, QString>::ConstIterator iterEnd;

    if (fromStorage) {
        iter = getConvertFromStorageMap().find(key.toLower());
        iterEnd = getConvertFromStorageMap().constEnd();
    } else {
        iter = getConvertToStorageMap().find(key.toLower());
        iterEnd = getConvertToStorageMap().constEnd();
    }

    if (iter == iterEnd) {
        return false;
    }

    key = iter.value();

    return true;
}

void ButtonShortcut::convertToNormalizedKeySequence(QString &sequence, bool fromStorage) const
{
    normalizeKeySequence(sequence);

    static const QRegularExpression rx(QStringLiteral("\\s+"));
    QStringList keyList = sequence.split(rx, Qt::SkipEmptyParts);
    bool isFirstKey = true;

    sequence.clear();

    for (QStringList::iterator iter = keyList.begin(); iter != keyList.end(); ++iter) {
        convertKey(*iter, fromStorage);
        prettifyKey(*iter);

        if (isFirstKey) {
            sequence.append(*iter);
            isFirstKey = false;
        } else {
            sequence.append(QString::fromLatin1(" %1").arg(*iter));
        }
    }
}

void ButtonShortcut::convertKeySequenceToStorageFormat(QString &sequence) const
{
    convertToNormalizedKeySequence(sequence, false);
}

void ButtonShortcut::convertKeySequenceToQKeySequenceFormat(QString &sequence) const
{
    convertToNormalizedKeySequence(sequence, true);
    sequence.replace(QLatin1String(" "), QLatin1String("+"));
}

const QMap<QString, QString> &ButtonShortcut::getConvertFromStorageMap()
{
    static QMap<QString, QString> map = initConversionMap(true);
    return map;
}

const QMap<QString, QString> &ButtonShortcut::getConvertToStorageMap()
{
    static QMap<QString, QString> map = initConversionMap(false);
    return map;
}

QMap<QString, QString> ButtonShortcut::initConversionMap(bool fromStorageMap)
{
    QMap<QString, QString> map;

    for (int i = 0;; ++i) {
        if (CONVERT_KEY_MAP_DATA[i][0] == nullptr || CONVERT_KEY_MAP_DATA[i][1] == nullptr) {
            return map;
        }

        if (fromStorageMap) {
            map.insert(QLatin1String(CONVERT_KEY_MAP_DATA[i][0]), QLatin1String(CONVERT_KEY_MAP_DATA[i][1]));
        } else {
            map.insert(QLatin1String(CONVERT_KEY_MAP_DATA[i][1]), QLatin1String(CONVERT_KEY_MAP_DATA[i][0]));
        }
    }

    return map;
}

void ButtonShortcut::normalizeKeySequence(QString &sequence) const
{
    // When setting a shortcut like "ctrl+x", xsetwacom will convert it to "key +ctrl +x -x"
    // therefore we just truncate the string on the first "-key" we find.
    static const QRegularExpression minusKeyRx(QLatin1String("(^|\\s)-\\S"));

    const QRegularExpressionMatch minusKeyRxMatch = minusKeyRx.match(sequence);

    if (minusKeyRxMatch.hasMatch()) {
        sequence = sequence.left(minusKeyRxMatch.capturedStart());
    }

    // cleanup leading "key " identifier from xsetwacom sequences
    static const QRegularExpression leadingKey(QStringLiteral("^\\s*key\\s+"), QRegularExpression::CaseInsensitiveOption);
    sequence.remove(leadingKey);

    // Remove all '+' prefixes from keys.
    // This will convert shortcuts like "+ctrl +alt" to "ctrl alt", but not
    // shortcuts like "ctrl +" which is required to keep compatibility to older
    // (buggy) configuration files.
    static const QRegularExpression plusPrefixes(QStringLiteral("(^|\\s)\\+(\\S)"), QRegularExpression::CaseInsensitiveOption);
    sequence.replace(plusPrefixes, QLatin1String("\\1\\2"));

    // Cleanup plus signs between keys.
    // This will convert shortcuts like "ctrl+alt+shift" or "Ctrl++" to "ctrl alt shift" or "Ctrl +".
    static const QRegularExpression cleanupPlus(QStringLiteral("(\\S)\\+(\\S)"), QRegularExpression::CaseInsensitiveOption);
    sequence.replace(cleanupPlus, QLatin1String("\\1 \\2"));

    // replace multiple whitespaces with one
    static const QRegularExpression whitespaces(QStringLiteral("\\s{2,}"), QRegularExpression::CaseInsensitiveOption);
    sequence.replace(whitespaces, QLatin1String(" "));

    // trim the string
    sequence = sequence.trimmed();
}

void ButtonShortcut::prettifyKey(QString &key) const
{
    if (!key.isEmpty()) {
        key = key.toLower();
        key[0] = key.at(0).toUpper();
    }
}

bool ButtonShortcut::setButtonSequence(const QString &buttonSequence)
{
    QString buttonNumber = buttonSequence;
    static const QRegularExpression rx(QStringLiteral("^\\s*button\\s+"), QRegularExpression::CaseInsensitiveOption);
    buttonNumber.remove(rx);

    bool ok = false;
    int button = buttonNumber.toInt(&ok);

    if (!ok) {
        return false;
    }

    return setButton(button);
}

bool ButtonShortcut::setKeySequence(QString sequence)
{
    Q_D(ButtonShortcut);

    clear();

    // Check if this keysequence is valid by converting it to a QKeySequence and then back
    // again. If both sequences are equal the sequence should be valid. This is not very
    // effective but it leaves us with something KDE can handle and it makes sure a key
    // is always converted to its default key name if there are multiple mappings.
    // TODO improve this
    QString convertedSequence = sequence;
    convertKeySequenceToQKeySequenceFormat(convertedSequence);

    QKeySequence qkeySequence(convertedSequence);
    convertedSequence = qkeySequence.toString();

    convertKeySequenceToStorageFormat(convertedSequence);
    convertKeySequenceToStorageFormat(sequence);

    // we do not allow invalid sequences to be set
    // the sequences have to be equal and only one sequence is allowed
    if (sequence.compare(convertedSequence, Qt::CaseInsensitive) != 0 || qkeySequence.count() != 1) {
        return false;
    }

    d->type = ShortcutType::KEYSTROKE;
    d->sequence = sequence;

    return true;
}

bool ButtonShortcut::setModifierSequence(QString sequence)
{
    Q_D(ButtonShortcut);

    clear();
    convertKeySequenceToStorageFormat(sequence);

    d->type = ShortcutType::MODIFIER;
    d->sequence = sequence;

    return true;
}
