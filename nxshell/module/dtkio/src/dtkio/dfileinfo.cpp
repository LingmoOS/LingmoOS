// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfileinfo_p.h"
#include "dfileinfo.h"

#include <sys/stat.h>

#include <gio/gio.h>

#include "dfilehelper.h"
#include "dfilefuture.h"

DIO_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;

DFileInfoPrivate::DFileInfoPrivate(DFileInfo *fileInfo)
    : q(fileInfo)
{
}

DFileInfoPrivate::DFileInfoPrivate(const DFileInfoPrivate &other)
{
    q = other.q;
}

DFileInfoPrivate &DFileInfoPrivate::operator=(DFileInfoPrivate &&other) noexcept
{
    qSwap(q, other.q);
    return *this;
}

DFileInfoPrivate &DFileInfoPrivate::operator=(const DFileInfoPrivate &other)
{
    q = other.q;
    return *this;
}

DFileInfoPrivate::~DFileInfoPrivate()
{
}

bool DFileInfoPrivate::initQuerier()
{
    const char *attributes = queryAttributes;
    const FileQueryInfoFlags flags = queryInfoFlags;
    g_autoptr(GCancellable) cancellabel = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    GFileInfo *fileinfo = g_file_query_info(gfile, attributes, GFileQueryInfoFlags(flags), cancellabel, &gerror);
    if (gerror)
        setError(IOErrorCode(gerror->code));
    if (!fileinfo)
        return false;

    if (gFileInfo) {
        g_object_unref(gFileInfo);
        gFileInfo = nullptr;
    }
    gFileInfo = fileinfo;
    querierInit = true;

    return true;
}

bool DFileInfoPrivate::checkQuerier()
{
    if (!querierInit)
        return initQuerier();

    return true;
}

void DFileInfoPrivate::setError(IOErrorCode code)
{
    error.setErrorCode(code);
    error.setErrorMessage(IOErrorMessage(code));
}

DFileFuture *DFileInfoPrivate::initQuerierAsync(int ioPriority, QObject *parent)
{
    const char *attributes = q->queryAttributes()->toStdString().data();
    auto flag = q->queryInfoFlag();

    DFileFuture *future = new DFileFuture(parent);
    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->future = future;
    dataOp->me = this;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_file_query_info_async(gfile, attributes, GFileQueryInfoFlags(flag.value()), ioPriority, cancellable, initQuerierAsyncCallback, dataOp);
    return future;
}

DFileFuture *DFileInfoPrivate::attributeAsync(AttributeID id, int ioPriority, QObject *parent)
{
    DFileFuture *futureRet = new DFileFuture(parent);
    if (!querierInit) {
        DFileFuture *future = this->initQuerierAsync(ioPriority, nullptr);
        connect(future, &DFileFuture::finished, this, [=]() {
            if (!future->hasError()) {
                Q_EMIT futureRet->fileAttributeRequired(id, q->attribute(id).value());
                Q_EMIT futureRet->finished();
            }
            future->deleteLater();
        });
    }
    QTimer::singleShot(10, this, [=]() {
        Q_EMIT futureRet->fileAttributeRequired(id, q->attribute(id).value());
        Q_EMIT futureRet->finished();
    });
    return futureRet;
}

DFileFuture *DFileInfoPrivate::attributeAsync(const QByteArray &key, const AttributeType type, int ioPriority, QObject *parent)
{
    DFileFuture *futureRet = new DFileFuture(parent);
    if (!querierInit) {
        DFileFuture *future = this->initQuerierAsync(ioPriority, nullptr);
        connect(future, &DFileFuture::finished, this, [=]() {
            if (!future->hasError()) {
                Q_EMIT futureRet->fileAttributeRequired(key, q->attribute(key, type).value());
                Q_EMIT futureRet->finished();
            }
            future->deleteLater();
        });
    }
    QTimer::singleShot(10, this, [=]() {
        Q_EMIT futureRet->fileAttributeRequired(key, q->attribute(key, type).value());
        Q_EMIT futureRet->finished();
    });
    return futureRet;
}

