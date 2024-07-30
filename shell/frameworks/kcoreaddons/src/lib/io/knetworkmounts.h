/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KNETWORKMOUNTS_H
#define KNETWORKMOUNTS_H

#include <memory>

#include <QObject>
#include <kcoreaddons_export.h>

/**
 * \class KNetworkMounts knetworkmounts.h <KNetworkMounts>
 *
 * Performance control on network mounts.
 *
 * This class provides methods for deciding whether operations
 * on slow network mounts should be performed or not.
 *
 * Configuration is read from a configuration file network_mounts in
 * the user's QStandardPaths::ConfigLocation. This file can be filled by using
 * the network mounts performance configuration module or directly via @ref setEnabled,
 * @ref setPaths, @ref addPath and @ref setOption
 * @code
 *   KNetworkMounts::self()->setEnabled(true);
 *   KNetworkMounts::self()->setOption(KNetworkMounts::LowSideEffectsOptimizations, true);
 *   KNetworkMounts::self()->addPath(path1, KNetworkMounts::NfsPaths);
 *   KNetworkMounts::self()->addPath(path2, KNetworkMounts::NfsPaths);
 *   KNetworkMounts::self()->setPaths(listOfPaths, KNetworkMounts::SmbPaths);
 * @endcode
 *
 * Use KNetworkMounts like this to check if the given url is on a
 * configured slow path and the KNetworkMountOption LowSideEffectsOptimizations
 * is enabled:
 * @code
 *   if (KNetworkMounts::self()->isOptionEnabledForPath(url.toLocalFile(),
 *       KNetworkMounts::LowSideEffectsOptimizations))
 *   {
 *       // skip operations which are slow on the given url if
 *       // KNetworkMountOption LowSideEffectsOptimizations is enabled
 *   } else {
 *       // given url is not configured being slow or the KNetworkMountOption
 *       // LowSideEffectsOptimizations is not enabled
 *   }
 * @endcode
 *
 * If called for the first time, this creates a singleton instance and reads
 * the config file. Subsequent calls just use this instance without reading
 * the config file again.
 *
 * @author Robert Hoffmann <robert@roberthoffmann.de>
 *
 * @since 5.85
 **/
