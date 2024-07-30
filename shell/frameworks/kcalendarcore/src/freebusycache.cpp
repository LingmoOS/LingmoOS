/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusyCache abstract base class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "freebusycache.h"

using namespace KCalendarCore;

FreeBusyCache::~FreeBusyCache()
{
}

void FreeBusyCache::virtual_hook(int id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    Q_ASSERT(false);
}
