// Copyright (c) 2019 coolxv
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stub.h"
#include "stub_function.h"

namespace StubFunc{
bool isPadSystemTrue_stub()
{
    return true;
}
bool isCompositedTrue_stub()
{
    return true;
}
bool isCompositedFalse_stub()
{
    return false;
}

PlayerEngine::CoreState playerEngineState_Paused_stub(void* obj)
{
    PlayerEngine * engine = (PlayerEngine *)obj;
    return PlayerEngine::CoreState::Paused;
}

void createSelectableLineEditOptionHandle_lambda_stub(void *obj)
{
    qDebug() << "shortcut save path btn clicked.";
}
}
