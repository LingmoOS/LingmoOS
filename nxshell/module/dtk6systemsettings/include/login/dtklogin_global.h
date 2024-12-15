// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#define DTK_NAMESPACE Dtk
#define DLOGIN_NAMESPACE Login
#define DTK_LOGIN_NAMESPACE DTK_NAMESPACE::DLOGIN_NAMESPACE

#define DLOGIN_USE_NAMESPACE using namespace DTK_LOGIN_NAMESPACE;

#define DLOGIN_BEGIN_NAMESPACE \
  namespace DTK_NAMESPACE {    \
  namespace DLOGIN_NAMESPACE {
#define DLOGIN_END_NAMESPACE \
  }                          \
  }
