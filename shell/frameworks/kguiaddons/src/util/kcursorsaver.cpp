/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2003 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/

#include "kcursorsaver.h"
#include "kguiaddons_debug.h"
#include <QGuiApplication>

class KCursorSaverPrivate
{
public:
    bool ownsCursor = true;
};

KCursorSaver::KCursorSaver(Qt::CursorShape shape)
    : d(new KCursorSaverPrivate)
{
    QGuiApplication::setOverrideCursor(QCursor(shape));
    d->ownsCursor = true;
}

KCursorSaver::KCursorSaver(KCursorSaver &&other)
    : d(other.d)
{
    *this = std::move(other);
}

KCursorSaver::~KCursorSaver()
{
    if (d->ownsCursor) {
        QGuiApplication::restoreOverrideCursor();
        delete d;
    }
}

void KCursorSaver::restoreCursor()
{
    if (!d->ownsCursor) {
        qCWarning(KGUIADDONS_LOG) << "This KCursorSaver doesn't own the cursor anymore, invalid call to restoreCursor().";
        return;
    }
    d->ownsCursor = false;
    QGuiApplication::restoreOverrideCursor();
}

KCursorSaver &KCursorSaver::operator=(KCursorSaver &&other)
{
    if (this != &other) {
        d->ownsCursor = false;
    }
    return *this;
}
