// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEFUTURE_H
#define DFILEFUTURE_H

#include <QObject>
#include <QScopedPointer>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFuturePrivate;
class DFileFuture : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DFileFuture)
public:
    explicit DFileFuture(QObject *parent = nullptr);
    virtual ~DFileFuture() override;

    // TODO: add control interfaces

    IOErrorCode error() const;
    bool hasError() const;
    void setError(IOErrorCode error);

Q_SIGNALS:
    void finished();
    // TODO: refactor follow signals
    // TODO: void progressChanged(qint64 current, qint64 total);
    void fileAttributeRequired(AttributeID id, const QVariant &value);
    void fileAttributeRequired(const QByteArray &key, const QVariant &value);
    void fileExistsStateRequired(const bool exists);
    void filePermissionsRequired(const Permissions permissions);
    void fileSizeRequired(const quint64 &size);
    void dataReaded(const QByteArray &data);
    void dataWrited(const qint64 &size);

private:
    QScopedPointer<DFuturePrivate> d;
};
DIO_END_NAMESPACE

#endif   // DFILEFUTURE_H
