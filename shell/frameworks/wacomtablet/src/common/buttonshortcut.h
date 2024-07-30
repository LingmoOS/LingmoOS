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

#ifndef BUTTONSHORTCUT_H
#define BUTTONSHORTCUT_H

#include <QMap>
#include <QString>
#include <QStringList>

namespace Wacom
{

class ButtonShortcutPrivate;

/**
 * A shortcut class which can handle all supported shortcut types and is
 * able to convert shortcut sequences between xsetwacom- and QKeySequence-
 * format.
 */
class ButtonShortcut
{
public:
    //! The shortcut types supported by this class.
    enum class ShortcutType {
        NONE = 0,
        BUTTON = 1,
        KEYSTROKE = 2,
        MODIFIER = 3,
    };

    //! Default Constructor
    ButtonShortcut();

    //! Copy Constructor
    explicit ButtonShortcut(const ButtonShortcut &that);

    /**
     * @brief Shortcut assignment constructor.
     *
     * Assigns a shortcut as returned by toString().
     *
     * @param shortcut The shortcut to assign.
     */
    explicit ButtonShortcut(const QString &shortcut);

    /**
     * @brief Button assignment constructor.
     *
     * Assigns a button number to the shortcut.
     *
     * @param buttonNumber The button number to assign.
     */
    explicit ButtonShortcut(int buttonNumber);

    //! Destructor
    virtual ~ButtonShortcut();

    //! Copy operator.
    ButtonShortcut &operator=(const ButtonShortcut &that);

    /**
     * @brief Shortcut assignment operator.
     *
     * Assigns a shortcut as returned by toString().
     *
     * @param shortcut The shortcut to assign.
     */
    ButtonShortcut &operator=(const QString &shortcut);

    //! Equals operator.
    bool operator==(const ButtonShortcut &that) const;

    //! Not-Equals operator.
    bool operator!=(const ButtonShortcut &that) const;

    /**
     * Clears the current shortcut.
     */
    void clear();

    /**
     * Returns the button number as integer if this shortcut is a mouse button
     * shortcut. If it is not a mouse button shortcut, 0 is returned.
     *
     * @return The mouse button number if this shortcut is a button shortcut, else 0.
     */
    int getButton() const;

    /**
     * @return The shortcut type.
     */
    ShortcutType getType() const;

    /**
     * @return True if this shortcut is a mouse button shortcut, else false.
     */
    bool isButton() const;

    /**
     * Checks if this shortcut is a keystroke. This does not include modifier
     * shortcuts! Modifier shortcuts are a special type as they can not be
     * handled properly by QKeySequence. Therefore a keystroke is a key sequence
     * which can be handled by QKeySequence.
     *
     * @return True if this shortcut is a keystroke, else false.
     */
    bool isKeystroke() const;

    /**
     * Checks if this shortcut is a modifier key sequence. Modifier key sequences
     * only consist of meta keys (ctrl, alt, shift, meta) and can not be handled
     * properly by QKeySequence.
     *
     * @return True if this shortcut is a modifier shortcut, else false.
     */
    bool isModifier() const;

    /**
     * @return True if this shortcut is set, else false.
     */
    bool isSet() const;

    /**
     * Sets a button shortcut by button number.
     *
     * @param buttonNumber The mouse button number this shortcut represents.
     *
     * @return True if the shortcut is valid, else false.
     */
    bool setButton(int buttonNumber);

    /**
     * Sets this shortcut by string. The shortcut can have one of the following
     * formats:
     *
     * - a mouse button number to set a button shortcut, i.e. "2"
     * - a keystroke in QKeySequence format, i.e. "Ctrl+x"
     * - a keystroke in xsetwacom format, i.e. "key ctrl x" or "key +ctrl +x"
     * - a modifier sequence in QKeySequence format, i.e. "Alt+Shift+Ctrl"
     * - a modifier sequence in xsetwacom format, i.e. "key alt shift" or "key +alt +shift"
     *
     * If the shortcut is invalid it will not be set.
     *
     * @param sequence The button, toggle, keystroke or modifier shortcut as string.
     *
     * @return True if the shortcut is valid, else false.
     */
    bool set(const QString &sequence);

    /**
     * Converts the shortcut to a translated, human readable string.
     * The result can not be used as input string for this class!
     * If no shortcut is set, an empty string is returned.
     *
     * @return The current shortcut as readable string.
     */
    const QString toDisplayString() const;

    /**
     * Converts the current shortcut to QKeySequence format if possible.
     * Only keystroke shortcuts can be converted to QKeySequence format.
     * If the shortcut can not be converted, an empty string is returned.
     *
     * @return The shortcut in QKeySequence format or an empty string.
     */
    const QString toQKeySequenceString() const;

