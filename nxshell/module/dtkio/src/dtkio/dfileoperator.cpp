// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfileoperator_p.h"
#include "dfileoperator.h"

#include <QtConcurrent/QtConcurrentRun>

#include <gio/gio.h>

#include "dfilehelper.h"
#include "dfileinfo.h"
#include "dfilefuture.h"

DIO_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;

DFileOperatorPrivate::DFileOperatorPrivate(DFileOperator *fileOperator)
    : q(fileOperator)
{
}

DFileOperatorPrivate::~DFileOperatorPrivate()
{
}

DFileFuture *DFileOperatorPrivate::renameFileAsync(const QString &newName, int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    // name must deep copy, otherwise name freed and crash
    g_autofree gchar *name = g_strdup(newName.toStdString().c_str());
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_file_set_display_name_async(gfile, name, ioPriority, cancellable, renameAsyncFutureCallback, dataOp);

    return future;
}

DFileFuture *DFileOperatorPrivate::copyFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    g_autoptr(GFile) gfileFrom = DFileHelper::fileNewForUrl(url);
    g_autoptr(GFile) gfileTo = DFileHelper::fileNewForUrl(destUrl);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_file_copy_async(gfileFrom, gfileTo, GFileCopyFlags(flag), ioPriority, cancellable, nullptr, nullptr, copyAsyncFutureCallback, dataOp);
}

DFileFuture *DFileOperatorPrivate::moveFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent)
{
    // g_file_move_async since 2.72
    Q_UNUSED(ioPriority);

    DFileFuture *future = new DFileFuture(parent);

    QPointer<DFileOperatorPrivate> me = this;
    QtConcurrent::run([=]() {
        q->moveFile(destUrl, flag);
        if (!me)
            return;
        Q_EMIT future->finished();
    });
    return future;
}

DFileFuture *DFileOperatorPrivate::trashFileAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_file_trash_async(gfile, ioPriority, cancellable, trashAsyncFutureCallback, dataOp);
}

DFileFuture *DFileOperatorPrivate::deleteFileAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_file_delete_async(gfile, ioPriority, cancellable, deleteAsyncFutureCallback, dataOp);
}

DFileFuture *DFileOperatorPrivate::restoreFileAsync(int ioPriority, QObject *parent)
{
    Q_UNUSED(ioPriority);

    DFileFuture *future = new DFileFuture(parent);

    QPointer<DFileOperatorPrivate> me = this;
    QtConcurrent::run([=]() {
        q->restoreFile();
        if (!me)
            return;
        Q_EMIT future->finished();
    });
    return future;
}

DFileFuture *DFileOperatorPrivate::touchFileAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_file_create_async(gfile, GFileCreateFlags::G_FILE_CREATE_NONE, ioPriority, cancellable, touchAsyncFutureCallback, dataOp);
}

DFileFuture *DFileOperatorPrivate::makeDirectoryAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_file_make_directory_async(gfile, ioPriority, cancellable, makeDirAsyncFutureCallback, dataOp);
}

DFileFuture *DFileOperatorPrivate::createLinkAsync(const QUrl &link, int ioPriority, QObject *parent)
{
    // g_file_make_symbolic_link_async since 2.72
    Q_UNUSED(ioPriority);

    DFileFuture *future = new DFileFuture(parent);

    QPointer<DFileOperatorPrivate> me = this;
    QtConcurrent::run([=]() {
        q->createLink(link);
        if (!me)
            return;
        Q_EMIT future->finished();
    });
    return future;
}

void DFileOperatorPrivate::setError(IOErrorCode code)
{
    error.setErrorCode(code);
    error.setErrorMessage(IOErrorMessage(code));
}

void DFileOperatorPrivate::renameAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFileOperatorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFile) gfileNew = g_file_set_display_name_finish(gfile, res, &gerror);
    Q_UNUSED(gfileNew);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

void DFileOperatorPrivate::copyAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFileOperatorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_file_copy_finish(gfile, res, &gerror);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

void DFileOperatorPrivate::trashAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFileOperatorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_file_trash_finish(gfile, res, &gerror);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

void DFileOperatorPrivate::deleteAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFileOperatorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_file_delete_finish(gfile, res, &gerror);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

void DFileOperatorPrivate::touchAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFileOperatorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFileOutputStream) stream = g_file_create_finish(gfile, res, &gerror);
    Q_UNUSED(stream);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

void DFileOperatorPrivate::makeDirAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFileOperatorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_file_make_directory_finish(gfile, res, &gerror);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

void DFileOperatorPrivate::createLinkAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
}

DFileOperator::DFileOperator(const QUrl &url)
    : d(new DFileOperatorPrivate(this))
{
    d->url = url;
}

