// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include <QObject>
#include <QMap>


enum FileType {
    // base type
    Directory,
    CharDevice,
    BlockDevice,
    FIFOFile,
    SocketFile,
    RegularFile,
    //
    Executable,
    Documents,
    Images,
    Videos,
    Audios,
    Archives,
    DesktopApplication,
    Backups,
    Unknown,
    CustomType = 0x100
};

#define MIMETYPE_PATH "/usr/share/dde-file-manager/mimetypes"

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT
public:

    explicit MimeTypeDisplayManager(QObject *parent = nullptr);

    void initData();
    void initConnect();

    QString displayName(const QString &mimeType);
    FileType displayNameToEnum(const QString &mimeType);
    QString defaultIcon(const QString &mimeType);

    QMap<FileType, QString> displayNames();
    static QStringList readlines(const QString &path);
    static void loadSupportMimeTypes();
    static QStringList supportArchiveMimetypes();
    static QStringList supportVideoMimeTypes();

private:
    QMap<FileType, QString> m_displayNames;
    QMap<FileType, QString> m_defaultIconNames;
    static QStringList ArchiveMimeTypes;
    static QStringList AvfsBlackList;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;
    static QStringList BackupMimeTypes;

    bool m_supportLoaded = false;
};

#endif // MIMETYPEDISPLAYMANAGER_H
