// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEINFO_H
#define DFILEINFO_H

#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QScopedPointer>

#include <DError>
#include <DExpected>

#include "dtkio_global.h"
#include "dtkiotypes.h"

DIO_BEGIN_NAMESPACE
class DFileFuture;
class DFileInfoPrivate;
class DFileInfo : public QObject
{
public:
    explicit DFileInfo(const QUrl &url, QObject *parent = nullptr);
    DFileInfo(const DFileInfo &other);
    DFileInfo(DFileInfo &&other) noexcept;
    DFileInfo &operator=(DFileInfo &&other) noexcept;
    DFileInfo &operator=(const DFileInfo &other);
    virtual ~DFileInfo() override;

    DTK_CORE_NAMESPACE::DExpected<QUrl> url() const;
    void setQueryAttributes(const QByteArray &attributes);
    void setQueryInfoFlags(const FileQueryInfoFlags flags);
    DTK_CORE_NAMESPACE::DExpected<QByteArray> queryAttributes() const;
    DTK_CORE_NAMESPACE::DExpected<FileQueryInfoFlags> queryInfoFlag() const;

    DTK_CORE_NAMESPACE::DExpected<bool> initQuerier();
    DTK_CORE_NAMESPACE::DExpected<QVariant> attribute(AttributeID id, bool *success = nullptr) const;
    DTK_CORE_NAMESPACE::DExpected<QVariant> attribute(const QByteArray &key, const AttributeType type) const;
    DTK_CORE_NAMESPACE::DExpected<bool> hasAttribute(AttributeID id) const;
    DTK_CORE_NAMESPACE::DExpected<bool> hasAttribute(const QByteArray &key) const;
    DTK_CORE_NAMESPACE::DExpected<bool> exists() const;
    DTK_CORE_NAMESPACE::DExpected<bool> refresh();
    DTK_CORE_NAMESPACE::DExpected<Permissions> permissions() const;

    [[nodiscard]] DFileFuture *initQuerierAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *attributeAsync(AttributeID id, int ioPriority, QObject *parent = nullptr) const;
    [[nodiscard]] DFileFuture *attributeAsync(const QByteArray &key, const AttributeType type, int ioPriority, QObject *parent = nullptr) const;
    [[nodiscard]] DFileFuture *existsAsync(int ioPriority, QObject *parent = nullptr) const;
    [[nodiscard]] DFileFuture *refreshAsync(int ioPriority, QObject *parent = nullptr);
    [[nodiscard]] DFileFuture *permissionsAsync(int ioPriority, QObject *parent = nullptr);

    DTK_CORE_NAMESPACE::DError lastError() const;

private:
    QSharedDataPointer<DFileInfoPrivate> d;
};
DIO_END_NAMESPACE

#endif   // DFILEINFO_H