DFileOperator::~DFileOperator()
{
}

DExpected<QUrl> DFileOperator::url() const
{
    return d->url;
}

DExpected<bool> DFileOperator::renameFile(const QString &newName)
{
    const QUrl &url = d->url;

    // name must deep copy, otherwise name freed and crash
    g_autofree gchar *name = g_strdup(newName.toStdString().c_str());

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFile) gfileNew = g_file_set_display_name(gfile, name, cancellable, &gerror);

    if (!gfileNew) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return true;
}

DExpected<bool> DFileOperator::copyFile(const QUrl &destUrl, CopyFlag flag)
{
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfileFrom = DFileHelper::fileNewForUrl(d->url);
    g_autoptr(GFile) gfileTo = DFileHelper::fileNewForUrl(destUrl);

    bool ret = g_file_copy(gfileFrom, gfileTo, GFileCopyFlags(flag), cancellable, nullptr, nullptr, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<bool> DFileOperator::moveFile(const QUrl &destUrl, CopyFlag flag)
{
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfileFrom = DFileHelper::fileNewForUrl(d->url);
    g_autoptr(GFile) gfileTo = DFileHelper::fileNewForUrl(destUrl);

    bool ret = g_file_move(gfileFrom, gfileTo, GFileCopyFlags(flag), cancellable, nullptr, nullptr, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<bool> DFileOperator::trashFile()
{
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    bool ret = g_file_trash(gfile, cancellable, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<bool> DFileOperator::deleteFile()
{
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    bool ret = g_file_delete(gfile, cancellable, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<bool> DFileOperator::touchFile()
{
    g_autoptr(GCancellable) cancallable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    // if file exist, return failed
    g_autoptr(GFileOutputStream) stream = g_file_create(gfile, GFileCreateFlags::G_FILE_CREATE_NONE, cancallable, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return stream != nullptr;
}

DExpected<bool> DFileOperator::makeDirectory()
{
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    bool ret = g_file_make_directory(gfile, cancellable, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<bool> DFileOperator::createLink(const QUrl &link)
{
    g_autoptr(GCancellable) cancellabel = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(link);

    const QString &linkValue = d->url.toLocalFile();
    bool ret = g_file_make_symbolic_link(gfile, linkValue.toStdString().c_str(), cancellabel, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<bool> DFileOperator::restoreFile()
{
    DFileInfo dfileinfo(d->url);
    dfileinfo.setQueryAttributes(DFileHelper::attributeKey(AttributeID::TrashOrigPath).c_str());
    bool succ = dfileinfo.initQuerier().value();
    if (!succ)
        return false;
    if (!dfileinfo.hasAttribute(AttributeID::TrashOrigPath))
        return false;
    const QString &path = dfileinfo.attribute(AttributeID::TrashOrigPath)->toString();
    if (path.isEmpty())
        return false;

    return moveFile(QUrl::fromLocalFile(path), CopyFlag::None);
}

DError DFileOperator::lastError() const
{
    return d->error;
}

DFileFuture *DFileOperator::renameFileAsync(const QString &newName, int ioPriority, QObject *parent)
{
    return d->renameFileAsync(newName, ioPriority, parent);
}

DFileFuture *DFileOperator::copyFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent)
{
    return d->copyFileAsync(destUrl, flag, ioPriority, parent);
}

DFileFuture *DFileOperator::moveFileAsync(const QUrl &destUrl, CopyFlag flag, int ioPriority, QObject *parent)
{
    return d->moveFileAsync(destUrl, flag, ioPriority, parent);
}

DFileFuture *DFileOperator::trashFileAsync(int ioPriority, QObject *parent)
{
    return d->trashFileAsync(ioPriority, parent);
}

DFileFuture *DFileOperator::deleteFileAsync(int ioPriority, QObject *parent)
{
    return d->deleteFileAsync(ioPriority, parent);
}

DFileFuture *DFileOperator::restoreFileAsync(int ioPriority, QObject *parent)
{
    return d->restoreFileAsync(ioPriority, parent);
}

DFileFuture *DFileOperator::touchFileAsync(int ioPriority, QObject *parent)
{
    return d->touchFileAsync(ioPriority, parent);
}

DFileFuture *DFileOperator::makeDirectoryAsync(int ioPriority, QObject *parent)
{
    return d->makeDirectoryAsync(ioPriority, parent);
}

DFileFuture *DFileOperator::createLinkAsync(const QUrl &link, int ioPriority, QObject *parent)
{
    return d->createLinkAsync(link, ioPriority, parent);
}

DIO_END_NAMESPACE
