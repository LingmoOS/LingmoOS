/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2013 John Layt <jlayt@kde.org>
    SPDX-FileCopyrightText: 2010 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2009 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFORMATTEST_H
#define KFORMATTEST_H

#include <QObject>

class KFormatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void formatByteSize();
    void formatDuration();
    void formatDecimalDuration();
    void formatSpelloutDuration();
    void formatRelativeDate();
    void formatValue();
};

#endif // KFORMATTEST_H
