// Copyright (c) 2019 coolxv
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STUB_FUNCTION_H
#define STUB_FUNCTION_H

#include <QObject>
#include "compositing_manager.h"
#include "player_engine.h"

#pragma once

using namespace dmr;

namespace StubFunc{

bool isPadSystemTrue_stub();
bool isCompositedTrue_stub();
bool isCompositedFalse_stub();

PlayerEngine::CoreState playerEngineState_Paused_stub(void* obj);

void createSelectableLineEditOptionHandle_lambda_stub(void *obj);

}

#endif // STUB_FUNCTION_H
