/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "package.h"

#include <QResource>
#include <qtemporarydir.h>

#include "kpackage_debug.h"
#include <KArchive>
#include <KLocalizedString>
#include <KTar>
#include <kzip.h>

#include "config-package.h"

#include <QMimeDatabase>
#include <QStandardPaths>

#include "packageloader.h"
#include "packagestructure.h"
#include "private/package_p.h"
#include "private/packageloader_p.h"

namespace KPackage
{
Package::Package(PackageStructure *structure)
    : d(new PackagePrivate())
{
    d->structure = structure;

    if (d->structure) {
        addFileDefinition("metadata", QStringLiteral("metadata.json"));
        d->structure.data()->initPackage(this);
    }
}

Package::Package(const Package &other)
    : d(other.d)
{
}

Package::~Package() = default;

Package &Package::operator=(const Package &rhs)
{
    if (&rhs != this) {
        d = rhs.d;
    }

    return *this;
}

bool Package::hasValidStructure() const
{
    qWarning() << d->structure << requiredFiles();
    return d->structure;
}

bool Package::isValid() const
{
    if (!d->structure) {
        return false;
    }

    // Minimal packages with no metadata *are* supposed to be possible
    // so if !metadata().isValid() go ahead
    if (metadata().isValid() && metadata().value(QStringLiteral("isHidden"), QStringLiteral("false")) == QLatin1String("true")) {
        return false;
    }

    if (d->checkedValid) {
        return d->valid;
    }

    const QString rootPath = d->tempRoot.isEmpty() ? d->path : d->tempRoot;
    if (rootPath.isEmpty()) {
        return false;
    }

    d->valid = true;

    // search for the file in all prefixes and in all possible paths for each prefix
    // even if it's a big nested loop, usually there is one prefix and one location
    // so shouldn't cause too much disk access
    for (auto it = d->contents.cbegin(), end = d->contents.cend(); it != end; ++it) {
        if (it.value().required && filePath(it.key()).isEmpty()) {
            qCWarning(KPACKAGE_LOG) << "Could not find required" << (it.value().directory ? "directory" : "file") << it.key() << "for package" << path()
                                    << "should be" << it.value().paths;
            d->valid = false;
            break;
        }
    }

    return d->valid;
}

bool Package::isRequired(const QByteArray &key) const
{
    auto it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return false;
    }

    return it.value().required;
}

QStringList Package::mimeTypes(const QByteArray &key) const
{
    auto it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QStringList();
    }

    if (it.value().mimeTypes.isEmpty()) {
        return d->mimeTypes;
    }

    return it.value().mimeTypes;
}

QString Package::defaultPackageRoot() const
{
    return d->defaultPackageRoot;
}

void Package::setDefaultPackageRoot(const QString &packageRoot)
{
    d.detach();
    d->defaultPackageRoot = packageRoot;
    if (!d->defaultPackageRoot.isEmpty() && !d->defaultPackageRoot.endsWith(QLatin1Char('/'))) {
        d->defaultPackageRoot.append(QLatin1Char('/'));
    }
}

void Package::setFallbackPackage(const KPackage::Package &package)
{
    if ((d->fallbackPackage && d->fallbackPackage->path() == package.path() && d->fallbackPackage->metadata() == package.metadata()) ||
        // can't be fallback of itself
        (package.path() == path() && package.metadata() == metadata()) || d->hasCycle(package)) {
        return;
    }

    d->fallbackPackage = std::make_unique<Package>(package);
}

KPackage::Package Package::fallbackPackage() const
{
    if (d->fallbackPackage) {
        return (*d->fallbackPackage);
    } else {
        return Package();
    }
}

bool Package::allowExternalPaths() const
{
    return d->externalPaths;
}

void Package::setMetadata(const KPluginMetaData &data)
{
    Q_ASSERT(data.isValid());
    d->metadata = data;
}

void Package::setAllowExternalPaths(bool allow)
{
    d.detach();
    d->externalPaths = allow;
}

