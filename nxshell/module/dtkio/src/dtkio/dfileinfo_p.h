// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEINFO_P_H
#define DFILEINFO_P_H

#include <QUrl>
#include <QPointer>

#include <gio/gio.h>

#include <DError>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFileInfo;
class DFileInfoPrivate : public QObject, public QSharedData
{
public:
    explicit DFileInfoPrivate(DFileInfo *q);
    DFileInfoPrivate(const DFileInfoPrivate &other);
    DFileInfoPrivate &operator=(DFileInfoPrivate &&other) noexcept;
    DFileInfoPrivate &operator=(const DFileInfoPrivate &other);
    ~DFileInfoPrivate();

    struct NormalFutureAsyncOp
    {
        DFileFuture *future = nullptr;
        QPointer<DFileInfoPrivate> me;
    };

    bool initQuerier();
    bool checkQuerier();
    void setError(IOErrorCode code);

    [[nodiscard]] DFileFuture *initQuerierAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *attributeAsync(AttributeID id, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *attributeAsync(const QByteArray &key, const AttributeType type, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *existsAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *refreshAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *permissionsAsync(int ioPriority, QObject *parent = nullptr);

    static void initQuerierAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    DFileInfo *q = nullptr;
    QUrl url;
    QByteArray queryAttributes = "*";
    FileQueryInfoFlags queryInfoFlags = FileQueryInfoFlags::TypeNone;
    bool querierInit = false;
    GFileInfo *gFileInfo = nullptr;

    DTK_CORE_NAMESPACE::DError error { IOErrorCode::NoError, IOErrorMessage(IOErrorCode::NoError) };
};
DIO_END_NAMESPACE

#endif   // DFILEINFO_P_H
