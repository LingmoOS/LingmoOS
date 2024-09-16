// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILE_H
#define DFILE_H

#include <QObject>
#include <QUrl>
#include <QScopedPointer>

#include <DError>
#include <DExpected>

#include "dtkio_global.h"
#include "dtkiotypes.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFilePrivate;
class DFile
{
public:
    explicit DFile(const QUrl &url);
    ~DFile();

    DTK_CORE_NAMESPACE::DExpected<QUrl> url() const;
    DTK_CORE_NAMESPACE::DExpected<bool> open(OpenFlags mode);
    DTK_CORE_NAMESPACE::DExpected<bool> close();
    bool isOpen() const;

    DTK_CORE_NAMESPACE::DExpected<size_t> read(QByteArray *data, size_t maxSize);
    DTK_CORE_NAMESPACE::DExpected<QByteArray> readAll();
    DTK_CORE_NAMESPACE::DExpected<size_t> write(const QByteArray &data, size_t len);
    DTK_CORE_NAMESPACE::DExpected<size_t> write(const QByteArray &data);

    DTK_CORE_NAMESPACE::DExpected<bool> seek(ssize_t pos, SeekType type = SeekType::Begin) const;
    DTK_CORE_NAMESPACE::DExpected<ssize_t> pos() const;
    DTK_CORE_NAMESPACE::DExpected<bool> flush();
    DTK_CORE_NAMESPACE::DExpected<size_t> size() const;
    bool exists() const;

    DTK_CORE_NAMESPACE::DExpected<Permissions> permissions() const;
    DTK_CORE_NAMESPACE::DExpected<bool> setPermissions(Permissions permission);
    DTK_CORE_NAMESPACE::DExpected<bool> setAttribute(AttributeID id, const QVariant &value);
    DTK_CORE_NAMESPACE::DExpected<bool> setAttribute(const QByteArray &key, const QVariant &value, const AttributeType type, const FileQueryInfoFlags flag = FileQueryInfoFlags::TypeNoFollowSymlinks);

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

    DTK_CORE_NAMESPACE::DError lastError() const;

private:
    QScopedPointer<DFilePrivate> d;
};
DIO_END_NAMESPACE

#endif   // DFILE_H
