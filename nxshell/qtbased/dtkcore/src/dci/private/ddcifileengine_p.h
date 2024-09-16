// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#ifndef DTK_NO_PROJECT
#include <dtkcore_global.h>
#else
#define DCORE_BEGIN_NAMESPACE
#define DCORE_END_NAMESPACE
#endif
#include <private/qabstractfileengine_p.h>

#include <QSharedPointer>
#include <QDateTime>

QT_BEGIN_NAMESPACE
class QBuffer;
QT_END_NAMESPACE

DCORE_BEGIN_NAMESPACE

class DDciFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    QAbstractFileEngine *create(const QString &fileName) const override;
};

class DDciFile;
using DDciFileShared = QSharedPointer<DDciFile>;
class DDciFileEngineIterator : public QAbstractFileEngineIterator
{
    friend class DDciFileEngine;
public:
    DDciFileEngineIterator(QDir::Filters filters, const QStringList &nameFilters);

    QString next() override;
    bool hasNext() const override;

    QString currentFileName() const override;

private:
    mutable DDciFileShared file;
    mutable QStringList list;
    mutable int nextValid = -1;
    int current = -1;
};

class DDciFileEngine : public QAbstractFileEngine
{
    friend class DDciFileEngineIterator;
public:
    explicit DDciFileEngine(const QString &fullPath);
    ~DDciFileEngine();

    bool isValid() const;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    bool open(QIODevice::OpenMode openMode, std::optional<QFile::Permissions> permissions = std::nullopt) override;
#else
    bool open(QIODevice::OpenMode openMode) override;
#endif
    bool close() override;
    bool flushToFile(QFile *target, bool writeFile) const;
    bool flush() override;
    bool syncToDisk() override;

    qint64 size() const override;
    qint64 pos() const override;
    bool seek(qint64 pos) override;
    bool isSequential() const override;
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
    bool setSize(qint64 size) override;
    bool caseSensitive() const override;
    bool isRelativePath() const override;

    QByteArray id() const override;
    uint ownerId(FileOwner owner) const override;
    QString owner(FileOwner owner) const override;

    FileFlags fileFlags(FileFlags type = FileInfoAll) const override;
    QString fileName(FileName file = DefaultName) const override;

    void setFileName(const QString &fullPath) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 1)
    QDateTime fileTime(QFile::FileTime time) const override;
#else
    QDateTime fileTime(FileTime time) const override;
#endif

    typedef DDciFileEngineIterator Iterator;
    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
    Iterator *endEntryList() override;

    qint64 read(char *data, qint64 maxlen) override;
    qint64 write(const char *data, qint64 len) override;

    bool extension(Extension extension, const ExtensionOption *option = 0,
                   ExtensionReturn *output = 0) override;
    bool supportsExtension(Extension extension) const override;

    bool cloneTo(QAbstractFileEngine *target) override;

private:
    bool forceSave(bool writeFile = false) const;

    /*
     * fullPath 格式："dci:" + "真实文件路径" + "DCI 内部文件的路径"
     * 例如："dci:/home/user/test.dci/subfile.png"
     * 其中 "/home/user/test.dci" 为真实文件路径，"/subfile.png"
     * 是 DCI 文件的内部路径。
     * 函数返回的第一个数据是"真实文件路径"。
     */
    static QPair<QString, QString> resolvePath(const QString &fullPath,
                                               const QString &realFilePath = QString(),
                                               bool needRealFileExists = true);

    DDciFileShared file;
    QString dciFilePath;
    QFile realDciFile;
    QString subfilePath;

    QByteArray fileData;
    QBuffer *fileBuffer = nullptr;
};


DCORE_END_NAMESPACE
