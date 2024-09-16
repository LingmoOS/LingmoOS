// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAPFILE_H
#define DCAPFILE_H

#include <dtkcore_global.h>

#include <DObject>
#include <QDir>
#include <QFile>

DCORE_BEGIN_NAMESPACE

class DCapFilePrivate;
class DCapFile : public QFile, public DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DCapFile)
    Q_DISABLE_COPY(DCapFile)

public:
    explicit DCapFile(QObject *parent = nullptr);
    DCapFile(const QString &name, QObject *parent = nullptr);

    ~DCapFile() override;

    void setFileName(const QString &name);

    bool exists() const;
    static bool exists(const QString &fileName);

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    D_DECL_DEPRECATED_X("Use QFile::symLinkTarget() instead")
    QString readLink() const;
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    QString symLinkTarget() const;
#endif
    bool remove();
    static bool remove(const QString &fileName);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    bool moveToTrash();
    static bool moveToTrash(const QString &fileName, QString *pathInTrash = nullptr);
#endif

    bool rename(const QString &newName);
    static bool rename(const QString &oldName, const QString &newName);

    bool link(const QString &newName);
    static bool link(const QString &oldname, const QString &newName);

    bool copy(const QString &newName);
    static bool copy(const QString &fileName, const QString &newName);

    bool open(OpenMode flags) override;

    bool resize(qint64 sz) override;
    static bool resize(const QString &filename, qint64 sz);

private:
    bool open(FILE *f, OpenMode ioFlags, FileHandleFlags handleFlags=DontCloseHandle);
    bool open(int fd, OpenMode ioFlags, FileHandleFlags handleFlags=DontCloseHandle);
};

class DCapDirPrivate;
class DCapDir : public QDir
{
public:
    DCapDir(const DCapDir &);
    DCapDir(const QString &path = QString());
    DCapDir(const QString &path, const QString &nameFilter,
         SortFlags sort = SortFlags(Name | IgnoreCase), Filters filter = AllEntries);
    ~DCapDir();

    void setPath(const QString &path);

    bool cd(const QString &dirName);

    QStringList entryList(Filters filters = NoFilter, SortFlags sort = NoSort) const;
    QStringList entryList(const QStringList &nameFilters, Filters filters = NoFilter,
                          SortFlags sort = NoSort) const;

    QFileInfoList entryInfoList(Filters filters = NoFilter, SortFlags sort = NoSort) const;
    QFileInfoList entryInfoList(const QStringList &nameFilters, Filters filters = NoFilter,
                                SortFlags sort = NoSort) const;

    bool mkdir(const QString &dirName) const;
    bool rmdir(const QString &dirName) const;
    bool mkpath(const QString &dirPath) const;
    bool rmpath(const QString &dirPath) const;
    bool exists() const;
    bool remove(const QString &fileName);
    bool rename(const QString &oldName, const QString &newName);
    bool exists(const QString &name) const;

private:
    QSharedDataPointer<DCapDirPrivate> dd_ptr;
};

DCORE_END_NAMESPACE
Q_DECLARE_SHARED(DTK_CORE_NAMESPACE::DCapDir)
#endif // DCAPFILE_H
