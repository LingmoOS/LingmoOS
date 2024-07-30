/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_PERSON_P_H
#define KCALCORE_PERSON_P_H

class QString;

namespace KCalendarCore
{
QString fullNameHelper(const QString &name, const QString &email);
}

#endif
