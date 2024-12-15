// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DTKRFMANAGER_GLOBAL_H
#define DTKRFMANAGER_GLOBAL_H

#define DTK_NAMESPACE Dtk
#define DRFMGR_NAMESPACE RfMgr
#define DTK_RFMGR_NAMESPACE DTK_NAMESPACE::DRFMGR_NAMESPACE

#define DRFMGR_USE_NAMESPACE using namespace DTK_RFMGR_NAMESPACE;

#define DRFMGR_BEGIN_NAMESPACE \
  namespace DTK_NAMESPACE {    \
  namespace DRFMGR_NAMESPACE {

#define DRFMGR_END_NAMESPACE \
  }                          \
  }

#endif // DTKRFMANAGER_GLOBAL_H
