/*
   SPDX-FileCopyrightText: 2024 g10 Code GmbH
   SPDX-FileContributor: Carl Schwan <carl.schwan@gnupg.com>
   SPDX-FileContributor: Sune Stolborg Vuorela <sune@vuorela.dk>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KPASSWORD_H
#define KPASSWORD_H
#include <QObject>
#include <kwidgetsaddons_export.h>

namespace KPassword
{
Q_NAMESPACE_EXPORT(KWIDGETSADDONS_EXPORT)

/**
 * This enum describe when the reveal password button is visible.
 * @since 6.0
 */
enum class RevealMode {
    /**
     * Display the button when entering a new password, but doesn't let you see a
     * previously entered password. This is the default.
     */
    OnlyNew,
    /**
     * Never display the reveal button.
     */
    Never,
    /**
     * Always display the reveal button. Usefull in a password manager for example.
     */
    Always,
};
Q_ENUM_NS(RevealMode)

}
#endif // KPASSWORD_H