DFileFuture *DFileInfoPrivate::existsAsync(int ioPriority, QObject *parent)
{
    DFileFuture *futureRet = new DFileFuture(parent);
    if (!querierInit) {
        DFileFuture *future = this->initQuerierAsync(ioPriority, nullptr);
        connect(future, &DFileFuture::finished, this, [=]() {
            if (!future->hasError()) {
                const bool exists = q->exists().value();
                Q_EMIT futureRet->fileExistsStateRequired(exists);
                Q_EMIT futureRet->finished();
            }
            future->deleteLater();
        });
    }
    QTimer::singleShot(10, this, [=]() {
        const bool exists = q->exists().value();
        Q_EMIT futureRet->fileExistsStateRequired(exists);
        Q_EMIT futureRet->finished();
    });
    return futureRet;
}

DFileFuture *DFileInfoPrivate::refreshAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = this->initQuerierAsync(ioPriority, parent);
    connect(future, &DFileFuture::finished, this, [=]() {
        Q_EMIT future->finished();
    });
    return future;
}

DFileFuture *DFileInfoPrivate::permissionsAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = this->initQuerierAsync(ioPriority, parent);
    connect(future, &DFileFuture::finished, this, [=]() {
        Q_EMIT future->filePermissionsRequired(q->permissions().value());
        Q_EMIT future->finished();
    });
    return future;
}

void DFileInfoPrivate::initQuerierAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    if (!data)
        return;

    DFileFuture *future = data->future;
    QPointer<DFileInfoPrivate> me = data->me;
    if (!future) {
        me = nullptr;
        g_free(data);
        return;
    }

    GFile *file = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    GFileInfo *fileinfo = g_file_query_info_finish(file, res, &gerror);

    if (gerror) {
        me->setError(IOErrorCode(gerror->code));
        future = nullptr;
        me = nullptr;
        g_free(data);

        return;
    }
    if (me->gFileInfo) {
        g_object_unref(me->gFileInfo);
        me->gFileInfo = nullptr;
    }

    me->gFileInfo = fileinfo;
    me->querierInit = true;

    Q_EMIT future->finished();

    future = nullptr;
    me = nullptr;
    g_free(data);
}

DFileInfo::DFileInfo(const QUrl &url, QObject *parent)
    : QObject(parent), d(new DFileInfoPrivate(this))
{
    d->url = url;
}

DFileInfo::DFileInfo(const DFileInfo &other)
{
    d = other.d;
}

DFileInfo::DFileInfo(DFileInfo &&other) noexcept
{
    d = other.d;
    other.d = nullptr;
}

DFileInfo &DFileInfo::operator=(DFileInfo &&other) noexcept
{
    qSwap(d, other.d);
    return *this;
}

DFileInfo &DFileInfo::operator=(const DFileInfo &other)
{
    d = other.d;
    return *this;
}

DFileInfo::~DFileInfo()
{
}

DExpected<QUrl> DFileInfo::url() const
{
    return d->url;
}

void DFileInfo::setQueryAttributes(const QByteArray &attributes)
{
    d->queryAttributes = attributes;
}

void DFileInfo::setQueryInfoFlags(const FileQueryInfoFlags flags)
{
    d->queryInfoFlags = flags;
}

DExpected<QByteArray> DFileInfo::queryAttributes() const
{
    return d->queryAttributes;
}

DExpected<FileQueryInfoFlags> DFileInfo::queryInfoFlag() const
{
    return d->queryInfoFlags;
}

DExpected<bool> DFileInfo::initQuerier()
{
    return d->initQuerier();
}

DExpected<QVariant> DFileInfo::attribute(AttributeID id, bool *success) const
{
    if (!(const_cast<DFileInfoPrivate *>(d.data()))->checkQuerier()) {
        if (success)
            *success = false;
        return DFileHelper::attributeDefaultValue(id);
    }

    const QVariant &value = DFileHelper::attributeFromInfo(id, d->gFileInfo, d->url);
    if (success)
        *success = value.isValid();

    return value;
}

