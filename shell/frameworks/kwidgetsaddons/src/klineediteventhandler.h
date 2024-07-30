/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLINEEDITEVENTHANDLER_H
#define KLINEEDITEVENTHANDLER_H

#include <kwidgetsaddons_export.h>

class QObject;

/**
 * Event-handling utilities for QLineEdit instances.
 *
 * @since 6.0
 */
namespace KLineEditEventHandler
{
/** Do not propagate Return or Enter key presses in line edits.
 *
 *  QLineEdit::returnPressed() will still be emitted as normal, but the
 *  key event isn't propagated further up the widget hierarchy. This is
 *  useful to prevent closing a dialog for example.
 */
KWIDGETSADDONS_EXPORT void catchReturnKey(QObject *lineEdit);

/**
 * Handle drop events with URLs and replace the current line edit with the
 * dropped URL.
 */
KWIDGETSADDONS_EXPORT void handleUrlDrops(QObject *lineEdit);
};

#endif
