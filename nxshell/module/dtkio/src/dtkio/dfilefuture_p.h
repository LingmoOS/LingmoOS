// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEFUTURE_P_H
#define DFILEFUTURE_P_H

#include <QUrl>

#include "dtkio_global.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFuturePrivate
{
public:
    explicit DFuturePrivate(DFileFuture *qq);
    ~DFuturePrivate();

    DFileFuture *q = nullptr;
    IOErrorCode error { IOErrorCode::NoError };
};
DIO_END_NAMESPACE

#endif   // DFILEFUTURE_P_H