KPluginMetaData Package::metadata() const
{
    // qCDebug(KPACKAGE_LOG) << "metadata: " << d->path << filePath("metadata");
    if (!d->metadata && !d->path.isEmpty()) {
        const QString metadataPath = filePath("metadata", QStringLiteral("metadata.json"));

        if (!metadataPath.isEmpty()) {
            d->createPackageMetadata(metadataPath);
        } else {
            // d->path might still be a file, if its path has a trailing /,
            // the fileInfo lookup will fail, so remove it.
            QString p = d->path;
            if (p.endsWith(QLatin1Char('/'))) {
                p.chop(1);
            }
            QFileInfo fileInfo(p);

            if (fileInfo.isDir()) {
                d->createPackageMetadata(d->path);
            } else if (fileInfo.exists()) {
                d->path = fileInfo.canonicalFilePath();
                d->tempRoot = d->unpack(p);
            }
        }
    }

    // Set a dummy KPluginMetaData object, this way we don't try to do the expensive
    // search for the metadata again if none of the paths have changed
    if (!d->metadata) {
        d->metadata = KPluginMetaData();
    }

    return d->metadata.value();
}

QString PackagePrivate::unpack(const QString &filePath)
{
    KArchive *archive = nullptr;
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(filePath);

    if (mimeType.inherits(QStringLiteral("application/zip"))) {
        archive = new KZip(filePath);
    } else if (mimeType.inherits(QStringLiteral("application/x-compressed-tar")) || //
               mimeType.inherits(QStringLiteral("application/x-gzip")) || //
               mimeType.inherits(QStringLiteral("application/x-tar")) || //
               mimeType.inherits(QStringLiteral("application/x-bzip-compressed-tar")) || //
               mimeType.inherits(QStringLiteral("application/x-xz")) || //
               mimeType.inherits(QStringLiteral("application/x-lzma"))) {
        archive = new KTar(filePath);
    } else {
        // qCWarning(KPACKAGE_LOG) << "Could not open package file, unsupported archive format:" << filePath << mimeType.name();
    }
    QString tempRoot;
    if (archive && archive->open(QIODevice::ReadOnly)) {
        const KArchiveDirectory *source = archive->directory();
        QTemporaryDir tempdir;
        tempdir.setAutoRemove(false);
        tempRoot = tempdir.path() + QLatin1Char('/');
        source->copyTo(tempRoot);

        if (!QFile::exists(tempdir.path() + QLatin1String("/metadata.json"))) {
            // search metadata.json, the zip file might have the package contents in a subdirectory
            QDir unpackedPath(tempdir.path());
            const auto entries = unpackedPath.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const auto &pack : entries) {
                if (QFile::exists(pack.filePath() + QLatin1String("/metadata.json"))) {
                    tempRoot = pack.filePath() + QLatin1Char('/');
                }
            }
        }

        createPackageMetadata(tempRoot);
    } else {
        // qCWarning(KPACKAGE_LOG) << "Could not open package file:" << path;
    }

    delete archive;
    return tempRoot;
}

bool PackagePrivate::isInsidePackageDir(const QString &canonicalPath) const
{
    // make sure that the target file is actually inside the package dir to prevent
    // path traversal using symlinks or "../" path segments

    // make sure we got passed a valid path
    Q_ASSERT(QFileInfo::exists(canonicalPath));
    Q_ASSERT(QFileInfo(canonicalPath).canonicalFilePath() == canonicalPath);
    // make sure that the base path is also canonical
    // this was not the case until 5.8, making this check fail e.g. if /home is a symlink
    // which in turn would make plasmashell not find the .qml files
    // installed package
    if (tempRoot.isEmpty()) {
        Q_ASSERT(QDir(path).exists());
        Q_ASSERT(path == QStringLiteral("/") || QDir(path).canonicalPath() + QLatin1Char('/') == path);

        if (canonicalPath.startsWith(path)) {
            return true;
        }
        // temporary compressed package
    } else {
        Q_ASSERT(QDir(tempRoot).exists());
        Q_ASSERT(tempRoot == QStringLiteral("/") || QDir(tempRoot).canonicalPath() + QLatin1Char('/') == tempRoot);

        if (canonicalPath.startsWith(tempRoot)) {
            return true;
        }
    }
    qCWarning(KPACKAGE_LOG) << "Path traversal attempt detected:" << canonicalPath << "is not inside" << path;
    return false;
}

