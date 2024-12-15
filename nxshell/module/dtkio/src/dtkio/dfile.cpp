// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfile_p.h"
#include "dfile.h"

#include <sys/stat.h>

#include <QtConcurrent/QtConcurrentRun>

#include <gio/gio.h>

#include "dfileerror.h"
#include "dfilehelper.h"
#include "dfilefuture.h"

using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;

DIO_BEGIN_NAMESPACE

DFilePrivate::DFilePrivate(DFile *file)
    : q(file)
{
}

DFilePrivate::~DFilePrivate()
{
}

bool DFilePrivate::exists()
{
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);

    return g_file_query_file_type(gfile, G_FILE_QUERY_INFO_NONE, nullptr) != G_FILE_TYPE_UNKNOWN;
}

bool DFilePrivate::checkOpenFlags(OpenFlags *modeIn)
{
    OpenFlags &mode = *modeIn;

    if (mode & OpenFlag::NewOnly) {
        if (exists()) {
            setError(IOErrorCode::OpenFlagError);
            return false;
        }
    }

    if (mode & OpenFlag::ExistingOnly) {
        if (!exists()) {
            setError(IOErrorCode::OpenFlagError);
            return false;
        }
    }
    if ((mode & OpenFlag::NewOnly) && (mode & OpenFlag::ExistingOnly)) {
        setError(IOErrorCode::OpenFlagError);
        return false;
    }

    // WriteOnly implies Truncate when ReadOnly, Append, and NewOnly are not set.
    if ((mode & OpenFlag::WriteOnly) && !(mode & (uint16_t(OpenFlag::ReadOnly) | uint16_t(OpenFlag::Append) | uint16_t(OpenFlag::NewOnly))))
        mode |= OpenFlag::Truncate;

    if (mode & (uint16_t(OpenFlag::Append) | uint16_t(OpenFlag::NewOnly)))
        mode |= OpenFlag::WriteOnly;

    if ((mode & (uint16_t(OpenFlag::ReadOnly) | uint16_t(OpenFlag::WriteOnly))) == 0) {
        setError(IOErrorCode::OpenFlagError);
        return false;
    }

    if ((mode & OpenFlag::ExistingOnly) && !(mode & (uint16_t(OpenFlag::ReadOnly) | uint16_t(OpenFlag::WriteOnly)))) {
        setError(IOErrorCode::OpenFlagError);
        return false;
    }

    return true;
}

bool DFilePrivate::checkSeekable(GInputStream *stream, GSeekable **seekable)
{
    Q_ASSERT(stream);
    Q_ASSERT(seekable);

    gboolean canSeek = G_IS_SEEKABLE(stream) /*&& g_seekable_can_seek(G_SEEKABLE(inputStream))*/;
    if (!canSeek)
        return false;

    *seekable = G_SEEKABLE(stream);
    if (!*seekable)
        return false;

    return true;
}

GInputStream *DFilePrivate::inputStream()
{
    if (iStream)
        return iStream;

    if (ioStream) {
        GInputStream *inputStream = g_io_stream_get_input_stream(ioStream);
        if (inputStream)
            return inputStream;
    }

    return nullptr;
}

GOutputStream *DFilePrivate::outputStream()
{
    if (oStream)
        return oStream;

    if (ioStream) {
        GOutputStream *outputStream = g_io_stream_get_output_stream(ioStream);
        if (outputStream)
            return outputStream;
    }

    return nullptr;
}

DFileFuture *DFilePrivate::openAsync(OpenFlags mode, int ioPriority, QObject *parent)
{
    Q_UNUSED(ioPriority);

    DFileFuture *future = new DFileFuture(parent);

    QPointer<DFilePrivate> me = this;
    QtConcurrent::run([=]() {
        q->open(mode);
        if (!me)
            return;
        Q_EMIT future->finished();
    });
    return future;
}

