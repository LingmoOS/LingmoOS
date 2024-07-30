/*
    SPDX-FileCopyrightText: 2007-2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGE_H
#define KPACKAGE_PACKAGE_H

#include <QCryptographicHash>
#include <QMetaType>
#include <QStringList>
#include <QUrl>

#include <KPluginMetaData>

#include <kpackage/package_export.h>

#include <KJob>

namespace KPackage
{
/**
 * @class Package kpackage/package.h <KPackage/Package>
 *
 * @short object representing an installed package
 *
 * Package defines what is in a package and provides easy access to the contents.
 *
 * To define a package, one might write the following code:
 *
 @code
    Package package;

    package.addDirectoryDefinition("images", "pics/");
    package.setMimeTypes("images", QStringList{"image/svg", "image/png", "image/jpeg"});

    package.addDirectoryDefinition("scripts", "code/");
    package.setMimeTypes("scripts", QStringList{"text/\*"});

    package.addFileDefinition("mainscript", "code/main.js");
    package.setRequired("mainscript", true);
 @endcode
 * One may also choose to create a subclass of PackageStructure and include the setup
 * in the constructor.
 *
 * Either way, Package creates a self-documenting contract between the packager and
 * the application without exposing package internals such as actual on-disk structure
 * of the package or requiring that all contents be explicitly known ahead of time.
 *
 * Subclassing PackageStructure does have provide a number of potential const benefits:
 *    * the package can be notified of path changes via the virtual pathChanged() method
 *    * the subclass may implement mechanisms to install and remove packages using the
 *      virtual install and uninstall methods
 *    * subclasses can be compiled as plugins for easy re-use
 **/
// TODO: write documentation on USING a package

class PackagePrivate;
class PackageStructure;

class KPACKAGE_EXPORT Package
{
public:
    /**
     * Default constructor
     *
     * @param structure if a null pointer is passed in, this will creates an empty (invalid) Package;
     * otherwise the structure is allowed to set up the Package's initial layout
     */
    explicit Package(PackageStructure *structure = nullptr);

    /**
     * Copy constructor
     */
    Package(const Package &other);

    virtual ~Package();

    /**
     * Assignment operator
     */
    Package &operator=(const Package &rhs);

    /**
     * @return true if this package has a valid PackageStructure associatedw it with it.
     * A package may not be valid, but have a valid structure. Useful when dealing with
     * Package objects in a semi-initialized state (e.g. before calling setPath())
     * @since 5.1
     */
    bool hasValidStructure() const;

    /**
     * @return true if all the required components exist
     **/
    bool isValid() const;

    /**
     * Sets the path to the root of this package
     * @param path an absolute path, or a relative path to the default package root
     */
    void setPath(const QString &path);

    /**
     * @return the path to the root of this particular package
     */
    const QString path() const;

    /**
     * Get the path to a given file based on the key and an optional filename.
     * Example: finding the main script in a scripting package:
     *      filePath("mainscript")
     *
     * Example: finding a specific image in the images directory:
     *      filePath("images", "myimage.png")
     *
     * @param key the key of the file type to look for,
     * @param filename optional name of the file to locate within the package
     * @return path to the file on disk. QString() if not found.
     **/
    QString filePath(const QByteArray &key, const QString &filename = QString()) const;

    /**
     * Get the url to a given file based on the key and an optional filename, is the file:// or qrc:// format
     * Example: finding the main script in a scripting package:
     *      filePath("mainscript")
     *
     * Example: finding a specific image in the images directory:
     *      filePath("images", "myimage.png")
     *
     * @param key the key of the file type to look for,
     * @param filename optional name of the file to locate within the package
     * @return path to the file on disk. QUrl() if not found.
     * @since 5.41
     **/
    QUrl fileUrl(const QByteArray &key, const QString &filename = QString()) const;

    /**
     * Get the list of files of a given type.
     *
     * @param fileType the type of file to look for, as defined in the
     *               package structure.
     * @return list of files by name, suitable for passing to filePath
     **/
    QStringList entryList(const QByteArray &key) const;

    /**
     * @return true if the item at path exists and is required
     **/
    bool isRequired(const QByteArray &key) const;

    /**
     * @return the mimeTypes associated with the path, if any
     **/
    QStringList mimeTypes(const QByteArray &key) const;

