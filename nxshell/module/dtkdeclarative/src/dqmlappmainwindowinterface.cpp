// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dqmlappmainwindowinterface.h"

DQUICK_BEGIN_NAMESPACE


void DQmlAppMainWindowInterface::initialize(QQmlApplicationEngine *engine)
{
    Q_UNUSED(engine);
}

void DQmlAppMainWindowInterface::finishedLoading(QQmlApplicationEngine *engine)
{
    Q_UNUSED(engine);
}


DQUICK_END_NAMESPACE