QString Package::filePath(const QByteArray &fileType, const QString &filename) const
{
    if (!d->valid && d->checkedValid) { // Don't check the validity here, because we'd have infinite recursion
        QString result = d->fallbackFilePath(fileType, filename);
        if (result.isEmpty()) {
            // qCDebug(KPACKAGE_LOG) << fileType << "file with name" << filename
            //    << "was not found in package with path" << d->path;
        }
        return result;
    }

    const QString discoveryKey(QString::fromUtf8(fileType) + filename);
    const auto path = d->discoveries.value(discoveryKey);
    if (!path.isEmpty()) {
        return path;
    }

    QStringList paths;

    if (!fileType.isEmpty()) {
        const auto contents = d->contents.constFind(fileType);
        if (contents == d->contents.constEnd()) {
            // qCDebug(KPACKAGE_LOG) << "package does not contain" << fileType << filename;
            return d->fallbackFilePath(fileType, filename);
        }

        paths = contents->paths;

        if (paths.isEmpty()) {
            // qCDebug(KPACKAGE_LOG) << "no matching path came of it, while looking for" << fileType << filename;
            d->discoveries.insert(discoveryKey, QString());
            return d->fallbackFilePath(fileType, filename);
        }
    } else {
        // when filetype is empty paths is always empty, so try with an empty string
        paths << QString();
    }

    // Nested loop, but in the medium case resolves to just one iteration
    //     qCDebug(KPACKAGE_LOG) << "prefixes:" << d->contentsPrefixPaths.count() << d->contentsPrefixPaths;
    for (const QString &contentsPrefix : std::as_const(d->contentsPrefixPaths)) {
        QString prefix;
        // We are an installed package
        if (d->tempRoot.isEmpty()) {
            prefix = fileType == "metadata" ? d->path : (d->path + contentsPrefix);
            // We are a compressed package temporarily uncompressed in /tmp
        } else {
            prefix = fileType == "metadata" ? d->tempRoot : (d->tempRoot + contentsPrefix);
        }

        for (const QString &path : std::as_const(paths)) {
            QString file = prefix + path;

            if (!filename.isEmpty()) {
                file.append(QLatin1Char('/') + filename);
            }

            QFileInfo fi(file);
            if (fi.exists()) {
                if (d->externalPaths) {
                    // qCDebug(KPACKAGE_LOG) << "found" << file;
                    d->discoveries.insert(discoveryKey, file);
                    return file;
                }

                // ensure that we don't return files outside of our base path
                // due to symlink or ../ games
                if (d->isInsidePackageDir(fi.canonicalFilePath())) {
                    // qCDebug(KPACKAGE_LOG) << "found" << file;
                    d->discoveries.insert(discoveryKey, file);
                    return file;
                }
            }
        }
    }

    // qCDebug(KPACKAGE_LOG) << fileType << filename << "does not exist in" << prefixes << "at root" << d->path;
    return d->fallbackFilePath(fileType, filename);
}

QUrl Package::fileUrl(const QByteArray &fileType, const QString &filename) const
{
    QString path = filePath(fileType, filename);
    // construct a qrc:/ url or a file:/ url, the only two protocols supported
    if (path.startsWith(QStringLiteral(":"))) {
        return QUrl(QStringLiteral("qrc") + path);
    } else {
        return QUrl::fromLocalFile(path);
    }
}

