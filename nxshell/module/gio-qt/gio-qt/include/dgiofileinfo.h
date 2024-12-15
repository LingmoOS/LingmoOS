// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOFILEINFO_H
#define DGIOFILEINFO_H

#include <QObject>
#include <QSharedData>

enum DGioFileType {
    FILE_TYPE_NOT_KNOWN,
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_SYMBOLIC_LINK,
    FILE_TYPE_SPECIAL,
    FILE_TYPE_SHORTCUT,
    FILE_TYPE_MOUNTABLE
};
Q_ENUMS(DGioFileType);

namespace Gio {
class FileInfo;
}

class DGioFileInfoPrivate;
class DGioFileInfo : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DGioFileInfo(Gio::FileInfo *gmmFileInfoInfoPtr, QObject *parent = nullptr);
    ~DGioFileInfo();

    // file info
    QString displayName() const;
    DGioFileType fileType() const;
    quint64 fileSize() const;
    QString contentType() const;
    QString iconString() const;
    QStringList themedIconNames() const;

    // filesystem info.
    bool fsReadOnly() const;
    quint64 fsTotalBytes() const;
    quint64 fsUsedBytes() const;
    quint64 fsFreeBytes() const;
    QString fsType() const;

private:
    QScopedPointer<DGioFileInfoPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioFileInfo)
};

#endif // DGIOFILEINFO_H