DFileFuture *DFilePrivate::closeAsync(int ioPriority, QObject *parent)
{
    Q_UNUSED(ioPriority);

    DFileFuture *future = new DFileFuture(parent);

    QPointer<DFilePrivate> me = this;
    QtConcurrent::run([=]() {
        q->close();
        if (!me)
            return;
        Q_EMIT future->finished();
    });
    return future;
}

DFileFuture *DFilePrivate::readAsync(size_t maxSize, int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    GInputStream *inputStream = this->inputStream();
    if (!inputStream) {
        setError(IOErrorCode::OpenFailed);
        return future;
    }

    QByteArray data;
    ReadAllAsyncFutureOp *dataOp = g_new0(ReadAllAsyncFutureOp, 1);
    dataOp->me = this;
    dataOp->future = future;
    dataOp->data = data;

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_input_stream_read_all_async(inputStream,
                                  &data,
                                  maxSize,
                                  ioPriority,
                                  cancellable,
                                  readAsyncFutureCallback,
                                  dataOp);
    return future;
}

DFileFuture *DFilePrivate::readAllAsync(int ioPriority, QObject *parent)
{
    return readAsync(G_MAXSSIZE, ioPriority, parent);
}

DFileFuture *DFilePrivate::writeAsync(const QByteArray &data, size_t len, int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    GOutputStream *outputStream = this->outputStream();
    if (!outputStream) {
        setError(IOErrorCode::OpenFailed);
        return future;
    }

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_output_stream_write_async(outputStream,
                                data,
                                len,
                                ioPriority,
                                cancellable,
                                writeAsyncFutureCallback,
                                dataOp);

    return future;
}

DFileFuture *DFilePrivate::writeAsync(const QByteArray &data, int ioPriority, QObject *parent)
{
    return writeAsync(data, strlen(data), ioPriority, parent);
}

DFileFuture *DFilePrivate::flushAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    GOutputStream *outputStream = this->outputStream();
    if (!outputStream) {
        setError(IOErrorCode::OpenFailed);
        return future;
    }

    NormalFutureAsyncOp *data = g_new0(NormalFutureAsyncOp, 1);
    data->me = this;
    data->future = future;

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_output_stream_flush_async(outputStream, ioPriority, cancellable, flushAsyncCallback, data);

    return future;
}

DFileFuture *DFilePrivate::sizeAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    NormalFutureAsyncOp *data = g_new0(NormalFutureAsyncOp, 1);
    data->me = this;
    data->future = future;

    g_autoptr(GFile) gfile = g_file_new_for_uri(url.toString().toStdString().c_str());
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::StandardSize);
    g_file_query_info_async(gfile, attributeKey.c_str(), G_FILE_QUERY_INFO_NONE, ioPriority, cancellable, sizeAsyncCallback, data);

    return future;
}

DFileFuture *DFilePrivate::existsAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    NormalFutureAsyncOp *data = g_new0(NormalFutureAsyncOp, 1);
    data->me = this;
    data->future = future;

    g_autoptr(GFile) gfile = g_file_new_for_uri(url.toString().toStdString().c_str());
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::StandardType);
    g_file_query_info_async(gfile, attributeKey.c_str(), G_FILE_QUERY_INFO_NONE, ioPriority, cancellable, existsAsyncCallback, data);

    return future;
}

DFileFuture *DFilePrivate::permissionsAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    NormalFutureAsyncOp *data = g_new0(NormalFutureAsyncOp, 1);
    data->me = this;
    data->future = future;

    g_autoptr(GFile) gfile = g_file_new_for_uri(url.toString().toStdString().c_str());
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::UnixMode);
    g_file_query_info_async(gfile, attributeKey.c_str(), G_FILE_QUERY_INFO_NONE, ioPriority, cancellable, permissionsAsyncCallback, data);

    return future;
}

