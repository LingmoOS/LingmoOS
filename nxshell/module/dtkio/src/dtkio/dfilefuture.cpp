// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfilefuture_p.h"
#include "dfilefuture.h"

DIO_BEGIN_NAMESPACE

DFuturePrivate::DFuturePrivate(DFileFuture *qq)
    : q(qq)
{
}

DFuturePrivate::~DFuturePrivate()
{
}

DFileFuture::DFileFuture(QObject *parent)
    : QObject(parent)
{
}

DFileFuture::~DFileFuture()
{
}

IOErrorCode DFileFuture::error() const
{
    return d->error;
}

bool DFileFuture::hasError() const
{
    return d->error != IOErrorCode::NoError;
}

void DFileFuture::setError(IOErrorCode error)
{
    d->error = error;
}

DIO_END_NAMESPACE
