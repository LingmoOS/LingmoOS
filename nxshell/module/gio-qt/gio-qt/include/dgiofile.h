// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOFILE_H
#define DGIOFILE_H

#include <QObject>
#include <QSharedData>

#include <limits.h>

namespace Gio {
class File;
}

enum DGioFileQueryInfoFlag
{
    FILE_QUERY_INFO_NONE = 0x0,
    FILE_QUERY_INFO_NOFOLLOW_SYMLINKS = (1 << 0)
};

Q_DECLARE_FLAGS(DGioFileQueryInfoFlags, DGioFileQueryInfoFlag)

class DGioFileInfo;
class DGioMount;
class DGioFileIterator;
class DGioMountOperation;
class DGioFilePrivate;
class DGioFile : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DGioFile(Gio::File *gmmFilePtr, QObject *parent = nullptr);
    ~DGioFile();

    static DGioFile * createFromPath(QString path, QObject *parent = nullptr);
    static DGioFile * createFromUri(QString uri, QObject *parent = nullptr);
    static DGioFile * createFromCmdArg(QString uri, QObject *parent = nullptr);

    QString basename() const;
    QString path() const;
    QString uri() const;

    QExplicitlySharedDataPointer<DGioFileInfo> createFileInfo(QString attr = "*", DGioFileQueryInfoFlags queryInfoFlags = FILE_QUERY_INFO_NONE, unsigned long timeout_msec = ULONG_MAX);
    QExplicitlySharedDataPointer<DGioFileInfo> createFileSystemInfo(QString attr = "*");
    QExplicitlySharedDataPointer<DGioFileIterator> createFileIterator(QString attr = "*", DGioFileQueryInfoFlags queryInfoFlags = FILE_QUERY_INFO_NONE);
    void createFileIteratorAsync(QString attr = "*", DGioFileQueryInfoFlags queryInfoFlags = FILE_QUERY_INFO_NONE);

    void mountEnclosingVolume(DGioMountOperation *dgioMountOperation);

    QExplicitlySharedDataPointer<DGioMount> findEnclosingMount();

Q_SIGNALS:
    void createFileIteratorReady(QExplicitlySharedDataPointer<DGioFileIterator> iter);
    void mountEnclosingVolumeReady(bool result, QString msg);

private:
    QScopedPointer<DGioFilePrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioFile)
};

#endif // DGIOFILE_H
