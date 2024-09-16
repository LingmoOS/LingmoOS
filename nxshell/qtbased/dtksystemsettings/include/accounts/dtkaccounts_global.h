// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define DTK_NAMESPACE Dtk
#define DACCOUNTS_NAMESPACE Accounts
#define DTK_ACCOUNTS_NAMESPACE DTK_NAMESPACE::DACCOUNTS_NAMESPACE

#define DACCOUNTS_USE_NAMESPACE using namespace DTK_ACCOUNTS_NAMESPACE;

#define DACCOUNTS_BEGIN_NAMESPACE \
  namespace DTK_NAMESPACE {       \
  namespace DACCOUNTS_NAMESPACE {

#define DACCOUNTS_END_NAMESPACE \
  }                             \
  }
