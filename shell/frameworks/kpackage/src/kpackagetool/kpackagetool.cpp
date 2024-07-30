/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012-2017 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpackagetool.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KShell>
#include <QDebug>

#include <KJob>
#include <kpackage/package.h>
#include <kpackage/packageloader.h>
#include <kpackage/packagestructure.h>
#include <kpackage/private/utils.h>

#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QXmlStreamWriter>

#include <iomanip>
#include <iostream>

#include "options.h"

#include "../kpackage/config-package.h"

#include "kpackage_debug.h"

Q_GLOBAL_STATIC_WITH_ARGS(QTextStream, cout, (stdout))
Q_GLOBAL_STATIC_WITH_ARGS(QTextStream, cerr, (stderr))

namespace KPackage
{
class PackageToolPrivate
{
public:
    QString packageRoot;
    QString packageFile;
    QString package;
    QString kpackageType = QStringLiteral("KPackage/Generic");
    KPluginMetaData metadata;
    QString installPath;
    void output(const QString &msg);
    QStringList packages(const QString &type, const QString &path = QString());
    void renderTypeTable(const QMap<QString, QString> &plugins);
    void listTypes();
    void coutput(const QString &msg);
    void cerror(const QString &msg);
    QCommandLineParser *parser = nullptr;
};

PackageTool::PackageTool(int &argc, char **argv, QCommandLineParser *parser)
    : QCoreApplication(argc, argv)
{
    d = new PackageToolPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, &PackageTool::runMain);
}

PackageTool::~PackageTool()
{
    delete d;
}

void PackageTool::runMain()
{
    if (d->parser->isSet(Options::hash())) {
        const QString path = d->parser->value(Options::hash());
        KPackage::PackageStructure structure;
        KPackage::Package package(&structure);
        package.setPath(path);
        const QString hash = QString::fromLocal8Bit(package.cryptographicHash(QCryptographicHash::Sha1));
        if (hash.isEmpty()) {
            d->coutput(i18n("Failed to generate a Package hash for %1", path));
            exit(9);
        } else {
            d->coutput(i18n("SHA1 hash for Package at %1: '%2'", package.path(), hash));
            exit(0);
        }
        return;
    }

    if (d->parser->isSet(Options::listTypes())) {
        d->listTypes();
        exit(0);
        return;
    }

    if (d->parser->isSet(Options::type())) {
        d->kpackageType = d->parser->value(Options::type());
    }
    d->packageRoot = KPackage::PackageLoader::self()->loadPackage(d->kpackageType).defaultPackageRoot();

    if (d->parser->isSet(Options::remove())) {
        d->package = d->parser->value(Options::remove());
    } else if (d->parser->isSet(Options::upgrade())) {
        d->package = d->parser->value(Options::upgrade());
    } else if (d->parser->isSet(Options::install())) {
        d->package = d->parser->value(Options::install());
    } else if (d->parser->isSet(Options::show())) {
        d->package = d->parser->value(Options::show());
    } else if (d->parser->isSet(Options::appstream())) {
        d->package = d->parser->value(Options::appstream());
    }

    if (!QDir::isAbsolutePath(d->package)) {
        d->packageFile = QDir(QDir::currentPath() + QLatin1Char('/') + d->package).absolutePath();
        d->packageFile = QFileInfo(d->packageFile).canonicalFilePath();
        if (d->parser->isSet(Options::upgrade())) {
            d->package = d->packageFile;
        }
    } else {
        d->packageFile = d->package;
    }

    if (!PackageLoader::self()->loadPackageStructure(d->kpackageType)) {
        qWarning() << "Package type" << d->kpackageType << "not found";
    }

    if (d->parser->isSet(Options::show())) {
        const QString pluginName = d->package;
        showPackageInfo(pluginName);
        return;
    } else if (d->parser->isSet(Options::appstream())) {
        const QString pluginName = d->package;
        showAppstreamInfo(pluginName);
        return;
    }

    if (d->parser->isSet(Options::list())) {
        QString packageRoot = resolvePackageRootWithOptions();
        d->coutput(i18n("Listing KPackageType: %1 in %2", d->kpackageType, packageRoot));
        listPackages(d->kpackageType, packageRoot);
        exit(0);
    } else {
        // install, remove or upgrade
        d->packageRoot = resolvePackageRootWithOptions();

        if (d->parser->isSet(Options::remove()) || d->parser->isSet(Options::upgrade())) {
            QString pkgPath;
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(d->kpackageType);
            pkg.setPath(d->package);
            if (pkg.isValid()) {
                pkgPath = pkg.path();
                if (pkgPath.isEmpty() && !d->packageFile.isEmpty()) {
                    pkgPath = d->packageFile;
                }
            }
            if (pkgPath.isEmpty()) {
                pkgPath = d->package;
            }
            QString _p = d->packageRoot;
            if (!_p.endsWith(QLatin1Char('/'))) {
                _p.append(QLatin1Char('/'));
            }
            _p.append(d->package);

            if (!d->parser->isSet(Options::type())) {
                d->kpackageType = readKPackageType(pkg.metadata());
            }

            QString pluginName;
            if (pkg.metadata().isValid()) {
                d->metadata = pkg.metadata();
                if (!d->metadata.isValid()) {
                    pluginName = d->package;
                } else if (!d->metadata.isValid() && d->metadata.pluginId().isEmpty()) {
                    // plugin id given in command line
                    pluginName = d->package;
                } else {
                    // Parameter was a plasma package, get plugin id from the package
                    pluginName = d->metadata.pluginId();
                }
            }
            QStringList installed = d->packages(d->kpackageType);

            // Uninstalling ...
            if (installed.contains(pluginName)) { // Assume it's a plugin id
                KPackage::PackageJob *uninstallJob = KPackage::PackageJob::uninstall(d->kpackageType, pluginName, d->packageRoot);
                connect(uninstallJob, &KPackage::PackageJob::finished, this, [uninstallJob, this]() {
                    packageUninstalled(uninstallJob);
                });
                return;
            } else {
                d->coutput(i18n("Error: Plugin %1 is not installed.", pluginName));
                exit(2);
            }
        }
        if (d->parser->isSet(Options::install())) {
            auto installJob = KPackage::PackageJob::install(d->kpackageType, d->packageFile, d->packageRoot);
            connect(installJob, &KPackage::PackageJob::finished, this, [installJob, this]() {
                packageInstalled(installJob);
            });
            return;
        }
        if (d->package.isEmpty()) {
            qWarning() << i18nc(
                "No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list",
                "One of install, remove, upgrade or list is required.");
            exit(6);
        }
    }
}

