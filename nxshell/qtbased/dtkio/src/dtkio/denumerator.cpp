// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "denumerator_p.h"
#include "denumerator.h"

#include <QtConcurrent/QtConcurrentRun>

#include <gio/gio.h>

#include "dfilehelper.h"
#include "dfileinfo.h"
#include "dfilefuture.h"

#define FILE_DEFAULT_ATTRIBUTES "standard::*,etag::*,id::*,access::*,mountable::*,time::*,unix::*,dos::*,\
owner::*,thumbnail::*,preview::*,filesystem::*,gvfs::*,selinux::*,trash::*,recent::*,metadata::*"

DIO_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;

DEnumeratorPrivate::DEnumeratorPrivate(DEnumerator *enumerator)
    : q(enumerator)
{
}

DEnumeratorPrivate::~DEnumeratorPrivate()
{
    if (fileInfo && !fileInfo->parent()) {
        delete fileInfo;
        fileInfo = nullptr;
    }
}

void DEnumeratorPrivate::setError(IOErrorCode code)
{
    error.setErrorCode(code);
    error.setErrorMessage(IOErrorMessage(code));
}

bool DEnumeratorPrivate::createEnumerator(const QUrl &url)
{
    const bool enumLinks = iteratorflags & IteratorFlag::FollowSymlinks;

    g_autoptr(GFile) gfile = g_file_new_for_uri(url.toString().toStdString().c_str());

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;

    GFileEnumerator *genumerator = nullptr;
    if (timeout != 0) {
        QScopedPointer<TimeoutHelper> blocker(new TimeoutHelper(timeout));

        QtConcurrent::run([gfile, enumLinks, cancellable, &gerror, &blocker, &genumerator]() {
            genumerator = g_file_enumerate_children(gfile,
                                                    FILE_DEFAULT_ATTRIBUTES,
                                                    enumLinks ? G_FILE_QUERY_INFO_NONE : G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                    cancellable,
                                                    &gerror);
            if (genumerator)
                blocker->exit();
        });
        auto ret = blocker->exec();
        if (ret == TimeoutHelper::Timeout) {
            if (cancellable)
                g_cancellable_cancel(cancellable);
        }
    } else {
        genumerator = g_file_enumerate_children(gfile,
                                                FILE_DEFAULT_ATTRIBUTES,
                                                enumLinks ? G_FILE_QUERY_INFO_NONE : G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                cancellable,
                                                &gerror);
    }

    bool ret = true;
    if (!genumerator || gerror) {
        if (gerror) {
            setError(IOErrorCode(gerror->code));
        }
        ret = false;
    } else {
        stackEnumerator.push_back(genumerator);
    }

    enumeratorInited = true;
    return ret;
}

