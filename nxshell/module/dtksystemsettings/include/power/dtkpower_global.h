// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define DTK_NAMESPACE Dtk
#define DPOWER_NAMESPACE Power
#define DTK_POWER_NAMESPACE DTK_NAMESPACE::DPOWER_NAMESPACE

#define DPOWER_USE_NAMESPACE using namespace DTK_POWER_NAMESPACE;

#define DPOWER_BEGIN_NAMESPACE \
  namespace DTK_NAMESPACE {    \
  namespace DPOWER_NAMESPACE {
#define DPOWER_END_NAMESPACE \
  }                          \
  }