void PackageToolPrivate::coutput(const QString &msg)
{
    *cout << msg << '\n';
    (*cout).flush();
}

void PackageToolPrivate::cerror(const QString &msg)
{
    *cerr << msg << '\n';
    (*cerr).flush();
}

QStringList PackageToolPrivate::packages(const QString &type, const QString &path)
{
    QStringList result;
    const QList<KPluginMetaData> dataList = KPackage::PackageLoader::self()->listPackages(type, path);
    for (const KPluginMetaData &data : dataList) {
        if (!result.contains(data.pluginId())) {
            result << data.pluginId();
        }
    }
    return result;
}

void PackageTool::showPackageInfo(const QString &pluginName)
{
    KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(d->kpackageType);
    pkg.setDefaultPackageRoot(d->packageRoot);

    if (QFile::exists(d->packageFile)) {
        pkg.setPath(d->packageFile);
    } else {
        pkg.setPath(pluginName);
    }

    KPluginMetaData i = pkg.metadata();
    if (!i.isValid()) {
        *cerr << i18n("Error: Can't find plugin metadata: %1\n", pluginName);
        exit(3);
        return;
    }
    d->coutput(i18n("Showing info for package: %1", pluginName));
    d->coutput(i18n("  Name       : %1", i.name()));
    d->coutput(i18n("  Description: %1", i.description()));
    d->coutput(i18n("  Plugin     : %1", i.pluginId()));
    auto const authors = i.authors();
    QStringList authorNames;
    for (const KAboutPerson &author : authors) {
        authorNames << author.name();
    }
    d->coutput(i18n("  Author     : %1", authorNames.join(QLatin1String(", "))));
    d->coutput(i18n("  Path       : %1", pkg.path()));

    exit(0);
}

bool translateKPluginToAppstream(const QString &tagName,
                                 const QString &configField,
                                 const QJsonObject &configObject,
                                 QXmlStreamWriter &writer,
                                 bool canEndWithDot)
{
    const QRegularExpression rx(QStringLiteral("%1\\[(.*)\\]").arg(configField));
    const QJsonValue native = configObject.value(configField);
    if (native.isUndefined()) {
        return false;
    }

    QString content = native.toString();
    if (!canEndWithDot && content.endsWith(QLatin1Char('.'))) {
        content.chop(1);
    }
    writer.writeTextElement(tagName, content);
    for (auto it = configObject.begin(), itEnd = configObject.end(); it != itEnd; ++it) {
        const auto match = rx.match(it.key());

        if (match.hasMatch()) {
            QString content = it->toString();
            if (!canEndWithDot && content.endsWith(QLatin1Char('.'))) {
                content.chop(1);
            }

            writer.writeStartElement(tagName);
            writer.writeAttribute(QStringLiteral("xml:lang"), match.captured(1));
            writer.writeCharacters(content);
            writer.writeEndElement();
        }
    }
    return true;
}