QStringList Package::entryList(const QByteArray &key) const
{
    if (!d->valid) {
        return QStringList();
    }

    const auto it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        qCWarning(KPACKAGE_LOG) << "couldn't find" << key << "when trying to list entries";
        return QStringList();
    }

    QStringList list;
    for (const QString &prefix : std::as_const(d->contentsPrefixPaths)) {
        // qCDebug(KPACKAGE_LOG) << "     looking in" << prefix;
        const QStringList paths = it.value().paths;
        for (const QString &path : paths) {
            // qCDebug(KPACKAGE_LOG) << "         looking in" << path;
            if (it.value().directory) {
                // qCDebug(KPACKAGE_LOG) << "it's a directory, so trying out" << d->path + prefix + path;
                QDir dir(d->path + prefix + path);
                if (d->externalPaths) {
                    list += dir.entryList(QDir::Files | QDir::Readable);
                } else {
                    // ensure that we don't return files outside of our base path
                    // due to symlink or ../ games
                    QString canonicalized = dir.canonicalPath();
                    if (canonicalized.startsWith(d->path)) {
                        list += dir.entryList(QDir::Files | QDir::Readable);
                    }
                }
            } else {
                const QString fullPath = d->path + prefix + path;
                // qCDebug(KPACKAGE_LOG) << "it's a file at" << fullPath << QFile::exists(fullPath);
                if (!QFile::exists(fullPath)) {
                    continue;
                }

                if (d->externalPaths) {
                    list += fullPath;
                } else {
                    QDir dir(fullPath);
                    QString canonicalized = dir.canonicalPath() + QDir::separator();

                    // qCDebug(KPACKAGE_LOG) << "testing that" << canonicalized << "is in" << d->path;
                    if (canonicalized.startsWith(d->path)) {
                        list += fullPath;
                    }
                }
            }
        }
    }

    return list;
}

void Package::setPath(const QString &path)
{
    // if the path is already what we have, don't bother
    if (path == d->path) {
        return;
    }

    // our dptr is shared, and it is almost certainly going to change.
    // hold onto the old pointer just in case it does not, however!
    QExplicitlySharedDataPointer<PackagePrivate> oldD(d);
    d.detach();
    d->metadata = std::nullopt;

    // without structure we're doomed
    if (!d->structure) {
        d->path.clear();
        d->discoveries.clear();
        d->valid = false;
        d->checkedValid = true;
        qCWarning(KPACKAGE_LOG) << "Cannot set a path in a package without structure" << path;
        return;
    }

    // empty path => nothing to do
    if (path.isEmpty()) {
        d->path.clear();
        d->discoveries.clear();
        d->valid = false;
        d->structure.data()->pathChanged(this);
        return;
    }

    // now we look for all possible paths, including resolving
    // relative paths against the system search paths
    QStringList paths;
    if (QDir::isRelativePath(path)) {
        QString p;

        if (d->defaultPackageRoot.isEmpty()) {
            p = path % QLatin1Char('/');
        } else {
            p = d->defaultPackageRoot % path % QLatin1Char('/');
        }

        if (QDir::isRelativePath(p)) {
            // FIXME: can searching of the qrc be better?
            paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, p, QStandardPaths::LocateDirectory);
        } else {
            const QDir dir(p);
            if (QFile::exists(dir.canonicalPath())) {
                paths << p;
            }
        }

        // qCDebug(KPACKAGE_LOG) << "paths:" << p << paths << d->defaultPackageRoot;
    } else {
        const QDir dir(path);
        if (QFile::exists(dir.canonicalPath())) {
            paths << path;
        }
    }

    QFileInfo fileInfo(path);
    if (fileInfo.isFile() && d->tempRoot.isEmpty()) {
        d->path = fileInfo.canonicalFilePath();
        d->tempRoot = d->unpack(path);
    }

    // now we search each path found, caching our previous path to know if
    // anything actually really changed
    const QString previousPath = d->path;
    for (const QString &p : std::as_const(paths)) {
        d->checkedValid = false;
        QDir dir(p);

        Q_ASSERT(QFile::exists(dir.canonicalPath()));

        d->path = dir.canonicalPath();
        // canonicalPath() does not include a trailing slash (unless it is the root dir)
        if (!d->path.endsWith(QLatin1Char('/'))) {
            d->path.append(QLatin1Char('/'));
        }

        const QString fallbackPath = metadata().value(QStringLiteral("X-Plasma-RootPath"));
        if (!fallbackPath.isEmpty()) {
            const KPackage::Package fp = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), fallbackPath);
            setFallbackPackage(fp);
        }

        // we need to tell the structure we're changing paths ...
        d->structure.data()->pathChanged(this);
        // ... and then testing the results for validity
        if (isValid()) {
            break;
        }
    }

    // if nothing did change, then we go back to the old dptr
    if (d->path == previousPath) {
        d = oldD;
        return;
    }

    // .. but something did change, so we get rid of our discovery cache
    d->discoveries.clear();

    // Do NOT override the metadata when the PackageStructure has set it
    if (!previousPath.isEmpty()) {
        d->metadata = std::nullopt;
    }

    // uh-oh, but we didn't end up with anything valid, so we sadly reset ourselves
    // to futility.
    if (!d->valid) {
        d->path.clear();
        d->structure.data()->pathChanged(this);
    }
}

