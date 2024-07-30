/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_CALFORMAT_P_H
#define KCALCORE_CALFORMAT_P_H

#include "exceptions.h"

namespace KCalendarCore
{
class CalFormatPrivate
{
public:
    virtual ~CalFormatPrivate();

    static QString mApplication; // Name of application, for creating unique ID strings
    static QString mProductId; // PRODID string to write to calendar files
    QString mLoadedProductId; // PRODID string loaded from calendar file
    std::unique_ptr<Exception> mException;
};
}

#endif