    /**
     * Returns the current shortcut as string in xsetwacom format. This will be:
     *
     * - "0" if the sequence is not set
     * - "modetoggle" if the shortcut is a mode-toggle shortcut.
     * - "displaytoggle" if the shortcut is a display-toggle shortcut.
     * - a button number if the shortcut is a mouse button shortcut.
     * - a modifier or keystroke sequence in xsetwacom format, i.e. "key ctrl x" or "key alt shift"
     *
     * @return The shortcut as string in xsetwacom format.
     */
    const QString toString() const;

private:
    /**
     * A two dimensional array which contains key conversions. The first value
     * is the key in storage format, the second value is the key in QKeySequence
     * format. The array has to be NULL terminated!
     *
     * DO NOT USE THIS ARRAY DIRECTLY! It is only required to build the static
     * conversion maps and should not be used for anything else.
     *
     * @sa getConvertFromStorageMap()
     * @sa getConvertToStorageMap()
     */
    static const char *CONVERT_KEY_MAP_DATA[][2];

    /**
     * Converts a key from storage format to QKeySequence format or vice versa.
     * Storage format is actually the format used by xsetwacom.
     *
     * @param key The key to convert. This parameter will also contain the conversion result.
     * @param fromStorage True to convert from xsetwacom to QKeySequence format, False to convert
     *                    from QKeySequence to xsetwacom format.
     *
     * @return True if the key was converted, false if it was not touched.
     */
    bool convertKey(QString &key, bool fromStorage) const;

    /**
     * Normalizes the key sequence and converts all keys.
     * The result is a string of keys separated by whitespaces.
     *
     * @param sequence The sequence to convert. This parameter will also hold the result of the conversion.
     * @param fromStorage True to convert from xsetwacom to QKeySequence format, False to convert
     *                    from QKeySequence to xsetwacom format.
     */
    void convertToNormalizedKeySequence(QString &sequence, bool fromStorage) const;

    /**
     * Normalizes the key sequence and converts it to storage format.
     * The result is a string of keys separated by whitespaces.
     *
     * @param sequence The sequence to convert. This parameter will also hold the result of the conversion.
     */
    void convertKeySequenceToStorageFormat(QString &sequence) const;

    /**
     * Normalizes the key sequence and converts it to QKeySequence format.
     * The result is a string of keys separated by whitespaces.
     *
     * @param sequence The sequence to convert. This parameter will also hold the result of the conversion.
     */
    void convertKeySequenceToQKeySequenceFormat(QString &sequence) const;

    /**
     * Returns the map which is used to convert keys from storage format to
     * the QKeySequence format.
     */
    static const QMap<QString, QString> &getConvertFromStorageMap();

    /**
     * Returns the map which is used to convert keys from QKeySequence format to
     * the storage format.
     */
    static const QMap<QString, QString> &getConvertToStorageMap();

    /**
     * This is just a helper method to initialize the static conversion maps.
     * Do not use it for anything else!
     *
     * @sa getConvertFromStorageMap()
     * @sa getConvertToStorageMap()
     */
    static QMap<QString, QString> initConversionMap(bool fromStorageMap);

    /**
     * Normalizes the key sequence by removing all unnecessary '+' signs.
     * The result is a string of keys and modifiers separated by a whitespace.
     * This method also does some basic checks to verify that the syntax of
     * the sequence is valid. If the sequence is invalid it will be cleared and
     * the result will therefore be an empty string. The result of this method
     * will be directly written into the parameter \a sequence.
     *
     * @param sequence The sequence to normalize.
     */
    void normalizeKeySequence(QString &sequence) const;

    /**
     * Prettifies a key by converting the first character to uppercase.
     *
     * @param key The key to prettify, this will also contain the result.
     */
    void prettifyKey(QString &key) const;

    /**
     * Sets a button sequence. This method expects that the given sequence is
     * actually a button sequence. If it is not, this method will fail and the
     * shortcut will not be set.
     *
     * @param buttonSequence The mouse button number as string.
     *
     * @return True if the button is valid and was set, else false.
     */
    bool setButtonSequence(const QString &buttonSequence);

    /**
     * Set a keystroke sequence. This methods expects that the given sequence
     * is actually a keystroke sequence. If it is not, this method will fail and
     * the shortcut will not be set.
     *
     * @param sequence The keystroke sequence to set.
     *
     * @return True if the sequence is valid and was set, else false.
     */
    bool setKeySequence(QString sequence);

    /**
     * Sets a modifier sequence. This method expects that the given sequence
     * is actually a modifier sequence. If it is not, this method will fail and
     * the shortcut will not be set.
     *
     * @param sequence The modifier sequence to set.
     *
     * @return True if the sequence is valid and was set, else false.
     */
    bool setModifierSequence(QString sequence);

    Q_DECLARE_PRIVATE(ButtonShortcut)
    ButtonShortcutPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
