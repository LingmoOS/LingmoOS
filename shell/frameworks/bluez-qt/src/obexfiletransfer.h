/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXFILETRANSFER_H
#define BLUEZQT_OBEXFILETRANSFER_H

#include <QObject>

#include "bluezqt_export.h"
#include "obexfiletransferentry.h"

#include <memory>

class QDBusObjectPath;

namespace BluezQt
{
class PendingCall;

/**
 * @class BluezQt::ObexFileTransfer obexfiletransfer.h <BluezQt/ObexFileTransfer>
 *
 * OBEX file transfer.
 *
 * This class represents an OBEX file transfer interface.
 */
class BLUEZQT_EXPORT ObexFileTransfer : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new ObexFileTransfer object.
     *
     * This class will be typically used with a @p path
     * from result of ObexManager::createSession().
     *
     * @param path path of session
     * @param parent
     */
    explicit ObexFileTransfer(const QDBusObjectPath &path, QObject *parent = nullptr);

    /**
     * Destroys an ObexFileTransfer object.
     */
    ~ObexFileTransfer() override;

    /**
     * D-Bus object path of the file transfer session.
     *
     * @return object path of session
     */
    QDBusObjectPath objectPath() const;

    /**
     * Changes the current folder.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param folder folder to be changed
     * @return void pending call
     */
    PendingCall *changeFolder(const QString &folder);

    /**
     * Creates a new folder.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param folder name of new folder
     * @return void pending call
     */
    PendingCall *createFolder(const QString &folder);

    /**
     * Lists a current folder.
     *
     * Possible errors: PendingCall::Failed
     *
     * @return QList<ObexFileTransferEntry> pending call
     */
    PendingCall *listFolder();

    /**
     * Gets the file from the remote device.
     *
     * If an empty @p targetFileName is given, a name will be
     * automatically calculated for the temporary file.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param targetFileName full local path where the file will be saved
     * @param sourceFileName file within the remote device
     * @return ObexTransferPtr pending call
     */
    PendingCall *getFile(const QString &targetFileName, const QString &sourceFileName);

    /**
     * Puts the file to the remote device.
     *
     * If an empty @p targetFileName is given, a name will be
     * automatically calculated for the temporary file.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param sourceFileName full path of the local file
     * @param targetFileName file to be saved within the remote device
     * @return ObexTransferPtr pending call
     */
    PendingCall *putFile(const QString &sourceFileName, const QString &targetFileName);

    /**
     * Copies a file within the remote device.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param sourceFileName source within the remote device
     * @param targetFileName target file within the remote device
     * @return void pending call
     */
    PendingCall *copyFile(const QString &sourceFileName, const QString &targetFileName);

    /**
     * Moves a file within the remote device.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param sourceFileName source file within the remote device
     * @param targetFileName target file within the remote device
     * @return void pending call
     */
    PendingCall *moveFile(const QString &sourceFileName, const QString &targetFileName);

    /**
     * Deletes a file/folder within the remote device.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed
     *
     * @param fileName file within the remote device
     * @return void pending call
     */
    PendingCall *deleteFile(const QString &fileName);

private:
    std::unique_ptr<class ObexFileTransferPrivate> const d;

    friend class ObexFileTransferPrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXFILETRANSFER_H
