/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2022 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KMEMORYINFO_H
#define KMEMORYINFO_H

#include <QSharedDataPointer>

#include <kcoreaddons_export.h>

class KMemoryInfoPrivate;

/**
 * @brief The KMemoryInfo class provides an interface to get memory information (RAM/SWAP).
 *
 * To use the class, simply create an instance.
 * \code
 * KMemoryInfo memInfo;
 * if (!memInfo.isNull()) {
 *     ...
 * }
 * \endcode
 *
 * @since 5.95
 */
class KCOREADDONS_EXPORT KMemoryInfo
{
public:
    ~KMemoryInfo();

    /**
     * @brief KMemoryInfo
     * Constructs a class with a snapshot of the state of the memory. If an error occurs, a null object is returned.
     * @sa isNull.
     */
    KMemoryInfo();

    /**
     * @brief KMemoryInfo
     * Constructs a copy of the other memoryinfo.
     */
    KMemoryInfo(const KMemoryInfo &other);

    /**
     * @brief operator =
     * Makes a copy of the other memoryinfo and returns a reference to the copy.
     */
    KMemoryInfo &operator=(const KMemoryInfo &other);

    /**
     * @brief operator ==
     * @return @c true if this memoryinfo is equal to the other memoryinfo, otherwise @c false.
     */
    bool operator==(const KMemoryInfo &other) const;

    /**
     * @brief operator !=
     * @return @c true if this memoryinfo is different from the other memoryinfo, otherwise @c false.
     */
    bool operator!=(const KMemoryInfo &other) const;

    /**
     * @brief isNull
     * @return @c true if the class is null, otherwise @c false.
     */
    bool isNull() const;

    /**
     * @brief totalPhysical
     * @return The total system RAM in bytes.
     */
    quint64 totalPhysical() const;

    /**
     * @brief freePhysical
     *
     * The free memory is the amount of free RAM as reported by the operating system.
     * This value is often tainted with caches and buffers used by the operating system, resulting in a low value.
     * @note Don't use this value to determine if you have enough RAM for your data.
     * @return The free RAM reported by OS in bytes.
     * @sa availablePhysical.
     */
    quint64 freePhysical() const;

    /**
     * @brief availablePhysical
     *
     * The available memory is the free RAM without considering caches and buffers allocated by the operating system.
     * @note You should always use this value to check if there is enough RAM for your data.
     * @return The memory available to the processes in bytes.
     * @sa freePhysical.
     */
    quint64 availablePhysical() const;

    /**
     * @brief cached
     * @return The size of RAM used as cache in bytes.
     */
    quint64 cached() const;

    /**
     * @brief buffers
     * @return The size of RAM used as buffers in bytes. This value can be zero.
     */
    quint64 buffers() const;

    /**
     * @brief totalSwapFile
     * @return The size of swap file in bytes.
     * @note On an operating system where the paging file is dynamically allocated, this value can be zero when no memory pages are swapped.
     */
    quint64 totalSwapFile() const;

    /**
     * @brief freeSwapFile
     * @return The free swap size in bytes.
     */
    quint64 freeSwapFile() const;

private:
    /**
     * @brief update Refresh the memory information.
     * @return @c true on success, otherwise @c false.
     */
    KCOREADDONS_NO_EXPORT bool update();

    QSharedDataPointer<KMemoryInfoPrivate> d;
};

#endif // KMEMORYINFO_H
