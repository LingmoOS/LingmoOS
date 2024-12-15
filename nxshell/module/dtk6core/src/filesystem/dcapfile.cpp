// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcapfile.h"
#include "dobject_p.h"
#include "private/dcapfsfileengine_p.h"

#include <private/qdir_p.h>

DCORE_BEGIN_NAMESPACE

extern QString _d_cleanPath(const QString &path);
extern bool _d_isSubFileOf(const QString &filePath, const QString &directoryPath);

class DCapFilePrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DCapFile)
public:
    DCapFilePrivate(DCapFile *qq, const QString &fileName = QString());
    static bool canReadWrite(const QString &path);

    QString fileName;
};

DCapFilePrivate::DCapFilePrivate(DCapFile *qq, const QString &fileName)
    : DObjectPrivate(qq)
    , fileName(fileName)
{
}

bool DCapFilePrivate::canReadWrite(const QString &path)
{
    DCapFSFileEngine engine(path);
    return engine.canReadWrite(path);
}

DCapFile::DCapFile(QObject *parent)
    : QFile(parent)
    , DObject(*new DCapFilePrivate(this))
{

}

DCapFile::DCapFile(const QString &name, QObject *parent)
    : QFile(name, parent)
    , DObject(*new DCapFilePrivate(this, name))
{
}

DCapFile::~DCapFile()
{

}

void DCapFile::setFileName(const QString &name)
{
    D_D(DCapFile);
    d->fileName = name;
    return QFile::setFileName(name);
}

bool DCapFile::exists() const
{
    D_DC(DCapFile);
    if (!d->canReadWrite(d->fileName))
        return false;

    return QFile::exists();
}

