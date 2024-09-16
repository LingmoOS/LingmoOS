// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DVIRTUALIMAGEFILEIO_H
#define DVIRTUALIMAGEFILEIO_H

#include <QHash>
#include <QExplicitlySharedDataPointer>
#include <QFile>

class DVirtualImageFileIOPrivate;
class DVirtualImageFileIO
{
public:
    explicit DVirtualImageFileIO(const QString &fileName);
    ~DVirtualImageFileIO();

    QString fileName();
    bool setFile(const QString &fileName);
    bool setSize(qint64 size);
    bool isValid() const;

    bool existes(const QString &fileName) const;

    bool isOpen(const QString &fileName) const;
    bool open(const QString &fileName, QIODevice::OpenMode openMode = QIODevice::ReadOnly);
    bool close();
    qint64 pos() const;
    bool seek(qint64 pos);

    bool flush();
    bool isSequential() const;
    QFileDevice::Permissions permissions() const;

    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

    qint64 size(const QString &fileName) const;
    qint64 start(const QString &fileName) const;
    qint64 end(const QString &fileName) const;
    bool setSize(const QString &fileName, qint64 size);
    bool rename(const QString &from, const QString &to);
    bool isWritable(const QString &fileName);

    static int maxFileCount();
    static qint64 metaDataSize();
    qint64 validMetaDataSize() const;
    qint64 fileDataSize() const;
    qint64 writableDataSize() const;
    QStringList fileList() const;

    static bool updateMD5sum(const QString &fileName);

private:
    bool addFile(const QString &name);
    QByteArray md5sum(bool readCache = true);
    bool updateMD5sum();

    QExplicitlySharedDataPointer<DVirtualImageFileIOPrivate> d;

    friend class DVirtualImageFileEngine;
};

#endif // DVIRTUALIMAGEFILEIO_H
