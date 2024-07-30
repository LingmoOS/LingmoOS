/*
    SPDX-FileCopyrightText: 2017 KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SECURE_TEXTBUFFER_P_H
#define KATE_SECURE_TEXTBUFFER_P_H

#include <QCryptographicHash>
#include <QObject>
#include <QString>

#include <KAuth/ActionReply>

using namespace KAuth;

/**
 * Class used as KAuth helper binary.
 * It is supposed to be called through KAuth action.
 *
 * It also contains couple of common methods intended to be used
 * directly by TextBuffer as well as from helper binary.
 *
 * This class should only be used by TextBuffer.
 */
class SecureTextBuffer : public QObject
{
    Q_OBJECT

public:
    /**
     * Common helper method
     */
    static void setOwner(const int filedes, const uint ownerId, const uint groupId);

    static const QCryptographicHash::Algorithm checksumAlgorithm = QCryptographicHash::Algorithm::Sha512;

private:
    /**
     * Saves file contents using sets permissions.
     */
    static bool saveFileInternal(const QString &sourceFile, const QString &targetFile, const QByteArray &checksum, const uint ownerId, const uint groupId);

    static bool moveFile(const QString &sourceFile, const QString &targetFile);

public Q_SLOTS:
    /**
     * KAuth action to perform both prepare or move work based on given parameters.
     * We keep this code in one method to prevent multiple KAuth user queries during one save action.
     */
    static ActionReply savefile(const QVariantMap &args);
};

#endif