const QString Package::path() const
{
    return d->path;
}

QStringList Package::contentsPrefixPaths() const
{
    return d->contentsPrefixPaths;
}

void Package::setContentsPrefixPaths(const QStringList &prefixPaths)
{
    d.detach();
    d->contentsPrefixPaths = prefixPaths;
    if (d->contentsPrefixPaths.isEmpty()) {
        d->contentsPrefixPaths << QString();
    } else {
        // the code assumes that the prefixes have a trailing slash
        // so let's make that true here
        QMutableStringListIterator it(d->contentsPrefixPaths);
        while (it.hasNext()) {
            it.next();

            if (!it.value().endsWith(QLatin1Char('/'))) {
                it.setValue(it.value() % QLatin1Char('/'));
            }
        }
    }
}

QByteArray Package::cryptographicHash(QCryptographicHash::Algorithm algorithm) const
{
    if (!d->valid) {
        qCWarning(KPACKAGE_LOG) << "can not create hash due to Package being invalid";
        return QByteArray();
    }

    QCryptographicHash hash(algorithm);
    const QString guessedMetaDataJson = d->path + QLatin1String("metadata.json");
    const QString metadataPath = QFile::exists(guessedMetaDataJson) ? guessedMetaDataJson : QString();
    if (!metadataPath.isEmpty()) {
        QFile f(metadataPath);
        if (f.open(QIODevice::ReadOnly)) {
            while (!f.atEnd()) {
                hash.addData(f.read(1024));
            }
        } else {
            qCWarning(KPACKAGE_LOG) << "could not add" << f.fileName() << "to the hash; file could not be opened for reading.";
        }
    } else {
        qCWarning(KPACKAGE_LOG) << "no metadata at" << metadataPath;
    }

    for (const QString &prefix : std::as_const(d->contentsPrefixPaths)) {
        const QString basePath = d->path + prefix;
        QDir dir(basePath);

        if (!dir.exists()) {
            return QByteArray();
        }

        d->updateHash(basePath, QString(), dir, hash);
    }

    return hash.result().toHex();
}

void Package::addDirectoryDefinition(const QByteArray &key, const QString &path)
{
    const auto contentsIt = d->contents.constFind(key);
    ContentStructure s;

    if (contentsIt != d->contents.constEnd()) {
        if (contentsIt->paths.contains(path) && contentsIt->directory == true) {
            return;
        }
        s = *contentsIt;
    }

    d.detach();

    s.paths.append(path);
    s.directory = true;

    d->contents[key] = s;
}

