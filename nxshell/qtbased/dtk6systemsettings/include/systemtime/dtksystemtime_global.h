// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DSYSTEMTIME_GLOBLE_H
#define DSYSTEMTIME_GLOBLE_H

#define DTK_NAMESPACE Dtk
#define DSYSTEMTIME_NAMESPACE SystemTime
#define DTK_SYSTEMTIME_NAMESPACE DTK_NAMESPACE::DSYSTIME_NAMESPACE

#define DSYSTEMTIME_USE_NAMESPACE using namespace DTK_SYSTEMTIME_NAMESPACE;

#define DSYSTEMTIME_BEGIN_NAMESPACE \
  namespace DTK_NAMESPACE {         \
  namespace DSYSTIME_NAMESPACE {
#define DSYSTEMTIME_END_NAMESPACE \
  }                               \
  }

#endif