DFileFuture *DFilePrivate::setPermissionsAsync(Permissions permission, int ioPriority, QObject *parent)
{
    Q_UNUSED(ioPriority)

    DFileFuture *future = new DFileFuture(parent);

    quint32 stMode = buildPermissions(permission);
    g_autoptr(GFile) gfile = g_file_new_for_uri(url.toString().toStdString().c_str());
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::UnixMode);

    QPointer<DFilePrivate> me = this;
    QtConcurrent::run([=, &gerror]() {
        g_file_set_attribute_uint32(gfile, attributeKey.c_str(), stMode, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellable, &gerror);
        if (!me)
            return;
        if (gerror)
            me->setError(IOErrorCode(gerror->code));
        Q_EMIT future->finished();
    });
    return future;
}

void DFilePrivate::permissionsAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    if (!data)
        return;

    QPointer<DFilePrivate> me = data->me;
    if (!me)
        return;
    DFileFuture *future = data->future;
    g_autoptr(GFile) gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFileInfo) gfileinfo = g_file_query_info_finish(gfile, res, &gerror);
    if (gerror) {
        me->setError(IOErrorCode(gerror->code));
        me = nullptr;
        future = nullptr;
        g_free(data);
        return;
    }
    auto permissions = data->me->permissionsFromGFileInfo(gfileinfo);
    Q_EMIT future->filePermissionsRequired(permissions);
    Q_EMIT future->finished();

    me = nullptr;
    future = nullptr;
    g_free(data);
}

void DFilePrivate::existsAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    if (!data)
        return;

    QPointer<DFilePrivate> me = data->me;
    if (!me)
        return;
    DFileFuture *future = data->future;
    g_autoptr(GFile) gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFileInfo) gfileinfo = g_file_query_info_finish(gfile, res, &gerror);
    if (gerror) {
        me->setError(IOErrorCode(gerror->code));
        me = nullptr;
        future = nullptr;
        g_free(data);
        return;
    }

    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::StandardType);
    const quint32 exists = g_file_info_get_attribute_uint32(gfileinfo, attributeKey.c_str());

    Q_EMIT future->fileExistsStateRequired(exists != G_FILE_TYPE_UNKNOWN);
    Q_EMIT future->finished();

    me = nullptr;
    future = nullptr;
    g_free(data);
}

void DFilePrivate::sizeAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    if (!data)
        return;

    QPointer<DFilePrivate> me = data->me;
    if (!me)
        return;
    DFileFuture *future = data->future;
    g_autoptr(GFile) gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GFileInfo) gfileinfo = g_file_query_info_finish(gfile, res, &gerror);
    if (gerror) {
        me->setError(IOErrorCode(gerror->code));
        me = nullptr;
        future = nullptr;
        g_free(data);
        return;
    }

    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::StandardSize);
    const quint64 size = g_file_info_get_attribute_uint64(gfileinfo, attributeKey.c_str());

    Q_EMIT future->fileSizeRequired(size);
    Q_EMIT future->finished();

    me = nullptr;
    future = nullptr;
    g_free(data);
}

void DFilePrivate::flushAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFilePrivate> me = data->me;
    DFileFuture *future = data->future;
    g_autoptr(GOutputStream) stream = G_OUTPUT_STREAM(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    g_output_stream_flush_finish(stream, res, &gerror);
    if (gerror) {
        me->setError(IOErrorCode(gerror->code));
        me = nullptr;
        future = nullptr;
        g_free(data);
        return;
    }
    Q_EMIT future->finished();

    me = nullptr;
    future = nullptr;
    g_free(data);
}

void DFilePrivate::writeAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DFilePrivate> me = data->me;
    DFileFuture *future = data->future;
    GOutputStream *stream = (GOutputStream *)(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    gssize size = g_output_stream_write_finish(stream, res, &gerror);
    if (gerror) {
        me->setError(IOErrorCode(gerror->code));
        me = nullptr;
        future = nullptr;
        g_free(data);
        return;
    }
    Q_EMIT future->dataWrited(size);
    Q_EMIT future->finished();

    me = nullptr;
    future = nullptr;
    g_free(data);
}

