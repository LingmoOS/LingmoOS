// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/qglobal.h>

#ifdef DS_LIB
#define DS_SHARE Q_DECL_EXPORT
#else
#define DS_SHARE Q_DECL_IMPORT
#endif

#define DS_NAMESPACE ds

#define DS_BEGIN_NAMESPACE namespace DS_NAMESPACE {
#define DS_END_NAMESPACE }
#define DS_USE_NAMESPACE using namespace DS_NAMESPACE;