void PackageTool::showAppstreamInfo(const QString &pluginName)
{
    KPluginMetaData i;
    // if the path passed is an absolute path, and a metadata file is found under it, use that metadata file to generate the appstream info.
    // This can happen in the case an application wanting to support kpackage based extensions includes in the same project both the packagestructure plugin and
    // the packages themselves. In that case at build time the packagestructure plugin wouldn't be installed yet

    if (QFile::exists(pluginName + QStringLiteral("/metadata.json"))) {
        i = KPluginMetaData::fromJsonFile(pluginName + QStringLiteral("/metadata.json"));
    } else {
        KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(d->kpackageType);

        pkg.setDefaultPackageRoot(d->packageRoot);

        if (QFile::exists(d->packageFile)) {
            pkg.setPath(d->packageFile);
        } else {
            pkg.setPath(pluginName);
        }

        i = pkg.metadata();
    }

    if (!i.isValid()) {
        *cerr << i18n("Error: Can't find plugin metadata: %1\n", pluginName);
        std::exit(3);
        return;
    }
    QString parentApp = i.value(QLatin1String("X-KDE-ParentApp"));

    if (i.value(QStringLiteral("NoDisplay"), false)) {
        std::exit(0);
    }

    QXmlStreamAttributes componentAttributes;
    if (!parentApp.isEmpty()) {
        componentAttributes << QXmlStreamAttribute(QLatin1String("type"), QLatin1String("addon"));
    }

    // Compatibility: without appstream-metainfo-output argument we print the XML output to STDOUT
    // with the argument we'll print to the defined path.
    // TODO: in KF6 we should switch to argument-only.
    QIODevice *outputDevice = cout->device();
    std::unique_ptr<QFile> outputFile;
    const auto outputPath = d->parser->value(Options::appstreamOutput());
    if (!outputPath.isEmpty()) {
        auto outputUrl = QUrl::fromUserInput(outputPath);
        outputFile.reset(new QFile(outputUrl.toLocalFile()));
        if (!outputFile->open(QFile::WriteOnly | QFile::Text)) {
            *cerr << "Failed to open output file for writing.";
            exit(1);
        }
        outputDevice = outputFile.get();
    }

    if (i.description().isEmpty()) {
        *cerr << "Error: description missing, will result in broken appdata field as <summary/> is mandatory at " << QFileInfo(i.fileName()).absoluteFilePath();
        std::exit(10);
    }

    QXmlStreamWriter writer(outputDevice);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("component"));
    writer.writeAttributes(componentAttributes);

    writer.writeTextElement(QStringLiteral("id"), i.pluginId());
    if (!parentApp.isEmpty()) {
        writer.writeTextElement(QStringLiteral("extends"), parentApp);
    }

    const QJsonObject rootObject = i.rawData()[QStringLiteral("KPlugin")].toObject();
    translateKPluginToAppstream(QStringLiteral("name"), QStringLiteral("Name"), rootObject, writer, false);
    translateKPluginToAppstream(QStringLiteral("summary"), QStringLiteral("Description"), rootObject, writer, false);
    if (!i.website().isEmpty()) {
        writer.writeStartElement(QStringLiteral("url"));
        writer.writeAttribute(QStringLiteral("type"), QStringLiteral("homepage"));
        writer.writeCharacters(i.website());
        writer.writeEndElement();
    }

    if (i.pluginId().startsWith(QLatin1String("org.kde."))) {
        writer.writeStartElement(QStringLiteral("url"));
        writer.writeAttribute(QStringLiteral("type"), QStringLiteral("donation"));
        writer.writeCharacters(QStringLiteral("https://www.kde.org/donate.php?app=%1").arg(i.pluginId()));
        writer.writeEndElement();
    }

    const auto authors = i.authors();
    if (!authors.isEmpty()) {
        QStringList authorsText;
        authorsText.reserve(authors.size());
        for (const auto &author : authors) {
            authorsText += QStringLiteral("%1").arg(author.name());
        }
        writer.writeStartElement(QStringLiteral("developer"));
        writer.writeAttribute(QStringLiteral("id"), QStringLiteral("kde.org"));
        writer.writeTextElement(QStringLiteral("name"), authorsText.join(QStringLiteral(", ")));
        writer.writeEndElement();
    }

    if (!i.iconName().isEmpty()) {
        writer.writeStartElement(QStringLiteral("icon"));
        writer.writeAttribute(QStringLiteral("type"), QStringLiteral("stock"));
        writer.writeCharacters(i.iconName());
        writer.writeEndElement();
    }
    writer.writeTextElement(QStringLiteral("project_license"), KAboutLicense::byKeyword(i.license()).spdx());
    writer.writeTextElement(QStringLiteral("metadata_license"), QStringLiteral("CC0-1.0"));
    writer.writeEndElement();
    writer.writeEndDocument();

    exit(0);
}