void DFilePrivate::readAsyncFutureCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    ReadAllAsyncFutureOp *data = static_cast<ReadAllAsyncFutureOp *>(userData);
    GInputStream *stream = (GInputStream *)(sourceObject);
    QPointer<DFilePrivate> me = data->me;
    DFileFuture *future = data->future;

    g_autoptr(GError) gerror = nullptr;
    gsize size = 0;
    bool succ = g_input_stream_read_all_finish(stream, res, &size, &gerror);
    if (!succ || gerror) {
        future->setError(IOErrorCode(gerror->code));
        me->setError(IOErrorCode(gerror->code));
    }

    Q_EMIT future->dataReaded(data->data);
    Q_EMIT future->finished();

    data->future = nullptr;
    data->me = nullptr;
    g_free(data);
}

void DFilePrivate::setError(IOErrorCode code)
{
    error.setErrorCode(code);
    error.setErrorMessage(IOErrorMessage(code));
}

quint32 DFilePrivate::buildPermissions(Permissions permission)
{
    quint32 stMode = 0000;
    if (permission.testFlag(Permission::ExeOwner) | permission.testFlag(Permission::ExeUser))
        stMode |= S_IXUSR;
    if (permission.testFlag(Permission::WriteOwner) | permission.testFlag(Permission::WriteUser))
        stMode |= S_IWUSR;
    if (permission.testFlag(Permission::ReadOwner) | permission.testFlag(Permission::ReadUser))
        stMode |= S_IRUSR;

    if (permission.testFlag(Permission::ExeGroup))
        stMode |= S_IXGRP;
    if (permission.testFlag(Permission::WriteGroup))
        stMode |= S_IWGRP;
    if (permission.testFlag(Permission::ReadGroup))
        stMode |= S_IRGRP;

    if (permission.testFlag(Permission::ExeOther))
        stMode |= S_IXOTH;
    if (permission.testFlag(Permission::WriteOther))
        stMode |= S_IWOTH;
    if (permission.testFlag(Permission::ReadOther))
        stMode |= S_IROTH;
    return stMode;
}

Permissions DFilePrivate::permissionsFromGFileInfo(GFileInfo *gfileinfo)
{
    Permissions retValue = Permission::NoPermission;
    if (!gfileinfo)
        return retValue;

    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::UnixMode);
    const quint32 &stMode = g_file_info_get_attribute_uint32(gfileinfo, attributeKey.c_str());
    if (!stMode)
        return retValue;

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

DFile::DFile(const QUrl &url)
    : d(new DFilePrivate(this))
{
    d->url = url;
}

DFile::~DFile()
{
}

DExpected<QUrl> DFile::url() const
{
    return d->url;
}

