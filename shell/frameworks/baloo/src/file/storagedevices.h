/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef STORAGE_DEVICES_H
#define STORAGE_DEVICES_H

#include <QObject>
#include <QHash>

#include <Solid/Device>

namespace Baloo
{

/**
 * The removable media cache
 * media that are supported by Baloo.
 */
class StorageDevices : public QObject
{
    Q_OBJECT

public:
    explicit StorageDevices(QObject* parent = nullptr);
    ~StorageDevices();

    class Entry
    {
    public:
        Entry();
        explicit Entry(const Solid::Device& device);

        Solid::Device device() const {
            return m_device;
        }

        bool isMounted() const;
        QString mountPath() const;

        /**
         * Returns true if Baloo should be indexing this
         * Currently we only index permanently mounted media
         */
        bool isUsable() const;

        QString udi() const {
            return m_device.udi();
        }
    private:
        Solid::Device m_device;
    };

    QList<Entry> allMedia() const;
    bool isEmpty() const;

Q_SIGNALS:
    void deviceAdded(const Baloo::StorageDevices::Entry* entry);
    void deviceRemoved(const Baloo::StorageDevices::Entry* entry);
    void deviceAccessibilityChanged(const Baloo::StorageDevices::Entry* entry);

private Q_SLOTS:
    void slotSolidDeviceAdded(const QString& udi);
    void slotSolidDeviceRemoved(const QString& udi);
    void slotAccessibilityChanged(bool accessible, const QString& udi);

private:
    void initCacheEntries();

    Entry* createCacheEntry(const Solid::Device& dev);

    /// maps Solid UDI to Entry
    QHash<QString, Entry> m_metadataCache;
};

} // namespace Baloo

Q_DECLARE_TYPEINFO(Baloo::StorageDevices::Entry, Q_RELOCATABLE_TYPE);

#endif // STORAGE_DEVICES_H
