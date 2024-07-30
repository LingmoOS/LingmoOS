/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXFILETRANSFERENTRY_H
#define BLUEZQT_OBEXFILETRANSFERENTRY_H

#include <QSharedPointer>
#include <QString>

#include "bluezqt_export.h"

namespace BluezQt
{
/**
 * @class BluezQt::ObexFileTransferEntry obexfiletransferentry.h <BluezQt/ObexFileTransferEntry>
 *
 * OBEX file transfer entry.
 *
 * This class represents an entry in remote file system.
 */
class BLUEZQT_EXPORT ObexFileTransferEntry
{
public:
    /** Type of entry. */
    enum Type {
        /** Indicates that the entry is a file. */
        File,
        /** Indicates that the entry is a folder. */
        Folder,
        /** Indicates that the entry is invalid. */
        Invalid,
    };

    /**
     * Creates a new invalid ObexFileTransferEntry object.
     */
    explicit ObexFileTransferEntry();

    /**
     * Destroys an ObexFileTransferEntry object.
     */
    virtual ~ObexFileTransferEntry();

    /**
     * Copy constructor.
     *
     * @param other
     */
    ObexFileTransferEntry(const ObexFileTransferEntry &other);

    /**
     * Copy assignment operator.
     *
     * @param other
     */
    ObexFileTransferEntry &operator=(const ObexFileTransferEntry &other);

    /**
     * Returns whether the entry is valid.
     *
     * This only checks if type() != Invalid.
     *
     * @return true if entry is valid
     */
    bool isValid() const;

    /**
     * Returns a name of the entry.
     *
     * @return name of entry
     */
    QString name() const;

    /**
     * Returns a label of the entry.
     *
     * @return label of entry
     */
    QString label() const;

    /**
     * Returns a type of the entry.
     *
     * Entry can be either a file or folder.
     *
     * @return type of entry
     */
    Type type() const;

    /**
     * Returns a size of the entry.
     *
     * Size is a number of items in the folder or file size in bytes.
     *
     * @return size of entry
     */
    quint64 size() const;

    /**
     * Returns a permissions of the entry.
     *
     * @return permissions of entry
     */
    QString permissions() const;

    /**
     * Returns a memory type where the entry is stored.
     *
     * @return memory type
     */
    QString memoryType() const;

    /**
     * Returns a modification time of the entry.
     *
     * @return modification time of entry
     */
    QDateTime modificationTime() const;

private:
    BLUEZQT_NO_EXPORT explicit ObexFileTransferEntry(const QVariantMap &properties);

    QSharedPointer<class ObexFileTransferEntryPrivate> d;

    friend class PendingCallPrivate;
};

} // namespace BluezQt

Q_DECLARE_METATYPE(BluezQt::ObexFileTransferEntry)

#endif // BLUEZQT_OBEXFILETRANSFERENTRY_H
