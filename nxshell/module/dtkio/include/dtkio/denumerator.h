// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DENUMERATOR_H
#define DENUMERATOR_H

#include <QUrl>
#include <QScopedPointer>

#include <DError>
#include <DExpected>

#include "dtkio_global.h"
#include "dtkiotypes.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFileInfo;
class DEnumeratorPrivate;
class DEnumerator
{
public:
    explicit DEnumerator(const QUrl &url);
    ~DEnumerator();

    DTK_CORE_NAMESPACE::DExpected<QUrl> url() const;

    DTK_CORE_NAMESPACE::DExpected<void> setNameFilters(const QStringList &nameFilters);
    DTK_CORE_NAMESPACE::DExpected<void> setDirFilters(DirFilters dirFilters);
    DTK_CORE_NAMESPACE::DExpected<void> setIteratorFlags(IteratorFlags flags);
    DTK_CORE_NAMESPACE::DExpected<QStringList> nameFilters() const;
    DTK_CORE_NAMESPACE::DExpected<DirFilters> dirFilters() const;
    DTK_CORE_NAMESPACE::DExpected<IteratorFlags> iteratorFlags() const;
    DTK_CORE_NAMESPACE::DExpected<void> setTimeout(quint64 timeout);
    DTK_CORE_NAMESPACE::DExpected<quint64> timeout() const;

    DTK_CORE_NAMESPACE::DExpected<bool> createEnumerator();
    [[nodiscard]] DFileFuture *createEnumeratorAsync(int ioPriority, QObject *parent = nullptr);

    DTK_CORE_NAMESPACE::DExpected<bool> hasNext() const;
    DTK_CORE_NAMESPACE::DExpected<QUrl> next() const;

    DTK_CORE_NAMESPACE::DExpected<quint64> fileCount();
    [[nodiscard]] DFileInfo *createFileInfo(QObject *parent = nullptr) const;

    DTK_CORE_NAMESPACE::DError lastError() const;

private:
    QScopedPointer<DEnumeratorPrivate> d;
};
DIO_END_NAMESPACE

#endif   // DENUMERATOR_H