    /**
     * @return the prefix paths inserted between the base path and content entries, in order of priority.
     *         When searching for a file, all paths will be tried in order.
     */
    QStringList contentsPrefixPaths() const;

    /**
     * @return preferred package root. This defaults to kpackage/generic/
     */
    QString defaultPackageRoot() const;

    /**
     * @return true if paths/symlinks outside the package itself should be followed.
     * By default this is set to false for security reasons.
     */
    bool allowExternalPaths() const;

    /**
     * Sets the metadata for the KPackage. This overwrites the current metadata.
     * This should be used in case a kpackage gets loaded by name, based
     * on the path a C++ plugin which has embedded metadata.
     * @since 5.88
     */
    void setMetadata(const KPluginMetaData &data);

    /**
     * @return the package metadata object.
     */
    KPluginMetaData metadata() const;

    /**
     * @return a hash digest of the contents of the package in hexadecimal form
     * @since 5.21
     */
    QByteArray cryptographicHash(QCryptographicHash::Algorithm algorithm) const;

    /**
     * Adds a directory to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     * If an entry with the given key already exists, the path
     * is added to it as a search alternative.
     *
     * @param key  used as an internal label for this directory
     * @param path the path within the package for this directory
     */
    void addDirectoryDefinition(const QByteArray &key, const QString &path);

    /**
     * Adds a file to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     * If an entry with the given key already exists, the path
     * is added to it as a search alternative.
     *
     * @param key  used as an internal label for this file
     * @param path the path within the package for this file
     */
    void addFileDefinition(const QByteArray &key, const QString &path);

    /**
     * Removes a definition from the structure of the package.
     * @param key the internal label of the file or directory to remove
     */
    void removeDefinition(const QByteArray &key);

    /**
     * Sets whether or not a given part of the structure is required or not.
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param required true if this entry is required, false if not
     */
    void setRequired(const QByteArray &key, bool required);

    /**
     * Defines the default mimeTypes for any definitions that do not have
     * associated mimeTypes. Handy for packages with only one or predominantly
     * one file type.
     *
     * @param mimeTypes a list of mimeTypes
     **/
    void setDefaultMimeTypes(const QStringList &mimeTypes);

    /**
     * Define mimeTypes for a given part of the structure
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param mimeTypes a list of mimeTypes
     **/
    void setMimeTypes(const QByteArray &key, const QStringList &mimeTypes);

    /**
     * Sets the prefixes that all the contents in this package should
     * appear under. This defaults to "contents/" and is added automatically
     * between the base path and the entries as defined by the package
     * structure. Multiple entries can be added.
     * In this case each file request will be searched in all prefixes in order,
     * and the first found will be returned.
     *
     * @param prefix paths the directory prefix to use
     */
    void setContentsPrefixPaths(const QStringList &prefixPaths);

    /**
     * Sets whether or not external paths/symlinks can be followed by a package
     * @param allow true if paths/symlinks outside of the package should be followed,
     *             false if they should be rejected.
     */
    void setAllowExternalPaths(bool allow);

    /**
     * Sets preferred package root.
     */
    void setDefaultPackageRoot(const QString &packageRoot);

    /**
     * Sets the fallback package root path
     * If a file won't be found in this package, it will search it in the package
     * with the same structure identified by path
     * It is intended to be used by the packageStructure
     * @param path package root path @see setPath
     */
    void setFallbackPackage(const KPackage::Package &package);

    /**
     * @return The fallback package root path
     */
    KPackage::Package fallbackPackage() const;

    // Content structure description methods
    /**
     * @return all directories registered as part of this Package's structure
     */
    QList<QByteArray> directories() const;

    /**
     * @return all directories registered as part of this Package's required structure
     */
    QList<QByteArray> requiredDirectories() const;

    /**
     * @return all files registered as part of this Package's structure
     */
    QList<QByteArray> files() const;

    /**
     * @return all files registered as part of this Package's required structure
     */
    QList<QByteArray> requiredFiles() const;

private:
    QExplicitlySharedDataPointer<PackagePrivate> d;
    friend class PackagePrivate;
};

}

Q_DECLARE_METATYPE(KPackage::Package)
#endif
