/*
    This file is part of KNewStuffQuick.
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNSQ_QUICKSETTINGS_H
#define KNSQ_QUICKSETTINGS_H

#include <QObject>

namespace KNewStuffQuick
{
/**
 * An object for handling KNewStuff related settings which make sense to handle without
 * instantiating an engine (specifically, for now, whether or not this is allowed by
 * the user's Kiosk settings)
 * @since 5.81
 */
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool allowedByKiosk READ allowedByKiosk CONSTANT)
public:
    static Settings *instance();
    bool allowedByKiosk() const;

    /// @internal
    Settings()
    {
    }
};
}
#endif // KNSQ_QUICKSETTINGS_H
