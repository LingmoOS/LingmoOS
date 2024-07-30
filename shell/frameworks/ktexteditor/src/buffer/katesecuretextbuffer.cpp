/*
    SPDX-FileCopyrightText: 2017 KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katesecuretextbuffer_p.h"

#include "config.h"

#include <KAuth/HelperSupport>

#ifndef Q_OS_WIN
#include <cerrno>
#include <unistd.h>
#endif

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTemporaryFile>

KAUTH_HELPER_MAIN("org.kde.ktexteditor6.katetextbuffer", SecureTextBuffer)

ActionReply SecureTextBuffer::savefile(const QVariantMap &args)
{
    const QString sourceFile = args[QStringLiteral("sourceFile")].toString();
    const QString targetFile = args[QStringLiteral("targetFile")].toString();
    const QByteArray checksum = args[QStringLiteral("checksum")].toByteArray();
    const uint ownerId = (uint)args[QStringLiteral("ownerId")].toInt();
    const uint groupId = (uint)args[QStringLiteral("groupId")].toInt();

    if (saveFileInternal(sourceFile, targetFile, checksum, ownerId, groupId)) {
        return ActionReply::SuccessReply();
    }

    return ActionReply::HelperErrorReply();
}

bool SecureTextBuffer::saveFileInternal(const QString &sourceFile,
                                        const QString &targetFile,
                                        const QByteArray &checksum,
                                        const uint ownerId,
                                        const uint groupId)
{
    // open source file for reading
    // if not possible, signal error
    QFile readFile(sourceFile);
    if (!readFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    // construct file info for target file
    // we need to know things like path/exists/permissions
    const QFileInfo targetFileInfo(targetFile);

    // create temporary file in current directory to be able to later do an atomic rename
    // we need to pass full path, else QTemporaryFile uses the temporary directory
    // if not possible, signal error, this catches e.g. a non-existing target directory, too
    QTemporaryFile tempFile(targetFileInfo.absolutePath() + QLatin1String("/secureXXXXXX"));
    if (!tempFile.open()) {
        return false;
    }

    // copy contents + do checksumming
    // if not possible, signal error
    QCryptographicHash cryptographicHash(checksumAlgorithm);
    const qint64 bufferLength = 4096;
    char buffer[bufferLength];
    qint64 read = -1;
    while ((read = readFile.read(buffer, bufferLength)) > 0) {
        cryptographicHash.addData(QByteArrayView(buffer, read));
        if (tempFile.write(buffer, read) == -1) {
            return false;
        }
    }

    // check that copying was successful and checksum matched
    // we need to flush the file, as QTemporaryFile keeps the handle open
    // and we later do things like renaming of the file!
    // if not possible, signal error
    if ((read == -1) || (cryptographicHash.result() != checksum) || !tempFile.flush()) {
        return false;
    }

    // try to preserve the permissions
    if (!targetFileInfo.exists()) {
        // ensure new file is readable by anyone
        tempFile.setPermissions(tempFile.permissions() | QFile::Permission::ReadGroup | QFile::Permission::ReadOther);
    } else {
        // ensure the same file permissions
        tempFile.setPermissions(targetFileInfo.permissions());

        // ensure file has the same owner and group as before
        setOwner(tempFile.handle(), ownerId, groupId);
    }

    // try to (atomic) rename temporary file to the target file
    if (moveFile(tempFile.fileName(), targetFileInfo.filePath())) {
        // temporary file was renamed, there is nothing to remove anymore
        tempFile.setAutoRemove(false);
        return true;
    }

    // we failed
    // QTemporaryFile will handle cleanup
    return false;
}

void SecureTextBuffer::setOwner(const int filedes, const uint ownerId, const uint groupId)
{
#ifndef Q_OS_WIN
    if (ownerId != (uint)-2 && groupId != (uint)-2) {
        int result = fchown(filedes, ownerId, groupId);
        // set at least correct group if owner cannot be changed
        if (result != 0 && errno == EPERM) {
            result = fchown(filedes, getuid(), groupId);
            (void)result; // ignore if we can't do a thing
        }
    }
#else
    // no-op for windows
#endif
}

bool SecureTextBuffer::moveFile(const QString &sourceFile, const QString &targetFile)
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_ANDROID)
    const int result = std::rename(QFile::encodeName(sourceFile).constData(), QFile::encodeName(targetFile).constData());
    return (result == 0);
#else
    // use racy fallback for windows
    QFile::remove(targetFile);
    return QFile::rename(sourceFile, targetFile);
#endif
}