void Package::addFileDefinition(const QByteArray &key, const QString &path)
{
    const auto contentsIt = d->contents.constFind(key);
    ContentStructure s;

    if (contentsIt != d->contents.constEnd()) {
        if (contentsIt->paths.contains(path) && contentsIt->directory == true) {
            return;
        }
        s = *contentsIt;
    }

    d.detach();

    s.paths.append(path);
    s.directory = false;

    d->contents[key] = s;
}

void Package::removeDefinition(const QByteArray &key)
{
    if (d->contents.contains(key)) {
        d.detach();
        d->contents.remove(key);
    }

    if (d->discoveries.contains(QString::fromLatin1(key))) {
        d.detach();
        d->discoveries.remove(QString::fromLatin1(key));
    }
}

void Package::setRequired(const QByteArray &key, bool required)
{
    QHash<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        qCWarning(KPACKAGE_LOG) << key << "is now a known key for the package. File is thus not set to being required";
        return;
    }

    d.detach();
    // have to find the item again after detaching: d->contents is a different object now
    it = d->contents.find(key);
    it.value().required = required;
}

void Package::setDefaultMimeTypes(const QStringList &mimeTypes)
{
    d.detach();
    d->mimeTypes = mimeTypes;
}

void Package::setMimeTypes(const QByteArray &key, const QStringList &mimeTypes)
{
    if (!d->contents.contains(key)) {
        return;
    }

    d.detach();
    d->contents[key].mimeTypes = mimeTypes;
}

QList<QByteArray> Package::directories() const
{
    QList<QByteArray> dirs;
    for (auto it = d->contents.cbegin(); it != d->contents.cend(); ++it) {
        if (it.value().directory) {
            dirs << it.key();
        }
    }
    return dirs;
}

QList<QByteArray> Package::requiredDirectories() const
{
    QList<QByteArray> dirs;
    for (auto it = d->contents.cbegin(); it != d->contents.cend(); ++it) {
        if (it.value().directory && it.value().required) {
            dirs << it.key();
        }
    }
    return dirs;
}

QList<QByteArray> Package::files() const
{
    QList<QByteArray> files;
    for (auto it = d->contents.cbegin(); it != d->contents.cend(); ++it) {
        if (!it.value().directory) {
            files << it.key();
        }
    }
    return files;
}

QList<QByteArray> Package::requiredFiles() const
{
    QList<QByteArray> files;
    for (auto it = d->contents.cbegin(); it != d->contents.cend(); ++it) {
        if (!it.value().directory && it.value().required) {
            files << it.key();
        }
    }

    return files;
}

PackagePrivate::PackagePrivate()
    : QSharedData()
{
    contentsPrefixPaths << QStringLiteral("contents/");
}

PackagePrivate::PackagePrivate(const PackagePrivate &other)
    : QSharedData()
{
    *this = other;
    if (other.metadata && other.metadata.value().isValid()) {
        metadata = other.metadata;
    }
}

PackagePrivate::~PackagePrivate()
{
    if (!tempRoot.isEmpty()) {
        QDir dir(tempRoot);
        dir.removeRecursively();
    }
}

PackagePrivate &PackagePrivate::operator=(const PackagePrivate &rhs)
{
    if (&rhs == this) {
        return *this;
    }

    structure = rhs.structure;
    if (rhs.fallbackPackage) {
        fallbackPackage = std::make_unique<Package>(*rhs.fallbackPackage);
    } else {
        fallbackPackage = nullptr;
    }
    if (rhs.metadata && rhs.metadata.value().isValid()) {
        metadata = rhs.metadata;
    }
    path = rhs.path;
    contentsPrefixPaths = rhs.contentsPrefixPaths;
    contents = rhs.contents;
    mimeTypes = rhs.mimeTypes;
    defaultPackageRoot = rhs.defaultPackageRoot;
    externalPaths = rhs.externalPaths;
    valid = rhs.valid;
    return *this;
}