QString PackageTool::resolvePackageRootWithOptions()
{
    QString packageRoot;
    if (d->parser->isSet(Options::packageRoot()) && d->parser->isSet(Options::global())) {
        qWarning() << i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one",
                            "The packageroot and global options conflict with each other, please select only one.");
        ::exit(7);
    } else if (d->parser->isSet(Options::packageRoot())) {
        packageRoot = d->parser->value(Options::packageRoot());
        // qDebug() << "(set via arg) d->packageRoot is: " << d->packageRoot;
    } else if (d->parser->isSet(Options::global())) {
        auto const paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, d->packageRoot, QStandardPaths::LocateDirectory);
        if (!paths.isEmpty()) {
            packageRoot = paths.last();
        }
    } else {
        packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + d->packageRoot;
    }
    return packageRoot;
}

void PackageTool::listPackages(const QString &kpackageType, const QString &path)
{
    QStringList list = d->packages(kpackageType, path);
    list.sort();
    for (const QString &package : std::as_const(list)) {
        d->coutput(package);
    }
    exit(0);
}

void PackageToolPrivate::renderTypeTable(const QMap<QString, QString> &plugins)
{
    const QString nameHeader = i18n("KPackage Structure Name");
    const QString pathHeader = i18n("Path");
    int nameWidth = nameHeader.length();
    int pathWidth = pathHeader.length();

    QMapIterator<QString, QString> pluginIt(plugins);
    while (pluginIt.hasNext()) {
        pluginIt.next();
        if (pluginIt.key().length() > nameWidth) {
            nameWidth = pluginIt.key().length();
        }

        if (pluginIt.value().length() > pathWidth) {
            pathWidth = pluginIt.value().length();
        }
    }

    std::cout << nameHeader.toLocal8Bit().constData() << std::setw(nameWidth - nameHeader.length() + 2) << ' ' << pathHeader.toLocal8Bit().constData()
              << std::setw(pathWidth - pathHeader.length() + 2) << ' ' << std::endl;
    std::cout << std::setfill('-') << std::setw(nameWidth) << '-' << "  " << std::setw(pathWidth) << '-' << "  " << std::endl;
    std::cout << std::setfill(' ');

    pluginIt.toFront();
    while (pluginIt.hasNext()) {
        pluginIt.next();
        std::cout << pluginIt.key().toLocal8Bit().constData() << std::setw(nameWidth - pluginIt.key().length() + 2) << ' '
                  << pluginIt.value().toLocal8Bit().constData() << std::setw(pathWidth - pluginIt.value().length() + 2) << std::endl;
    }
}

void PackageToolPrivate::listTypes()
{
    coutput(i18n("Package types that are installable with this tool:"));
    coutput(i18n("Built in:"));

    QMap<QString, QString> builtIns;
    builtIns.insert(i18n("KPackage/Generic"), QStringLiteral(KPACKAGE_RELATIVE_DATA_INSTALL_DIR "/packages/"));
    builtIns.insert(i18n("KPackage/GenericQML"), QStringLiteral(KPACKAGE_RELATIVE_DATA_INSTALL_DIR "/genericqml/"));

    renderTypeTable(builtIns);

    const QList<KPluginMetaData> offers = KPluginMetaData::findPlugins(QStringLiteral("kf6/packagestructure"));

    if (!offers.isEmpty()) {
        std::cout << std::endl;
        coutput(i18n("Provided by plugins:"));

        QMap<QString, QString> plugins;
        for (const KPluginMetaData &info : offers) {
            const QString type = readKPackageType(info);
            if (type.isEmpty()) {
                continue;
            }
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(type);
            plugins.insert(type, pkg.defaultPackageRoot());
        }

        renderTypeTable(plugins);
    }
}

void PackageTool::packageInstalled(KPackage::PackageJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->parser->isSet(Options::upgrade())) {
            d->coutput(i18n("Successfully upgraded %1", job->package().path()));
        } else {
            d->coutput(i18n("Successfully installed %1", job->package().path()));
        }
    } else {
        d->cerror(i18n("Error: Installation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 4;
    }
    exit(exitcode);
}

void PackageTool::packageUninstalled(KPackage::PackageJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->parser->isSet(Options::upgrade())) {
            d->coutput(i18n("Upgrading package from file: %1", d->packageFile));
            auto installJob = KPackage::PackageJob::install(d->kpackageType, d->packageFile, d->packageRoot);
            connect(installJob, &KPackage::PackageJob::finished, this, [installJob, this]() {
                packageInstalled(installJob);
            });
            return;
        }
        d->coutput(i18n("Successfully uninstalled %1", job->package().path()));
    } else {
        d->cerror(i18n("Error: Uninstallation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 7;
    }
    exit(exitcode);
}

} // namespace KPackage

#include "moc_kpackagetool.cpp"
