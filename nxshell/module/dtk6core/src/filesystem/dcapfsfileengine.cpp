// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dcapfsfileengine_p.h"
#include "private/dobject_p.h"
#include "dvtablehook.h"

#include "dcapmanager.h"
#include <QDebug>

DCORE_BEGIN_NAMESPACE

extern QString _d_cleanPath(const QString &path);
extern bool _d_isSubFileOf(const QString &filePath, const QString &directoryPath);

#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
static bool capDirIteraterHasNext(QAbstractFileEngineIterator *it)
{
    const QStringList &paths = DCapManager::instance()->paths();
    QString path = it->path();
    QFileInfo info(path);
    if (info.isSymLink())
        info = QFileInfo{info.symLinkTarget()};

    bool ret = std::any_of(paths.cbegin(), paths.cend(), std::bind(_d_isSubFileOf, path, std::placeholders::_1));

    if (!ret)
        return ret;
    return DVtableHook::callOriginalFun(it, &QAbstractFileEngineIterator::hasNext);
}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
std::unique_ptr<QAbstractFileEngine> DCapFSFileEngineHandler::create(const QString &fileName) const
#else
QAbstractFileEngine *DCapFSFileEngineHandler::create(const QString &fileName) const
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return std::unique_ptr<QAbstractFileEngine>(new DCapFSFileEngine(fileName));
#else
    return new DCapFSFileEngine(fileName);
#endif
}


class DCapFSFileEnginePrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DCapFSFileEngine)
public:
    DCapFSFileEnginePrivate(const QString &file, DCapFSFileEngine *qq);

    bool canReadWrite(const QString &path) const;

    QString file;
};

DCapFSFileEnginePrivate::DCapFSFileEnginePrivate(const QString &file, DCapFSFileEngine *qq)
    : DObjectPrivate(qq)
    , file(file)
{

}

bool DCapFSFileEnginePrivate::canReadWrite(const QString &path) const
{
    if (path.isEmpty())
        return false;

    QString target = path;
    if (path == this->file) {
        D_QC(DCapFSFileEngine);
        target = q->fileName(DCapFSFileEngine::AbsoluteName);
    } else {
        QFSFileEngine engine(path);
        target = engine.fileName(DCapFSFileEngine::AbsoluteName);
    }

    auto paths = DCapManager::instance()->paths();
    return std::any_of(paths.cbegin(), paths.cend(),
                       std::bind(_d_isSubFileOf, target, std::placeholders::_1));
}

DCapFSFileEngine::DCapFSFileEngine(const QString &file)
    : QFSFileEngine(file)
    , DObject(*new DCapFSFileEnginePrivate(file, this))
{

}

DCapFSFileEngine::~DCapFSFileEngine()
{
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
bool DCapFSFileEngine::open(QIODevice::OpenMode openMode, std::optional<QFile::Permissions> permissions)
#else
bool DCapFSFileEngine::open(QIODevice::OpenMode openMode)
#endif
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(d->file))
        return false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    return QFSFileEngine::open(openMode, permissions);
#else
    return QFSFileEngine::open(openMode);
#endif
}

bool DCapFSFileEngine::remove()
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(d->file))
        return false;
    return QFSFileEngine::remove();
}

bool DCapFSFileEngine::copy(const QString &newName)
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(newName)) {
        // ###(Chen Bin): If false is returned here, QFile
        // will use the interface of qtemporaryfile for
        // file operation, and the restrictions in
        // DCapFSFileEngine cannot be used. And it will be
        // copied successfully.
        qWarning() << "DCapFSFileEngine: " << QStringLiteral("The file [%1] has no permission to copy!").arg(newName);
        return true;
    }
    return QFSFileEngine::copy(newName);
}

bool DCapFSFileEngine::rename(const QString &newName)
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(newName))
        return false;
    return QFSFileEngine::rename(newName);
}

bool DCapFSFileEngine::renameOverwrite(const QString &newName)
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(newName))
        return false;
    return QFSFileEngine::renameOverwrite(newName);
}

bool DCapFSFileEngine::link(const QString &newName)
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(newName))
        return false;
    return QFSFileEngine::link(newName);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
bool DCapFSFileEngine::mkdir(const QString &dirName,
                             bool createParentDirectories,
                             std::optional<QFile::Permissions> permissions) const
#else
bool DCapFSFileEngine::mkdir(const QString &dirName, bool createParentDirectories) const
#endif
{
    D_DC(DCapFSFileEngine);
    if (!d->canReadWrite(dirName))
        return false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    return QFSFileEngine::mkdir(dirName, createParentDirectories, permissions);
#else
    return QFSFileEngine::mkdir(dirName, createParentDirectories);
#endif
}

bool DCapFSFileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
{
    D_DC(DCapFSFileEngine);
    if (!d->canReadWrite(dirName))
        return false;
    return QFSFileEngine::rmdir(dirName, recurseParentDirectories);
}

QAbstractFileEngine::FileFlags DCapFSFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    D_DC(DCapFSFileEngine);
    FileFlags ret = QFSFileEngine::fileFlags(type);
    if (ret | ExistsFlag) {
        if (!d->canReadWrite(d->file)) {
            ret &= ~ExistsFlag;
        }
    }

    return ret;
}

bool DCapFSFileEngine::cloneTo(QAbstractFileEngine *target)
{
    D_DC(DCapFSFileEngine);
    const QString targetPath = target->fileName(DCapFSFileEngine::AbsolutePathName);
    if (!d->canReadWrite(targetPath))
        return false;
    return QFSFileEngine::cloneTo(target);
}

bool DCapFSFileEngine::setSize(qint64 size)
{
    D_D(DCapFSFileEngine);
    if (!d->canReadWrite(d->file))
        return false;
    return QFSFileEngine::setSize(size);
}

QStringList DCapFSFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    D_DC(DCapFSFileEngine);
    if (!d->canReadWrite(d->file))
        return {};
    return QFSFileEngine::entryList(filters, filterNames);
}
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 1)
QAbstractFileEngine::IteratorUniquePtr DCapFSFileEngine::beginEntryList(const QString &path, QDirListing::IteratorFlags filters, const QStringList &filterNames)
#elif QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
QAbstractFileEngine::IteratorUniquePtr DCapFSFileEngine::beginEntryList(const QString &path, QDir::Filters filters, const QStringList &filterNames)
#else
QAbstractFileEngine::Iterator *DCapFSFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    auto ret = QFSFileEngine::beginEntryList(path, filters, filterNames);
#else
    auto ret = QFSFileEngine::beginEntryList(filters, filterNames);
    DVtableHook::overrideVfptrFun(ret, &QAbstractFileEngineIterator::hasNext, &capDirIteraterHasNext);
#endif
    return ret;
}

bool DCapFSFileEngine::canReadWrite(const QString &path) const
{
    D_DC(DCapFSFileEngine);
    return d->canReadWrite(path);
}

DCORE_END_NAMESPACE