void PackagePrivate::updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCryptographicHash &hash)
{
    // hash is calculated as a function of:
    // * files ordered alphabetically by name, with each file's:
    //      * path relative to the content root
    //      * file data
    // * directories ordered alphabetically by name, with each dir's:
    //      * path relative to the content root
    //      * file listing (recursing)
    // symlinks (in both the file and dir case) are handled by adding
    // the name of the symlink itself and the abs path of what it points to

    const QDir::SortFlags sorting = QDir::Name | QDir::IgnoreCase;
    const QDir::Filters filters = QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
    const auto lstEntries = dir.entryList(QDir::Files | filters, sorting);
    for (const QString &file : lstEntries) {
        if (!subPath.isEmpty()) {
            hash.addData(subPath.toUtf8());
        }

        hash.addData(file.toUtf8());

        QFileInfo info(dir.path() + QLatin1Char('/') + file);
        if (info.isSymLink()) {
            hash.addData(info.symLinkTarget().toUtf8());
        } else {
            QFile f(info.filePath());
            if (f.open(QIODevice::ReadOnly)) {
                while (!f.atEnd()) {
                    hash.addData(f.read(1024));
                }
            } else {
                qCWarning(KPACKAGE_LOG) << "could not add" << f.fileName() << "to the hash; file could not be opened for reading. "
                                        << "permissions fail?" << info.permissions() << info.isFile();
            }
        }
    }

    const auto lstEntries2 = dir.entryList(QDir::Dirs | filters, sorting);
    for (const QString &subDirPath : lstEntries2) {
        const QString relativePath = subPath + subDirPath + QLatin1Char('/');
        hash.addData(relativePath.toUtf8());

        QDir subDir(dir.path());
        subDir.cd(subDirPath);

        if (subDir.path() != subDir.canonicalPath()) {
            hash.addData(subDir.canonicalPath().toUtf8());
        } else {
            updateHash(basePath, relativePath, subDir, hash);
        }
    }
}

void PackagePrivate::createPackageMetadata(const QString &path)
{
    if (QFileInfo(path).isDir()) {
        if (const QString jsonPath = path + QLatin1String("/metadata.json"); QFileInfo::exists(jsonPath)) {
            metadata = KPluginMetaData::fromJsonFile(jsonPath);
        } else {
            qCDebug(KPACKAGE_LOG) << "No metadata file in the package, expected it at:" << jsonPath;
        }
    } else {
        metadata = KPluginMetaData::fromJsonFile(path);
    }
}

QString PackagePrivate::fallbackFilePath(const QByteArray &key, const QString &filename) const
{
    // don't fallback if the package isn't valid and never fallback the metadata file
    if (key != "metadata" && fallbackPackage && fallbackPackage->isValid()) {
        return fallbackPackage->filePath(key, filename);
    } else {
        return QString();
    }
}

bool PackagePrivate::hasCycle(const KPackage::Package &package)
{
    if (!package.d->fallbackPackage) {
        return false;
    }

    // This is the Floyd cycle detection algorithm
    // http://en.wikipedia.org/wiki/Cycle_detection#Tortoise_and_hare
    const KPackage::Package *slowPackage = &package;
    const KPackage::Package *fastPackage = &package;

    while (fastPackage && fastPackage->d->fallbackPackage) {
        // consider two packages the same if they have the same metadata
        if ((fastPackage->d->fallbackPackage->metadata().isValid() && fastPackage->d->fallbackPackage->metadata() == slowPackage->metadata())
            || (fastPackage->d->fallbackPackage->d->fallbackPackage && fastPackage->d->fallbackPackage->d->fallbackPackage->metadata().isValid()
                && fastPackage->d->fallbackPackage->d->fallbackPackage->metadata() == slowPackage->metadata())) {
            qCWarning(KPACKAGE_LOG) << "Warning: the fallback chain of " << package.metadata().pluginId() << "contains a cyclical dependency.";
            return true;
        }
        fastPackage = fastPackage->d->fallbackPackage->d->fallbackPackage.get();
        slowPackage = slowPackage->d->fallbackPackage.get();
    }
    return false;
}

} // Namespace