bool DEnumeratorPrivate::checkFilter()
{
    if (dirFilters == DirFilters(DirFilter::NoFilter))
        return true;

    if (!fileInfo)
        return false;

    const bool isDir = fileInfo->attribute(AttributeID::StandardIsDir)->toBool();
    if ((dirFilters & DirFilter::AllDirs) == DirFilters(DirFilter::AllDirs)) {   // all dir, no apply filters rules
        if (isDir)
            return true;
    }

    // dir filter
    bool ret = true;

    const bool readable = fileInfo->attribute(AttributeID::AccessCanRead)->toBool();
    const bool writable = fileInfo->attribute(AttributeID::AccessCanWrite)->toBool();
    const bool executable = fileInfo->attribute(AttributeID::AccessCanExecute)->toBool();

    auto checkRWE = [&]() -> bool {
        if ((dirFilters & DirFilter::Readable) == DirFilters(DirFilter::Readable)) {
            if (!readable)
                return false;
        }
        if ((dirFilters & DirFilter::Writable) == DirFilters(DirFilter::Writable)) {
            if (!writable)
                return false;
        }
        if ((dirFilters & DirFilter::Executable) == DirFilters(DirFilter::Executable)) {
            if (!executable)
                return false;
        }
        return true;
    };

    if ((dirFilters & DirFilter::AllEntries) == DirFilters(DirFilter::AllEntries)
        || ((dirFilters & DirFilter::Dirs) && (dirFilters & DirFilter::Files))) {
        // 判断读写执行
        if (!checkRWE())
            ret = false;
    } else if ((dirFilters & DirFilter::Dirs) == DirFilters(DirFilter::Dirs)) {
        if (!isDir) {
            ret = false;
        } else {
            // 判断读写执行
            if (!checkRWE())
                ret = false;
        }
    } else if ((dirFilters & DirFilter::Files) == DirFilters(DirFilter::Files)) {
        const bool isFile = fileInfo->attribute(AttributeID::StandardIsFile)->toBool();
        if (!isFile) {
            ret = false;
        } else {
            // 判断读写执行
            if (!checkRWE())
                ret = false;
        }
    }

    if ((dirFilters & DirFilter::NoSymLinks) == DirFilters(DirFilter::NoSymLinks)) {
        const bool isSymlinks = fileInfo->attribute(AttributeID::StandardIsSymlink)->toBool();
        if (isSymlinks)
            ret = false;
    }

    const QString &fileInfoName = fileInfo->attribute(AttributeID::StandardName)->toString();
    const bool showHidden = (dirFilters & DirFilter::Hidden) == DirFilters(DirFilter::Hidden);
    if (!showHidden) {   // hide files
        const QString &parentPath = fileInfo->attribute(AttributeID::StandardParentPath)->toString();
        const QUrl &urlHidden = QUrl::fromLocalFile(parentPath + "/.hidden");

        QSet<QString> hideList;
        if (hideListMap.count(urlHidden) > 0) {
            hideList = hideListMap.value(urlHidden);
        } else {
            hideList = DFileHelper::hideListFromUrl(urlHidden);
            if (!hideList.empty())
                hideListMap.insert(urlHidden, hideList);
        }
        bool isHidden = DFileHelper::fileIsHidden(fileInfo, hideList);
        if (isHidden)
            ret = false;
    }

    // filter name
    const bool caseSensitive = (dirFilters & DirFilter::CaseSensitive) == DirFilters(DirFilter::CaseSensitive);
    if (nameFilters.contains(fileInfoName, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
        ret = false;

    const bool showDot = !((dirFilters & DirFilter::NoDotAndDotDot) == DirFilters(DirFilter::NoDotAndDotDot)) && !((dirFilters & DirFilter::NoDot) == DirFilters(DirFilter::NoDot));
    const bool showDotDot = !((dirFilters & DirFilter::NoDotAndDotDot) == DirFilters(DirFilter::NoDotAndDotDot)) && !((dirFilters & DirFilter::NoDotDot) == DirFilters(DirFilter::NoDotDot));
    if (!showDot && fileInfoName == ".")
        ret = false;
    if (!showDotDot && fileInfoName == "..")
        ret = false;

    return ret;
}

DFileFuture *DEnumeratorPrivate::createEnumeratorAsync(int ioPriority, QObject *parent)
{
    DFileFuture *future = new DFileFuture(parent);

    NormalFutureAsyncOp *dataOp = g_new0(NormalFutureAsyncOp, 1);
    dataOp->me = this;
    dataOp->future = future;

    const bool enumLinks = iteratorflags & IteratorFlag::FollowSymlinks;

    g_autoptr(GFile) gfile = g_file_new_for_uri(url.toString().toStdString().c_str());
    g_autoptr(GCancellable) cancellable = g_cancellable_new();

    g_file_enumerate_children_async(gfile,
                                    FILE_DEFAULT_ATTRIBUTES,
                                    enumLinks ? G_FILE_QUERY_INFO_NONE : G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                    ioPriority,
                                    cancellable,
                                    createEnumeratorAsyncCallback, dataOp);
    return future;
}

void DEnumeratorPrivate::createEnumeratorAsyncCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
    NormalFutureAsyncOp *data = static_cast<NormalFutureAsyncOp *>(userData);
    QPointer<DEnumeratorPrivate> me = data->me;
    DFileFuture *future = data->future;
    if (!me) {
        data->me = nullptr;
        data->future = nullptr;
        g_free(data);
        return;
    }

    GFile *gfile = G_FILE(sourceObject);
    g_autoptr(GError) gerror = nullptr;
    GFileEnumerator *genumerator = g_file_enumerate_children_finish(gfile, res, &gerror);

    if (gerror)
        me->setError(IOErrorCode(gerror->code));

    if (genumerator)
        me->stackEnumerator.push_back(genumerator);
    me->enumeratorInited = true;

    Q_EMIT future->finished();

    data->me = nullptr;
    data->future = nullptr;
    g_free(data);
}

DEnumerator::DEnumerator(const QUrl &url)
    : d(new DEnumeratorPrivate(this))
{
    d->url = url;
}

DEnumerator::~DEnumerator()
{
}

DExpected<QUrl> DEnumerator::url() const
{
    return d->url;
}

DExpected<void> DEnumerator::setNameFilters(const QStringList &nameFilters)
{
    d->nameFilters = nameFilters;
}

DExpected<void> DEnumerator::setDirFilters(DirFilters dirFilters)
{
    d->dirFilters = dirFilters;
}

DExpected<void> DEnumerator::setIteratorFlags(IteratorFlags flags)
{
    d->iteratorflags = flags;
}

DExpected<QStringList> DEnumerator::nameFilters() const
{
    return d->nameFilters;
}

DExpected<DirFilters> DEnumerator::dirFilters() const
{
    return d->dirFilters;
}

DExpected<IteratorFlags> DEnumerator::iteratorFlags() const
{
    return d->iteratorflags;
}

DExpected<void> DEnumerator::setTimeout(quint64 timeout)
{
    d->timeout = timeout;
}

DExpected<quint64> DEnumerator::timeout() const
{
    return d->timeout;
}

DExpected<bool> DEnumerator::createEnumerator()
{
    return d->createEnumerator(d->url);
}

DFileFuture *DEnumerator::createEnumeratorAsync(int ioPriority, QObject *parent)
{
    return d->createEnumeratorAsync(ioPriority, parent);
}

DExpected<bool> DEnumerator::hasNext() const
{
    if (!d->enumeratorInited) {
        bool succ = d->createEnumerator(d->url);
        if (!succ)
            return false;
    }
    if (d->stackEnumerator.isEmpty())
        return false;

    const bool enumSubDir = d->iteratorflags & IteratorFlag::Subdirectories;
    const bool enumLinks = d->iteratorflags & IteratorFlag::FollowSymlinks;
    // sub dir enumerator
    if (enumSubDir && d->fileInfo && d->fileInfo->attribute(AttributeID::StandardIsDir)->toBool()) {
        bool showDir = true;
        if (d->fileInfo->attribute(AttributeID::StandardIsSymlink)->toBool()) {
            // is symlink, need enumSymlink
            showDir = enumLinks;
        }
        if (showDir) {
            d->createEnumerator(d->nextUrl);
        }
    }
    if (d->stackEnumerator.isEmpty())
        return false;

    GFileEnumerator *enumerator = d->stackEnumerator.top();

    GFileInfo *gfileInfo = nullptr;
    GFile *gfile = nullptr;

    g_autoptr(GError) gerror = nullptr;
    bool hasNext = g_file_enumerator_iterate(enumerator, &gfileInfo, &gfile, nullptr, &gerror);
    if (hasNext) {
        if (!gfileInfo || !gfile) {
            GFileEnumerator *enumeratorPop = d->stackEnumerator.pop();
            g_object_unref(enumeratorPop);
            return this->hasNext().value();
        }

        g_autofree gchar *path = g_file_get_path(gfile);
        if (path) {
            d->nextUrl = QUrl::fromLocalFile(QString::fromStdString(path));
        } else {
            g_autofree gchar *uri = g_file_get_uri(gfile);
            d->nextUrl = QUrl(QString::fromStdString(uri));
        }
        d->fileInfo = new DFileInfo(d->nextUrl);
        d->fileInfo->setQueryAttributes(FILE_DEFAULT_ATTRIBUTES);
        d->fileInfo->setQueryInfoFlags(enumLinks ? FileQueryInfoFlags::TypeNone : FileQueryInfoFlags::TypeNoFollowSymlinks);
        d->fileInfo->initQuerier();

        if (!d->checkFilter())
            return this->hasNext().value();

        return true;
    }

    if (gerror)
        d->setError(IOErrorCode(gerror->code));

    return false;
}

DExpected<QUrl> DEnumerator::next() const
{
    return d->nextUrl;
}

DFileInfo *DEnumerator::createFileInfo(QObject *parent) const
{
    if (d->fileInfo)
        d->fileInfo->setParent(parent);

    return d->fileInfo;
}

DError DEnumerator::lastError() const
{
    return d->error;
}

DExpected<quint64> DEnumerator::fileCount()
{
    if (!d->enumeratorInited) {
        bool succ = d->createEnumerator(d->url);
        if (!succ)
            return false;
    }

    quint64 count = 0;

    while (hasNext().value()) {
        ++count;
    }

    return count;
}

DIO_END_NAMESPACE
