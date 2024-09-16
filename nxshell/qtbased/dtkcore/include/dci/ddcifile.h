// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#ifndef DTK_NO_PROJECT
#include <DObject>
#include <dtkcore_global.h>
#else
#define DCORE_BEGIN_NAMESPACE
#define DCORE_END_NAMESPACE
#define LIBDTKCORESHARED_EXPORT
#define D_DECLARE_PRIVATE(Class) Class##Private *d;
#endif

#include <QStringList>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

DCORE_BEGIN_NAMESPACE

class DDciFilePrivate;
class LIBDTKCORESHARED_EXPORT DDciFile
#ifndef DTK_NO_PROJECT
        : public DObject
#endif
{
    D_DECLARE_PRIVATE(DDciFile)
public:
    enum FileType {
        UnknowFile,
        File = 1,
        Directory = 2,
        Symlink = 3
    };

    static void registerFileEngine();

    DDciFile();
    explicit DDciFile(const QString &fileName);
    explicit DDciFile(const QByteArray &data);

    bool isValid() const;
    QString lastErrorString() const;

    bool writeToFile(const QString &fileName) const;
    bool writeToDevice(QIODevice *device) const;
    QByteArray toData() const;

    static constexpr int metadataSizeV1();

    // for reader
    QStringList list(const QString &dir, bool onlyFileName = false) const;
    int childrenCount(const QString &dir) const;
    bool exists(const QString &filePath) const;
    FileType type(const QString &filePath) const;
    QByteArray dataRef(const QString &filePath) const;
    QString name(const QString &filePath) const;
    QString symlinkTarget(const QString &filePath, bool originData = false) const;

    // for writer
    bool mkdir(const QString &filePath);
    bool writeFile(const QString &filePath, const QByteArray &data, bool override = false);
    bool remove(const QString &filePath);
    bool rename(const QString &filePath, const QString &newFilePath, bool override = false);
    bool copy(const QString &from, const QString &to);
    bool link(const QString &source, const QString &to);
};

DCORE_END_NAMESPACE
