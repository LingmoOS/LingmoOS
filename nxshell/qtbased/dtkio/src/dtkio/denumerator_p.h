// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DENUMERATOR_P_H
#define DENUMERATOR_P_H

#include <QUrl>
#include <QStack>
#include <QPointer>

#include <gio/gio.h>

#include <DError>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFileInfo;
class DEnumerator;
class DEnumeratorPrivate : public QObject
{
public:
    explicit DEnumeratorPrivate(DEnumerator *q);
    ~DEnumeratorPrivate();

    struct NormalFutureAsyncOp
    {
        DFileFuture *future = nullptr;
        QPointer<DEnumeratorPrivate> me;
    };

    void setError(IOErrorCode code);
    bool createEnumerator(const QUrl &url);
    bool checkFilter();

    [[nodiscard]] DFileFuture *createEnumeratorAsync(int ioPriority, QObject *parent = nullptr);
    static void createEnumeratorAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

public:
    DEnumerator *q = nullptr;
    QUrl url;
    QStringList nameFilters;
    DirFilters dirFilters;
    IteratorFlags iteratorflags;
    quint64 timeout = 0;
    DTK_CORE_NAMESPACE::DError error { IOErrorCode::NoError, IOErrorMessage(IOErrorCode::NoError) };
    bool enumeratorInited = false;

    QStack<GFileEnumerator *> stackEnumerator;
    QUrl nextUrl;
    DFileInfo *fileInfo = nullptr;
    QMap<QUrl, QSet<QString>> hideListMap;
};
DIO_END_NAMESPACE

#endif   // DENUMERATOR_P_H
