// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypedisplaymanager.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>

QStringList MimeTypeDisplayManager::ArchiveMimeTypes;
QStringList MimeTypeDisplayManager::TextMimeTypes;
QStringList MimeTypeDisplayManager::VideoMimeTypes;
QStringList MimeTypeDisplayManager::AudioMimeTypes;
QStringList MimeTypeDisplayManager::ImageMimeTypes;
QStringList MimeTypeDisplayManager::ExecutableMimeTypes;
QStringList MimeTypeDisplayManager::BackupMimeTypes;

MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent) : QObject(parent)
{
    initData();
    initConnect();
}

void MimeTypeDisplayManager::initData()
{
    m_displayNames[FileType::Directory] = tr("Directory");
    m_displayNames[FileType::DesktopApplication] = tr("Application");
    m_displayNames[FileType::Videos] = tr("Video");
    m_displayNames[FileType::Audios] = tr("Audio");
    m_displayNames[FileType::Images] = tr("Image");
    m_displayNames[FileType::Archives] = tr("Archive");
    m_displayNames[FileType::Documents] = tr("Document");
    m_displayNames[FileType::Executable] = tr("Executable");
    m_displayNames[FileType::Backups] = tr("Backup file");
    m_displayNames[FileType::Unknown] = tr("Unknown");

    m_defaultIconNames[FileType::Directory] = "folder";
    m_defaultIconNames[FileType::DesktopApplication] = "application-default-icon";
    m_defaultIconNames[FileType::Videos] = "video";
    m_defaultIconNames[FileType::Audios] = "music";
    m_defaultIconNames[FileType::Images] = "image";
    m_defaultIconNames[FileType::Archives] = "application-x-archive";
    m_defaultIconNames[FileType::Documents] = "text-plain";
    m_defaultIconNames[FileType::Executable] = "application-x-executable";
    m_defaultIconNames[FileType::Backups] = "application-x-archive"; // generic backup file icon?
    m_defaultIconNames[FileType::Unknown] = "application-default-icon";

    loadSupportMimeTypes();
}

void MimeTypeDisplayManager::initConnect()
{

}

QString MimeTypeDisplayManager::displayName(const QString &mimeType)
{
    return m_displayNames.value(displayNameToEnum(mimeType));
}

FileType MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType)
{
    if (mimeType == "application/x-desktop") {
        return FileType::DesktopApplication;
    } else if (mimeType == "inode/directory") {
        return FileType::Directory;
    } else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)) {
        return FileType::Executable;
    } else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)) {
        return FileType::Videos;
    } else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)) {
        return FileType::Audios;
    } else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)) {
        return FileType::Images;
    } else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)) {
        return FileType::Documents;
    } else if (ArchiveMimeTypes.contains(mimeType)) {
        return FileType::Archives;
    } else if (BackupMimeTypes.contains(mimeType)) {
        return FileType::Backups;
    } else {
        return FileType::Unknown;
    }
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    return m_defaultIconNames.value(displayNameToEnum(mimeType));
}

QMap<FileType, QString> MimeTypeDisplayManager::displayNames()
{
    return m_displayNames;
}

QStringList MimeTypeDisplayManager::readlines(const QString &path)
{
    QStringList result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        // Read new line
        QString line = in.readLine();
        // Skip empty line or line with invalid format
        if (line.trimmed().isEmpty()) {
            continue;
        }

        result.append(line.trimmed());
    }

    file.close();
    return result;
}

void MimeTypeDisplayManager::loadSupportMimeTypes()
{
    QString textPath = QString("%1/%2").arg(MIMETYPE_PATH, "text.mimetype");
    QString archivePath = QString("%1/%2").arg(MIMETYPE_PATH, "archive.mimetype");
    QString videoPath = QString("%1/%2").arg(MIMETYPE_PATH, "video.mimetype");
    QString audioPath = QString("%1/%2").arg(MIMETYPE_PATH, "audio.mimetype");
    QString imagePath = QString("%1/%2").arg(MIMETYPE_PATH, "image.mimetype");
    QString executablePath = QString("%1/%2").arg(MIMETYPE_PATH, "executable.mimetype");
    QString backupPath = QString("%1/%2").arg(MIMETYPE_PATH, "backup.mimetype");
    TextMimeTypes = readlines(textPath);
    ArchiveMimeTypes = readlines(archivePath);
    VideoMimeTypes = readlines(videoPath);
    AudioMimeTypes = readlines(audioPath);
    ImageMimeTypes = readlines(imagePath);
    ExecutableMimeTypes = readlines(executablePath);
    BackupMimeTypes = readlines(backupPath);
}

QStringList MimeTypeDisplayManager::supportArchiveMimetypes()
{
    return ArchiveMimeTypes;
}

QStringList MimeTypeDisplayManager::supportVideoMimeTypes()
{
    return VideoMimeTypes;
}
