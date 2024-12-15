// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAPFSFILEENGINE_P_H
#define DCAPFSFILEENGINE_P_H

#include <DObject>

#include <private/qfsfileengine_p.h>

DCORE_BEGIN_NAMESPACE

class DCapFSFileEngineHandler : public QAbstractFileEngineHandler
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    std::unique_ptr<QAbstractFileEngine> create(const QString &fileName) const override;
#else
    QAbstractFileEngine *create(const QString &fileName) const override;
#endif
};

class DCapFSFileEnginePrivate;
class DCapFSFileEngine : public QFSFileEngine, public DObject
{
    D_DECLARE_PRIVATE(DCapFSFileEngine);
public:
    DCapFSFileEngine(const QString &file);
    ~DCapFSFileEngine() override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    bool open(QIODevice::OpenMode openMode, std::optional<QFile::Permissions> permissions = std::nullopt) override;
#else
    bool open(QIODevice::OpenMode openMode) override;
#endif
    bool remove() override;
    bool copy(const QString &newName) override;
    bool rename(const QString &newName) override;
    bool renameOverwrite(const QString &newName) override;
    bool link(const QString &newName) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    bool mkdir(const QString &dirName,
               bool createParentDirectories,
               std::optional<QFile::Permissions> permissions = std::nullopt) const override;
#else
    bool mkdir(const QString &dirName, bool createParentDirectories) const override;
#endif
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const override;
    FileFlags fileFlags(FileFlags type) const override;
    bool cloneTo(QAbstractFileEngine *target) override;
    bool setSize(qint64 size) override;
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 1)
    IteratorUniquePtr beginEntryList(const QString &path, QDirListing::IteratorFlags filters, const QStringList &filterNames) override;
#elif QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    IteratorUniquePtr beginEntryList(const QString &path, QDir::Filters filters, const QStringList &filterNames) override;
#else
    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
#endif

    bool canReadWrite(const QString &path) const;
};

DCORE_END_NAMESPACE
#endif // DCAPFSFILEENGINE_P_H