DExpected<bool> DFile::open(OpenFlags mode)
{
    if (d->isOpen) {
        d->setError(IOErrorCode::FileAlreadyOpened);
        return DUnexpected<> { d->error };
    }

    if (!d->checkOpenFlags(&mode)) {
        return DUnexpected<> { d->error };
    }

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);
    g_autoptr(GError) gerror = nullptr;
    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    if (mode & OpenFlag::ReadOnly && !(mode & OpenFlag::WriteOnly)) {
        if (!exists()) {
            d->setError(IOErrorCode::FileNotFound);
            return DUnexpected<> { d->error };
        }
        d->iStream = (GInputStream *)g_file_read(gfile, cancellable, &gerror);
        if (gerror)
            d->setError(IOErrorCode(gerror->code));

        if (!d->iStream) {
            return DUnexpected<> { d->error };
        }
        return true;
    } else if (mode & OpenFlag::WriteOnly && !(mode & OpenFlag::ReadOnly)) {
        if (mode & OpenFlag::NewOnly) {
            d->oStream = (GOutputStream *)g_file_create(gfile, G_FILE_CREATE_NONE, cancellable, &gerror);
            if (gerror)
                d->setError(IOErrorCode(gerror->code));

            if (!d->oStream) {
                return DUnexpected<> { d->error };
            }
        } else if (mode & OpenFlag::Append) {
            d->oStream = (GOutputStream *)g_file_append_to(gfile, G_FILE_CREATE_NONE, cancellable, &gerror);
            if (gerror)
                d->setError(IOErrorCode(gerror->code));

            if (!d->oStream) {
                return DUnexpected<> { d->error };
            }
        } else {
            d->oStream = (GOutputStream *)g_file_replace(gfile, nullptr, false, G_FILE_CREATE_NONE, cancellable, &gerror);
            if (gerror)
                d->setError(IOErrorCode(gerror->code));

            if (!d->oStream) {
                return DUnexpected<> { d->error };
            }
        }

        return true;
    } else if (mode & OpenFlag::ReadOnly && mode & OpenFlag::WriteOnly) {
        if (mode & OpenFlag::NewOnly) {
            d->ioStream = (GIOStream *)g_file_create_readwrite(gfile, G_FILE_CREATE_NONE, cancellable, &gerror);
            if (gerror)
                d->setError(IOErrorCode(gerror->code));

            if (!d->ioStream) {
                return DUnexpected<> { d->error };
            }
        } else if (mode & OpenFlag::ExistingOnly) {
            d->ioStream = (GIOStream *)g_file_open_readwrite(gfile, cancellable, &gerror);
            if (gerror)
                d->setError(IOErrorCode(gerror->code));

            if (!d->ioStream) {
                return DUnexpected<> { d->error };
            }
        } else {
            d->ioStream = (GIOStream *)g_file_replace_readwrite(gfile, nullptr, false, G_FILE_CREATE_NONE, cancellable, &gerror);
            if (gerror)
                d->setError(IOErrorCode(gerror->code));

            if (!d->ioStream) {
                return DUnexpected<> { d->error };
            }
        }
        return true;
    } else {
        d->ioStream = (GIOStream *)g_file_replace_readwrite(gfile, nullptr, false, G_FILE_CREATE_NONE, cancellable, &gerror);
        if (gerror)
            d->setError(IOErrorCode(gerror->code));

        if (!d->ioStream) {
            return DUnexpected<> { d->error };
        }
        return true;
    }
}

DExpected<bool> DFile::close()
{
    gboolean res { FALSE };
    g_autoptr(GError) gerror = nullptr;   // TODO: get error
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    if (d->iStream) {
        if (!g_input_stream_is_closed(d->iStream))
            res = g_input_stream_close(d->iStream, cancellable, &gerror);
        g_object_unref(d->iStream);
        d->iStream = nullptr;
    }
    if (d->oStream) {
        if (!g_output_stream_is_closed(d->oStream))
            res = g_output_stream_close(d->oStream, cancellable, &gerror);
        g_object_unref(d->oStream);
        d->oStream = nullptr;
    }
    if (d->ioStream) {
        if (!g_io_stream_is_closed(d->ioStream))
            res = g_io_stream_close(d->ioStream, cancellable, &gerror);
        g_object_unref(d->ioStream);
        d->ioStream = nullptr;
    }

    if (gerror && !res) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return true;
}

bool DFile::isOpen() const
{
    return d->isOpen;
}

DExpected<size_t> DFile::read(QByteArray *data, size_t maxSize)
{
    Q_ASSERT(data);

    GInputStream *inputStream = d->inputStream();
    if (!inputStream) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    char cdata[maxSize];
    memset(&cdata, 0, maxSize);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    gssize readSize = g_input_stream_read(inputStream,
                                          cdata,
                                          maxSize,
                                          cancellable,
                                          &gerror);
    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    data->append(cdata, static_cast<int>(readSize));
    return readSize;
}