bool DCapFile::exists(const QString &fileName)
{
    return DCapFile(fileName).exists();
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QString DCapFile::readLink() const
{
    D_DC(DCapFile);
    if (!d->canReadWrite(d->fileName))
        return {};

    return QFile::symLinkTarget();
}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
QString DCapFile::symLinkTarget() const
{
    return QFile::symLinkTarget();
}
#endif

bool DCapFile::remove()
{
    D_D(DCapFile);
    if (!d->canReadWrite(d->fileName))
        return false;

    return QFile::remove();
}

bool DCapFile::remove(const QString &fileName)
{
    return DCapFile(fileName).remove();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
bool DCapFile::moveToTrash()
{
    D_D(DCapFile);
    if (!d->canReadWrite(d->fileName))
        return false;

    return QFile::moveToTrash();
}

bool DCapFile::moveToTrash(const QString &fileName, QString *pathInTrash)
{
    DCapFile file(fileName);
    if (file.moveToTrash()) {
        if (pathInTrash)
            *pathInTrash = file.fileName();
        return true;
    }
    return false;
}
#endif

bool DCapFile::rename(const QString &newName)
{
    D_D(DCapFile);
    if (!d->canReadWrite(newName))
        return false;

    return QFile::rename(newName);
}

bool DCapFile::rename(const QString &oldName, const QString &newName)
{
    if (!DCapFilePrivate::canReadWrite(oldName))
        return false;

    return DCapFile(oldName).rename(newName);
}

bool DCapFile::link(const QString &newName)
{
    D_D(DCapFile);
    if (!d->canReadWrite(newName))
        return false;

    return QFile::link(newName);
}

bool DCapFile::link(const QString &oldName, const QString &newName)
{
    if (!DCapFilePrivate::canReadWrite(oldName))
        return false;

    return DCapFile(oldName).link(newName);
}

bool DCapFile::copy(const QString &newName)
{
    D_D(DCapFile);
    if (!d->canReadWrite(newName))
        return false;

    return QFile::copy(newName);
}

bool DCapFile::copy(const QString &fileName, const QString &newName)
{
    if (!DCapFilePrivate::canReadWrite(fileName))
        return false;

    return DCapFile(fileName).copy(newName);
}

bool DCapFile::open(QIODevice::OpenMode flags)
{
    D_D(DCapFile);
    if (!d->canReadWrite(d->fileName))
        return false;

    return QFile::open(flags);
}

bool DCapFile::resize(qint64 sz)
{
    D_D(DCapFile);
    if (!d->canReadWrite(d->fileName))
        return false;

    return QFile::resize(sz);
}

bool DCapFile::resize(const QString &fileName, qint64 sz)
{
    return DCapFile(fileName).resize(sz);
}

bool DCapFile::open(FILE *, QIODevice::OpenMode, QFileDevice::FileHandleFlags)
{
    return false;
}

bool DCapFile::open(int, QIODevice::OpenMode, QFileDevice::FileHandleFlags)
{
    return false;
}

class DCapDirPrivate  : public QSharedData
{
public:
    DCapDirPrivate(QString filePath);
    explicit DCapDirPrivate(const DCapDirPrivate &copy);

    QString filePath;
};

DCapDirPrivate::DCapDirPrivate(QString filePath)
    : filePath(filePath)
{
}

DCapDirPrivate::DCapDirPrivate(const DCapDirPrivate &copy)
    : QSharedData(copy)
    , filePath(copy.filePath)
{
}

DCapDir::DCapDir(const DCapDir &dir)
    : QDir(dir)
    , dd_ptr(dir.dd_ptr)
{

}

DCapDir::DCapDir(const QString &path)
    : QDir(path)
    , dd_ptr(new DCapDirPrivate(path))
{

}

DCapDir::DCapDir(const QString &path, const QString &nameFilter,
                 QDir::SortFlags sort, QDir::Filters filter)
    : QDir(path, nameFilter, sort, filter)
    , dd_ptr(new DCapDirPrivate(path))
{

}

DCapDir::~DCapDir()
{

}

void DCapDir::setPath(const QString &path)
{
    dd_ptr = new DCapDirPrivate(path);
    return QDir::setPath(path);
}

bool DCapDir::cd(const QString &dirName)
{
    auto old_d = d_ptr;
    bool ret = QDir::cd(dirName);
    if (!ret)
        return ret;

    // take the new path.
    auto path = QDir::filePath("");
    QScopedPointer<DCapFSFileEngine> fsEngine(new DCapFSFileEngine(path));
    if (fsEngine->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::ExistsFlag) {
        dd_ptr = new DCapDirPrivate(path);
        return true;
    }
    d_ptr = old_d;
    return false;
}

QStringList DCapDir::entryList(DCapDir::Filters filters, DCapDir::SortFlags sort) const
{
    const QDirPrivate* d = d_ptr.constData();
    return entryList(d->nameFilters, filters, sort);
}

QStringList DCapDir::entryList(const QStringList &nameFilters, DCapDir::Filters filters, DCapDir::SortFlags sort) const
{
    if (!DCapFilePrivate::canReadWrite(dd_ptr->filePath))
        return {};
    return QDir::entryList(nameFilters, filters, sort);
}

QFileInfoList DCapDir::entryInfoList(QDir::Filters filters, QDir::SortFlags sort) const
{
    const QDirPrivate* d = d_ptr.constData();
    return entryInfoList(d->nameFilters, filters, sort);
}

QFileInfoList DCapDir::entryInfoList(const QStringList &nameFilters, DCapDir::Filters filters, DCapDir::SortFlags sort) const
{
    if (!DCapFilePrivate::canReadWrite(dd_ptr->filePath))
        return {};
    return QDir::entryInfoList(nameFilters, filters, sort);
}

bool DCapDir::mkdir(const QString &dirName) const
{
    QString fn = filePath(dirName);
    if (!DCapFilePrivate::canReadWrite(fn))
        return false;

    return QDir::mkdir(dirName);
}

bool DCapDir::rmdir(const QString &dirName) const
{
    QString fn = filePath(dirName);
    if (!DCapFilePrivate::canReadWrite(fn))
        return false;

    return QDir::rmdir(dirName);
}

bool DCapDir::mkpath(const QString &dirPath) const
{
    QString fn = filePath(dirPath);
    if (!DCapFilePrivate::canReadWrite(fn))
        return false;

    return QDir::mkpath(dirPath);
}

bool DCapDir::rmpath(const QString &dirPath) const
{
    QString fn = filePath(dirPath);
    if (!DCapFilePrivate::canReadWrite(fn))
        return false;

    return QDir::rmpath(dirPath);
}

bool DCapDir::exists() const
{
    if (!DCapFilePrivate::canReadWrite(dd_ptr->filePath))
        return false;

    return QDir::exists();
}

bool DCapDir::exists(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("DCapFile::exists: Empty or null file name");
        return false;
    }
    return DCapFile::exists(filePath(name));
}

bool DCapDir::remove(const QString &fileName)
{
    if (fileName.isEmpty()) {
        qWarning("DCapDir::remove: Empty or null file name");
        return false;
    }
    return DCapFile::remove(filePath(fileName));
}

bool DCapDir::rename(const QString &oldName, const QString &newName)
{
    if (oldName.isEmpty() || newName.isEmpty()) {
        qWarning("DCapDir::rename: Empty or null file name(s)");
        return false;
    }

    DCapFile file(filePath(oldName));
    if (!file.exists())
        return false;
    return file.rename(filePath(newName));
}

DCORE_END_NAMESPACE
