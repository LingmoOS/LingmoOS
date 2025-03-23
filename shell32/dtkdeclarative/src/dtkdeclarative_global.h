// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkcore_global.h>

#include <QtCore/QMetaMethod>

#define DQUICK_NAMESPACE Quick
#define DTK_QUICK_NAMESPACE DTK_NAMESPACE::Quick

#define DQUICK_BEGIN_NAMESPACE namespace DTK_NAMESPACE { namespace DQUICK_NAMESPACE {
#define DQUICK_END_NAMESPACE }}
#define DQUICK_USE_NAMESPACE using namespace DTK_QUICK_NAMESPACE;