DExpected<QByteArray> DFile::readAll()
{
    GInputStream *inputStream = d->inputStream();
    if (!inputStream) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    QByteArray dataRet;

    const gsize size = 8192;

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    while (true) {
        gsize bytesRead;
        char data[size];
        memset(data, 0, size);

        gboolean read = g_input_stream_read_all(inputStream,
                                                data,
                                                size,
                                                &bytesRead,
                                                cancellable,
                                                &gerror);
        if (!read || gerror) {
            if (gerror) {
                d->setError(IOErrorCode(gerror->code));
                return DUnexpected<> { d->error };
            }
            break;
        }
        if (bytesRead == 0)
            break;

        dataRet.append(data, static_cast<int>(bytesRead));
    }

    return dataRet;
}

DExpected<size_t> DFile::write(const QByteArray &data, size_t len)
{
    GOutputStream *outputStream = d->outputStream();
    if (!outputStream) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    gssize write = g_output_stream_write(outputStream,
                                         data.constData(),
                                         len,
                                         cancellable,
                                         &gerror);
    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return write;
}

DExpected<size_t> DFile::write(const QByteArray &data)
{
    return write(data, static_cast<size_t>(data.length()));
}

DExpected<bool> DFile::seek(ssize_t pos, SeekType type) const
{
    GInputStream *inputStream = d->inputStream();
    if (!inputStream) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    // TODO: seems g_seekable_can_seek only support local file, survey after.
    GSeekable *seekable { nullptr };
    if (!d->checkSeekable(inputStream, &seekable)) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    bool ret = false;
    GSeekType gtype = G_SEEK_CUR;
    switch (type) {
    case SeekType::Begin:
        gtype = G_SEEK_SET;
        break;
    case SeekType::End:
        gtype = G_SEEK_END;
        break;

    default:
        break;
    }

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    ret = g_seekable_seek(seekable, pos, gtype, cancellable, &gerror);
    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return ret;
}

DExpected<ssize_t> DFile::pos() const
{
    GInputStream *inputStream = d->inputStream();
    if (!inputStream) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    GSeekable *seekable { nullptr };
    if (!d->checkSeekable(inputStream, &seekable)) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    goffset pos = g_seekable_tell(seekable);

    return pos;
}

DExpected<bool> DFile::flush()
{
    GOutputStream *outputStream = d->outputStream();
    if (!outputStream) {
        d->setError(IOErrorCode::OpenFailed);
        return DUnexpected<> { d->error };
    }

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    gboolean ret = g_output_stream_flush(outputStream, cancellable, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }
    return ret;
}

DExpected<size_t> DFile::size() const
{
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::StandardSize);
    g_autoptr(GFileInfo) fileInfo = g_file_query_info(gfile, attributeKey.c_str(), G_FILE_QUERY_INFO_NONE, cancellable, &gerror);

    if (gerror && !fileInfo) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    const QVariant &value = DFileHelper::attributeFromInfo(AttributeID::StandardSize, fileInfo);
    if (!value.isValid())
        return DFileHelper::attributeDefaultValue(AttributeID::StandardSize).toLongLong();

    return value.toULongLong();
}

bool DFile::exists() const
{
    return d->exists();
}

DExpected<Permissions> DFile::permissions() const
{
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::UnixMode);
    g_autoptr(GFileInfo) fileInfo = g_file_query_info(gfile, attributeKey.c_str(), G_FILE_QUERY_INFO_NONE, cancellable, &gerror);

    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }

    return d->permissionsFromGFileInfo(fileInfo);
}

