// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKGUI_GLOBAL_H
#define DTKGUI_GLOBAL_H

#pragma once

#include <dtkcore_global.h>

#define DGUI_NAMESPACE Gui
#define DTK_GUI_NAMESPACE DTK_NAMESPACE::Gui

#define DGUI_BEGIN_NAMESPACE namespace DTK_NAMESPACE { namespace DGUI_NAMESPACE {
#define DGUI_END_NAMESPACE }}
#define DGUI_USE_NAMESPACE using namespace DTK_GUI_NAMESPACE;

#endif // DTKGUI_GLOBAL_H
