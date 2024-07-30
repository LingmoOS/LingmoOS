/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "src/queue.h"

#include "WaitForIdleAndFinished.h"

using namespace ThreadWeaver;

WaitForIdleAndFinished::WaitForIdleAndFinished(Queue *weaver)
    : weaver_(weaver)
{
    Q_ASSERT(weaver);
    weaver_->finish();
    Q_ASSERT(weaver_->isIdle());
    weaver_->resume();
}

WaitForIdleAndFinished::~WaitForIdleAndFinished()
{
    weaver_->resume();
    weaver_->dequeue();
    weaver_->finish();
    weaver_->suspend();
    Q_ASSERT(weaver_->isIdle());
}

void WaitForIdleAndFinished::finish()
{
    weaver_->finish();
    Q_ASSERT(weaver_->isIdle());
}
