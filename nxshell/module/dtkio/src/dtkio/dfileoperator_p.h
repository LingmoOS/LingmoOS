// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEOPERATOR_P_H
#define DFILEOPERATOR_P_H

#include <QUrl>
#include <QPointer>

#include <gio/gio.h>

#include <DError>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFileOperator;
class DFileOperatorPrivate : public QObject
{
public:
    explicit DFileOperatorPrivate(DFileOperator *q);
    ~DFileOperatorPrivate();

    struct NormalFutureAsyncOp
    {
        DFileFuture *future = nullptr;
        QPointer<DFileOperatorPrivate> me;
    };

    [[nodiscard]] DFileFuture *renameFileAsync(const QString &newName, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *copyFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *moveFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *trashFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *deleteFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *restoreFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *touchFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *makeDirectoryAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *createLinkAsync(const QUrl &link, int ioPriority, QObject *parent = nullptr);

    void setError(IOErrorCode code);

    DFileOperator *q = nullptr;
    QUrl url;
    DTK_CORE_NAMESPACE::DError error { IOErrorCode::NoError, IOErrorMessage(IOErrorCode::NoError) };

    // future callback
    static void renameAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void copyAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void trashAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void deleteAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void touchAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void makeDirAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void createLinkAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
};
DIO_END_NAMESPACE

#endif   // DFILEOPERATOR_P_H
