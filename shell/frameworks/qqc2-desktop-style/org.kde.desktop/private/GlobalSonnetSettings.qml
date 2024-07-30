/*
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma Singleton

import QtQml.Models
import org.kde.sonnet as Sonnet

/*
 * Global singleton of Sonnet Settings. It is loaded asynchronously when a first
 * non-readonly TextField or TextArea is instantiated.
 */
Instantiator {
    // type-safe nullable reference
    readonly property Sonnet.Settings instance: object as Sonnet.Settings

    // This property can be used as a default binding for Kirigami.SpellCheck.enabled flag.
    readonly property bool checkerEnabledByDefault: instance?.checkerEnabledByDefault ?? false

    active: true
    asynchronous: true

    Sonnet.Settings {}
}
