/* -*- C++ -*-
    This file implements the Sequence class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#include "sequence.h"
#include "debuggingaids.h"
#include "dependencypolicy.h"
#include "managedjobpointer.h"
#include "queueapi.h"
#include "sequence_p.h"

namespace ThreadWeaver
{
Sequence::Sequence()
    : Collection(new Private::Sequence_Private())
{
}

Private::Sequence_Private *Sequence::d()
{
    return reinterpret_cast<Private::Sequence_Private *>(Collection::d());
}

const Private::Sequence_Private *Sequence::d() const
{
    return reinterpret_cast<const Private::Sequence_Private *>(Collection::d());
}

}
