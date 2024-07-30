/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2003 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/

#ifndef KCURSORSAVER_H
#define KCURSORSAVER_H
#include <kguiaddons_export.h>

#include <QCursor>

class KCursorSaverPrivate;

/**
 * @class KCursorSaver kcursorsaver.h KCursorSaver
 *
 * @short Class to temporarily set a mouse cursor and restore the previous one on destruction
 *
 * Create a KCursorSaver object when you want to set the cursor.
 * As soon as it gets out of scope, it will restore the original
 * cursor.
 * @code
    KCursorSaver saver(Qt::WaitCursor);
    ... long-running operation here ...
   @endcode
 * @since 5.73
 */
class KGUIADDONS_EXPORT KCursorSaver
{
public:
    /// Creates a KCursorSaver, setting the mouse cursor to @p shape.
    explicit KCursorSaver(Qt::CursorShape shape);

    /// Move-constructs a KCursorSaver from other
    KCursorSaver(KCursorSaver &&other);

    /// restore the cursor
    ~KCursorSaver();

    /// call this to explicitly restore the cursor
    void restoreCursor();

    KCursorSaver &operator=(KCursorSaver &&other);

private:
    KCursorSaver(KCursorSaver &other) = delete;
    void operator=(const KCursorSaver &rhs) = delete;
    KCursorSaverPrivate *const d; ///< @internal
};

#endif