DExpected<bool> DFile::setPermissions(Permissions permission)
{
    uint32_t stMode = 0000;
    if (permission.testFlag(Permission::ExeOwner) | permission.testFlag(Permission::ExeUser))
        stMode |= S_IXUSR;
    if (permission.testFlag(Permission::WriteOwner) | permission.testFlag(Permission::WriteUser))
        stMode |= S_IWUSR;
    if (permission.testFlag(Permission::ReadOwner) | permission.testFlag(Permission::ReadUser))
        stMode |= S_IRUSR;

    if (permission.testFlag(Permission::ExeGroup))
        stMode |= S_IXGRP;
    if (permission.testFlag(Permission::WriteGroup))
        stMode |= S_IWGRP;
    if (permission.testFlag(Permission::ReadGroup))
        stMode |= S_IRGRP;

    if (permission.testFlag(Permission::ExeOther))
        stMode |= S_IXOTH;
    if (permission.testFlag(Permission::WriteOther))
        stMode |= S_IWOTH;
    if (permission.testFlag(Permission::ReadOther))
        stMode |= S_IROTH;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    const std::string &attributeKey = DFileHelper::attributeKey(AttributeID::UnixMode);
    bool succ = DFileHelper::setAttribute(gfile, attributeKey.c_str(), AttributeType::TypeUInt32, stMode, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellable, &gerror);
    if (gerror) {
        d->setError(IOErrorCode(gerror->code));
        return DUnexpected<> { d->error };
    }
    return succ;
}

DExpected<bool> DFile::setAttribute(AttributeID id, const QVariant &value)
{
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);
    const std::string &key = DFileHelper::attributeKey(id);
    const AttributeType &type = DFileHelper::attributeType(id);

    g_autoptr(GCancellable) cancellabel = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    if (key.empty() || type == AttributeType::TypeInvalid) {
        d->setError(IOErrorCode::InvalidArgument);
        return DUnexpected<> { d->error };
    }

    bool succ = DFileHelper::setAttribute(gfile, key.c_str(), type, value, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellabel, &gerror);
    return succ;
}

DExpected<bool> DFile::setAttribute(const QByteArray &key, const QVariant &value, const AttributeType type, const FileQueryInfoFlags flag)
{
    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(d->url);
    g_autoptr(GCancellable) cancellabel = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    bool succ = DFileHelper::setAttribute(gfile, key, type, value, GFileQueryInfoFlags(flag), cancellabel, &gerror);
    return succ;
}

DFileFuture *DFile::openAsync(OpenFlags mode, int ioPriority, QObject *parent)
{
    return d->openAsync(mode, ioPriority, parent);
}

DFileFuture *DFile::closeAsync(int ioPriority, QObject *parent)
{
    return d->closeAsync(ioPriority, parent);
}

DFileFuture *DFile::readAsync(size_t maxSize, int ioPriority, QObject *parent)
{
    return d->readAsync(maxSize, ioPriority, parent);
}

DFileFuture *DFile::readAllAsync(int ioPriority, QObject *parent)
{
    return d->readAllAsync(ioPriority, parent);
}

DFileFuture *DFile::writeAsync(const QByteArray &data, size_t len, int ioPriority, QObject *parent)
{
    return d->writeAsync(data, len, ioPriority, parent);
}

DFileFuture *DFile::writeAsync(const QByteArray &data, int ioPriority, QObject *parent)
{
    return d->writeAsync(data, ioPriority, parent);
}

DFileFuture *DFile::flushAsync(int ioPriority, QObject *parent)
{
    return d->flushAsync(ioPriority, parent);
}

DFileFuture *DFile::sizeAsync(int ioPriority, QObject *parent)
{
    return d->sizeAsync(ioPriority, parent);
}

DFileFuture *DFile::existsAsync(int ioPriority, QObject *parent)
{
    return d->existsAsync(ioPriority, parent);
}

DFileFuture *DFile::permissionsAsync(int ioPriority, QObject *parent)
{
    return d->permissionsAsync(ioPriority, parent);
}

DFileFuture *DFile::setPermissionsAsync(Permissions permission, int ioPriority, QObject *parent)
{
    return d->setPermissionsAsync(permission, ioPriority, parent);
}

DError DFile::lastError() const
{
    return d->error;
}

DIO_END_NAMESPACE
