/*
    SPDX-FileCopyrightText: 2008, 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SETTINGS_CONNECTION_H
#define NETWORKMANAGERQT_SETTINGS_CONNECTION_H

#include "connectionsettings.h"
#include "generictypes.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

namespace NetworkManager
{
class ConnectionPrivate;

/**
 * This class represents a single network connection configuration.
 */
class NETWORKMANAGERQT_EXPORT Connection : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<Connection> Ptr;
    typedef QList<Ptr> List;

    /**
     * Constructs a connection object for the given path
     */
    explicit Connection(const QString &path, QObject *parent = nullptr);
    ~Connection() override;

    /**
     * Returns if this connection is valid
     */
    bool isValid() const;

    /**
     * Returns the unique identifier of this connection
     */
    QString uuid() const;

    /**
     * Returns the path (DBus) of this connection
     */
    QString path() const;

    /**
     * Returns the name of this connection
     */
    QString name() const;
    /**
     * If set, indicates that the in-memory state of the
     * connection does not match the on-disk state. This flag
     * will be set when updateUnsaved() is called or when any
     * connection details change, and cleared when the connection
     * is saved to disk via save() or from internal operations.
     *
     * @since 0.9.9.0
     */
    bool isUnsaved() const;
    /**
     * Returns the settings of this connection
     */
    ConnectionSettings::Ptr settings();

    /**
     * Retrieves this connections's secrets (passwords and / or encryption keys).
     *
     * @param setting the setting identifier.
     */
    QDBusPendingReply<NMVariantMapMap> secrets(const QString &setting);

    /**
     * Update the connection with new @p settings and properties, replacing all previous settings and properties.
     * Secrets may be part of the update request, and will be either stored in persistent storage or given to a Secret Agent for storage,
     * depending on the request.
     */
    QDBusPendingReply<> update(const NMVariantMapMap &settings);
    /**
     * Update the connection with new @p settings and properties (replacing
     * all previous settings and properties) but do not immediately save
     * the connection to disk. Secrets may be part of the update request
     * and may sent to a Secret Agent for storage, depending on the
     * flags associated with each secret.
     *
     * Use the save() method to save these changes to disk. Note
     * that unsaved changes will be lost if the connection is
     * reloaded from disk (either automatically on file change or
     * due to an explicit reloadConnections() call).
     *
     * @since 0.9.9.0
     */
    QDBusPendingReply<> updateUnsaved(const NMVariantMapMap &settings);

    /**
     * Saves a "dirty" connection (that had previously been
     * updated with updateUnsaved()) to persistent storage.
     *
     * @since 0.9.9.0
     */
    QDBusPendingReply<> save();

    /**
     * Clear the secrets belonging to this network connection profile.
     * @since 5.8.0
     */
    QDBusPendingReply<> clearSecrets();

    /**
     * Removes the connection from NetworkManager database,
     * this operation does not ask for confirmation but
     * a policykit rule might prevent it from being removed
     * without the proper password.
     */
    QDBusPendingReply<> remove();

Q_SIGNALS:
    /**
     * Emitted when the connection settings changes
     */
    void updated();

    /**
     * Emitted when the connection was removed
     * @param path connections's path.
     */
    void removed(const QString &path);
    /**
     * Emitted when the connection unsaved state changes
     */
    void unsavedChanged(bool unsaved);

private:
    Q_DECLARE_PRIVATE(Connection)

    ConnectionPrivate *const d_ptr;
};

}
#endif // CONNECTION_H
