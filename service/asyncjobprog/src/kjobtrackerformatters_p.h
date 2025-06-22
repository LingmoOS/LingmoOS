/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1997, 2001 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999-2002 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2002 Lukas Tinkl <lukas@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2009, 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KJOBTRACKERFORMATTERS_P_H
#define KJOBTRACKERFORMATTERS_P_H

#include <QString>

namespace KJobTrackerFormatters
{
QString byteSize(double size);

QString daysDuration(int n);
QString hoursDuration(int n);
QString minutesDuration(int n);
QString secondsDuration(int n);
QString duration(unsigned long mSec);
}

#endif
