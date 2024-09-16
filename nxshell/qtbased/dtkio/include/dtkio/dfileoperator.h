// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEOPERATOR_H
#define DFILEOPERATOR_H

#include <QUrl>
#include <QScopedPointer>

#include <DError>
#include <DExpected>

#include "dtkio_global.h"
#include "dtkiotypes.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFileOperatorPrivate;
class DFileOperator
{
public:
    explicit DFileOperator(const QUrl &url);
    ~DFileOperator();

    DTK_CORE_NAMESPACE::DExpected<QUrl> url() const;
    DTK_CORE_NAMESPACE::DExpected<bool> renameFile(const QString &newName);
    DTK_CORE_NAMESPACE::DExpected<bool> copyFile(const QUrl &destUrl, CopyFlag flag);
    DTK_CORE_NAMESPACE::DExpected<bool> moveFile(const QUrl &destUrl, CopyFlag flag);
    DTK_CORE_NAMESPACE::DExpected<bool> trashFile();
    DTK_CORE_NAMESPACE::DExpected<bool> deleteFile();
    DTK_CORE_NAMESPACE::DExpected<bool> touchFile();
    DTK_CORE_NAMESPACE::DExpected<bool> makeDirectory();
    DTK_CORE_NAMESPACE::DExpected<bool> createLink(const QUrl &link);
    DTK_CORE_NAMESPACE::DExpected<bool> restoreFile();

    DTK_CORE_NAMESPACE::DError lastError() const;

    [[nodiscard]] DFileFuture *renameFileAsync(const QString &newName, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *copyFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *moveFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *trashFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *deleteFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *restoreFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *touchFileAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *makeDirectoryAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *createLinkAsync(const QUrl &link, int ioPriority, QObject *parent = nullptr);

private:
    QScopedPointer<DFileOperatorPrivate> d;
};
DIO_END_NAMESPACE

#endif   // DFILEOPERATOR_H
