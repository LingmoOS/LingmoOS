// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILE_P_H
#define DFILE_P_H

#include <QUrl>
#include <QPointer>

#include <gio/gio.h>

#include <DError>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFile;
class DFilePrivate : public QObject
{
public:
    explicit DFilePrivate(DFile *q);
    ~DFilePrivate();

    typedef struct
    {
        DFileFuture *future = nullptr;
        QPointer<DFilePrivate> me;
    } NormalFutureAsyncOp;

    typedef struct
    {
        QByteArray data;
        DFileFuture *future = nullptr;
        QPointer<DFilePrivate> me;
    } ReadAllAsyncFutureOp;

    bool exists();
    bool checkOpenFlags(OpenFlags *mode);
    bool checkSeekable(GInputStream *stream, GSeekable **seekable);
    GInputStream *inputStream();
    GOutputStream *outputStream();

    // future
    [[nodiscard]] DFileFuture *openAsync(OpenFlags mode, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *closeAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *readAsync(size_t maxSize, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *readAllAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *writeAsync(const QByteArray &data, size_t len, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *writeAsync(const QByteArray &data, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *flushAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *sizeAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *existsAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *permissionsAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *setPermissionsAsync(Permissions permission, int ioPriority, QObject *parent = nullptr);

    static void permissionsAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void existsAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void sizeAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void flushAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void writeAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void readAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    void setError(IOErrorCode code);

    quint32 buildPermissions(Permissions permission);
    Permissions permissionsFromGFileInfo(GFileInfo *gfileinfo);

    DFile *q = nullptr;
    QUrl url;
    bool isOpen { false };
    DTK_CORE_NAMESPACE::DError error { IOErrorCode::NoError, IOErrorMessage(IOErrorCode::NoError) };

    GIOStream *ioStream = nullptr;
    GInputStream *iStream = nullptr;
    GOutputStream *oStream = nullptr;
};
DIO_END_NAMESPACE

#endif   // DFILE_P_H