class KCOREADDONS_EXPORT KNetworkMounts : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns (and creates if necessary) the singleton instance
     *
     * @return the singleton instance
     */
    static KNetworkMounts *self();

    /**
     * The KNetworkMountOption enum
     *
     * Uses are:
     */
    enum KNetworkMountOption {
        LowSideEffectsOptimizations, ///< Don't run KDiskFreeSpaceInfo if slow path.<br>
                                     ///< Don't check for manually mounted drives.<br>
                                     ///< Don't check with QFileInfo::isWritable if it is writable, if not yet known, return true.<br>
                                     ///< Don't check with QFileInfo::isReadable if it is readable, return false.<br>
                                     ///< Don't check for desktop files just return false.<br>
                                     ///< Ignore .hidden files on slow paths.<br>
                                     ///< Don't read mime comment from .desktop or .directory files.<br>
                                     ///< Don't get the size with QFileInfo::size, just return 0, if not yet known.<br>
                                     ///< Don't determine mime type from file content, use file extension.<br>
                                     ///< Don't check for desktop files just return false.<br>
                                     ///< Don't call KFileSystemType::fileSystemType to check if the filesystem is slow, just return true.<br>
                                     ///< Don't count files/directories in subdirectories.<br>
                                     ///< Don't calculate sizes of subdirectories.<br>
                                     ///< Avoid check for dir at Kate startup
        MediumSideEffectsOptimizations, ///< Don't return project for dir, avoid QFileInfo().absoluteDir()<br>
                                        ///< Don't search for .kateconfig recursively<br>
                                        ///< Ignore recent files on slow paths
        StrongSideEffectsOptimizations, ///< Turn off symbolic link resolution
        KDirWatchDontAddWatches, ///< Disables dir watching completely for slow paths, avoids stat() calls on added dirs and subdirs
        SymlinkPathsUseCache ///< Cache resolved symlink paths
    };
    Q_ENUM(KNetworkMountOption)

    /**
     * The KNetworkMountsType enum
     */
    enum KNetworkMountsType {
        NfsPaths, ///< NFS paths
        SmbPaths, ///< SMB paths
        SymlinkDirectory, ///< Paths to directories which contain symbolic links to network mounts
        SymlinkToNetworkMount, ///< Paths which are symbolic links to network mounts
        Any ///< Any slow path type. Do not use with @ref setPaths or @ref addPath
    };
    Q_ENUM(KNetworkMountsType)

    /**
     * Query if @p path is configured to be a slow path of type @p type
     *
     * @param path the path to query
     * @param type the type to query. If omitted, any type matches
     * @return @c true if @p path is a configured slow path of type @p type
     *
     * This function is also used to determine the filesystem type in @ref KFileSystemType::fileSystemType
     * (KFileSystemType::Smb or KFileSystemType::Nfs) without an expensive call to stafs(). For this
     * to work the types of paths need to be correctly assigned in @ref setPath or @ref addPath
     */
    bool isSlowPath(const QString &path, KNetworkMountsType type = Any);

    /**
     * Query if @p path is configured to be a slow path and @p option is enabled
     *
     * @param path the path to query
     * @param option the option to query
     * @return @c true if @p path is a configured slow path and option @p option is enabled
     */
    bool isOptionEnabledForPath(const QString &path, KNetworkMountOption option);

    /**
     * Query if the performance optimizations are switched on
     *
     * @return @c true if on, @c false otherwise
     */
    bool isEnabled() const;

    /**
     * Switch the performance optimizations on or off
     *
     * @param value the value to set
     */
    void setEnabled(bool value);

    /**
     * Query a performance option
     *
     * @param option the option to query
     * @param defaultValue the value to return if the option is not configured
     * @return @c true if option is on, @c false if not
     * @see KNetworkMountOption
     */
    bool isOptionEnabled(const KNetworkMountOption option, const bool defaultValue = false) const;

    /**
     * Switch a performance option on or off
     *
     * @param option the option to change
     * @param value the value to set
     * @see KNetworkMountOption
     */
    void setOption(const KNetworkMountOption option, const bool value);

    /**
     * Query the configured paths for which optimizations are to take place
     *
     * @return a list of paths
     */
    QStringList paths(KNetworkMountsType type = Any) const;

    /**
     * Set the paths for which optimizations are to take place
     *
     * @param paths the paths to set
     * @param type the type of paths. Do not use @ref Any
     * @see KNetworkMountsType
     */
    void setPaths(const QStringList &paths, KNetworkMountsType type);

    /**
     * Add a path for which optimizations are to take place
     *
     * @param path the path to add
     * @param type the type of the path. Do not use @ref Any
     * @see KNetworkMountsType
     */
    void addPath(const QString &path, KNetworkMountsType type);

    /**
     * Resolves a @p path that may contain symbolic links to mounted network shares.
     *
     * A symlink path is either a directory which contains symbolic links to slow network mounts
     * (@ref SymlinkDirectory) or a direct symbolic link to a slow network mount (@ref
     * SymlinkToNfsOrSmbPaths).
     *
     * Example:
     * There are some Samba shares mounted below /mnt. These are @ref paths of type @ref SmbPaths
     * @code
     * /mnt/server1/share1
     * /mnt/server1/share2
     * /mnt/server2/share3
     * @endcode
     *
     * A (logged in) user may have symbolic links to them in his home directory below netshares. The
     * directory /home/user/netshares is a @ref SymlinkDirectory:
     * @code
     * /home/user/netshares/share1 -> /mnt/server1/share1
     * /home/user/netshares/share2 -> /mnt/server1/share2
     * /home/user/netshares/share3 -> /mnt/server2/share3
     * @endcode
     *
     * There is a direct symbolic link from /home/user/share1 to /mnt/server1/share1. This is of type
     * @ref SymlinkToNfsOrSmbPaths:
     * @code
     * /home/user/share1 -> /mnt/server1/share1
     * @endcode
     *
     * Both types of symbolic links from symlink paths to the real mounted shares are resolved even if
     * KNetworkMountOption @ref StrongSideEffectsOptimizations is enabled.

     * If the setup is like above a @p path
     * @code
     * /home/user/netshares/share1/Documents/file.txt
     * @endcode
     *
     * would be resolved to
     * @code
     * /mnt/server1/share1/Documents/file.txt
     * @endcode
     *
     * and a @p path
     * @code
     * /home/user/share1/Documents/file.txt
     * @endcode
     *
     * would also be resolved to
     * @code
     * /mnt/server1/share1/Documents/file.txt
     * @endcode
     *
     * Resolved paths are cached in a hash.
     *
     * @param path the path to resolve
     * @return the resolved path or @p path if @p path is not a symlink path or no symlink found
     * @see KNetworkMountsType
     * @see clearCache
     * @see isSlowPath
     */
    QString canonicalSymlinkPath(const QString &path);

    /**
     * Clears the canonical symlink path cache
     *
     * Call this if directory structures on mounted network drives changed. Don't enable the
     * cache (@ref SymlinkPathsUseCache) if this happens often and the drives are usually accessed
     * via the symlinks. This method exists mainly for the KCM.
     * @see canonicalSymlinkPath
     */
    void clearCache();

    /**
     * Synchronizes to config file
     *
     * QSettings synchronization also takes place automatically at regular intervals and from
     * QSettings destructor, see QSettings::sync() documentation.
     *
     * Calls QSettings::sync()
     */
    void sync();

private:
    /// Creates a new KNetworkMounts object
    KCOREADDONS_NO_EXPORT KNetworkMounts();

    /// Destructor
    KCOREADDONS_NO_EXPORT ~KNetworkMounts() override;

    std::unique_ptr<class KNetworkMountsPrivate> const d;
};

#endif // KNETWORKMOUNTS_H