DExpected<QVariant> DFileInfo::attribute(const QByteArray &key, const AttributeType type) const
{
    if (!(const_cast<DFileInfoPrivate *>(d.data()))->checkQuerier())
        return QVariant();

    const QVariant &value = DFileHelper::attribute(key, type, d->gFileInfo);
    return value;
}

DExpected<bool> DFileInfo::hasAttribute(AttributeID id) const
{
    if (!(const_cast<DFileInfoPrivate *>(d.data()))->checkQuerier())
        return false;

    const std::string &key = DFileHelper::attributeKey(id);
    return g_file_info_has_attribute(d->gFileInfo, key.c_str());
}

DExpected<bool> DFileInfo::hasAttribute(const QByteArray &key) const
{
    if (!(const_cast<DFileInfoPrivate *>(d.data()))->checkQuerier())
        return false;

    return g_file_info_has_attribute(d->gFileInfo, key);
}

DExpected<bool> DFileInfo::exists() const
{
    if (!(const_cast<DFileInfoPrivate *>(d.data()))->checkQuerier())
        return false;

    return g_file_info_get_file_type(d->gFileInfo) != G_FILE_TYPE_UNKNOWN;
}

DExpected<bool> DFileInfo::refresh()
{
    return d->initQuerier();
}

DExpected<Permissions> DFileInfo::permissions() const
{
    Permissions retValue = Permission::NoPermission;

    if (!(const_cast<DFileInfoPrivate *>(d.data()))->checkQuerier())
        return retValue;

    const QVariant &value = DFileHelper::attributeFromInfo(AttributeID::UnixMode, d->gFileInfo);
    if (!value.isValid())
        return retValue;
    const uint32_t stMode = value.toUInt();

    if ((stMode & S_IXUSR) == S_IXUSR) {
        retValue |= Permission::ExeOwner;
        retValue |= Permission::ExeUser;
    }
    if ((stMode & S_IWUSR) == S_IWUSR) {
        retValue |= Permission::WriteOwner;
        retValue |= Permission::WriteUser;
    }
    if ((stMode & S_IRUSR) == S_IRUSR) {
        retValue |= Permission::ReadOwner;
        retValue |= Permission::ReadUser;
    }

    if ((stMode & S_IXGRP) == S_IXGRP)
        retValue |= Permission::ExeGroup;
    if ((stMode & S_IWGRP) == S_IWGRP)
        retValue |= Permission::WriteGroup;
    if ((stMode & S_IRGRP) == S_IRGRP)
        retValue |= Permission::ReadGroup;

    if ((stMode & S_IXOTH) == S_IXOTH)
        retValue |= Permission::ExeOther;
    if ((stMode & S_IWOTH) == S_IWOTH)
        retValue |= Permission::WriteOther;
    if ((stMode & S_IROTH) == S_IROTH)
        retValue |= Permission::ReadOther;

    return retValue;
}

DFileFuture *DFileInfo::initQuerierAsync(int ioPriority, QObject *parent)
{
    return d->initQuerierAsync(ioPriority, parent);
}

DFileFuture *DFileInfo::attributeAsync(AttributeID id, int ioPriority, QObject *parent) const
{
    return (const_cast<DFileInfoPrivate *>(d.data()))->attributeAsync(id, ioPriority, parent);
}

DFileFuture *DFileInfo::attributeAsync(const QByteArray &key, const AttributeType type, int ioPriority, QObject *parent) const
{
    return (const_cast<DFileInfoPrivate *>(d.data()))->attributeAsync(key, type, ioPriority, parent);
}

DFileFuture *DFileInfo::existsAsync(int ioPriority, QObject *parent) const
{
    return (const_cast<DFileInfoPrivate *>(d.data()))->existsAsync(ioPriority, parent);
}

DFileFuture *DFileInfo::refreshAsync(int ioPriority, QObject *parent)
{
    return d->refreshAsync(ioPriority, parent);
}

DFileFuture *DFileInfo::permissionsAsync(int ioPriority, QObject *parent)
{
    return d->permissionsAsync(ioPriority, parent);
}

DError DFileInfo::lastError() const
{
    return d->error;
}

DIO_END_NAMESPACE
