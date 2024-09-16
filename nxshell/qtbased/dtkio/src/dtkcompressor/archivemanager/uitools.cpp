// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "uitools.h"
#include "mimetypes.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "pluginmanager.h"

#include <DStandardPaths>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QTextCodec>
#include <QRegularExpression>
#include <QUuid>
#include <QStorageInfo>
#include <QProcessEnvironment>

#include <KEncodingProber>

DCORE_USE_NAMESPACE
Q_DECLARE_METATYPE(KPluginMetaData)
DCOMPRESSOR_USE_NAMESPACE
QStringList UiTools::m_associtionlist = QStringList() << "file_association.file_association_type.x-7z-compressed"
                                                      << "file_association.file_association_type.x-archive"
                                                      << "file_association.file_association_type.x-bcpio"
                                                      << "file_association.file_association_type.x-bzip"
                                                      << "file_association.file_association_type.x-cpio"
                                                      << "file_association.file_association_type.x-cpio-compressed"
                                                      << "file_association.file_association_type.vnd.debian.binary-package"
                                                      << "file_association.file_association_type.gzip"
                                                      << "file_association.file_association_type.x-java-archive"
                                                      << "file_association.file_association_type.x-lzma"
                                                      << "file_association.file_association_type.vnd.ms-cab-compressed"
                                                      << "file_association.file_association_type.vnd.rar"
                                                      << "file_association.file_association_type.x-rpm"
                                                      << "file_association.file_association_type.x-sv4cpio"
                                                      << "file_association.file_association_type.x-sv4crc"
                                                      << "file_association.file_association_type.x-tar"
                                                      << "file_association.file_association_type.x-bzip-compressed-tar"
                                                      << "file_association.file_association_type.x-compressed-tar"
                                                      << "file_association.file_association_type.x-lzip-compressed-tar"
                                                      << "file_association.file_association_type.x-lzma-compressed-tar"
                                                      << "file_association.file_association_type.x-tzo"
                                                      << "file_association.file_association_type.x-xz-compressed-tar"
                                                      << "file_association.file_association_type.x-tarz"
                                                      << "file_association.file_association_type.x-xar"
                                                      << "file_association.file_association_type.x-xz"
                                                      << "file_association.file_association_type.zip"
                                                      << "file_association.file_association_type.x-iso9660-image"
                                                      << "file_association.file_association_type.x-iso9660-appimage"
                                                      << "file_association.file_association_type.x-source-rpm"
                                                      << "file_association.file_association_type.x-chrome-extension";

UiTools::UiTools(QObject *parent)
    : QObject(parent)
{
}

UiTools::~UiTools()
{
}

QString UiTools::getConfigPath()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
                     .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

QString UiTools::humanReadableSize(const qint64 &size, int precision)
{
    if (0 == size) {
        return "-";
    }

    double sizeAsDouble = size;
    static QStringList measures;
    if (measures.isEmpty()) {
        measures << "B"
                 << "KB"
                 << "MB"
                 << "GB"
                 << "TB"
                 << "PB"
                 << "EB"
                 << "ZB"
                 << "YB";
    }
    QStringListIterator it(measures);
    QString measure(it.next());
    while (sizeAsDouble >= 1024.0 && it.hasNext()) {
        measure = it.next();
        sizeAsDouble /= 1024.0;
    }

    return QString::fromLatin1("%1 %2").arg(sizeAsDouble, 0, 'f', precision).arg(measure);
}

bool UiTools::isArchiveFile(const QString &strFileName)
{
    QString strTransFileName = strFileName;
    UnCompressParameter::SplitType type;
    UiTools::transSplitFileName(strTransFileName, type);

    CustomMimeType mimeType = determineMimeType(strTransFileName);
    QString mime;
    if (mimeType.name().contains("application/"))
        mime = mimeType.name().remove("application/");

    bool ret = false;

    if (mime.size() > 0) {
        bool bArchive = false;
        ret = isExistMimeType(mime, bArchive);
    } else {
        ret = false;
    }

    if (strTransFileName.endsWith(".deb")) {
        ret = false;
    }

    if (strTransFileName.endsWith(".crx") || strTransFileName.endsWith(".apk")) {
        ret = true;
    }

    return ret;
}

bool UiTools::isExistMimeType(const QString &strMimeType, bool &bArchive)
{
    QString conf = readConf();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringList confList = conf.split("\n", Qt::SkipEmptyParts);
#else
    QStringList confList = conf.split("\n", QString::SkipEmptyParts);
#endif

    bool exist = false;
    for (int i = 0; i < confList.count(); i++) {
        if (confList.at(i).contains("." + strMimeType + ":")) {
            bArchive = true;
            if (confList.at(i).contains("true")) {
                exist = true;
                break;
            } else {
                exist = false;
                continue;
            }
        }
    }

    return exist;
}

QString UiTools::readConf()
{
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir;
    if (!dir.exists(confDir + QDir::separator())) {
        dir.mkpath(confDir + QDir::separator());
    }

    const QString confPath = confDir + QDir::separator() + "deepin-compressor.confbf";
    QFile confFile(confPath);

    // default settings
    if (!confFile.exists()) {
        confFile.open(QIODevice::WriteOnly | QIODevice::Text);

        foreach (QString key, m_associtionlist) {
            QString content = key + ":" + "true" + "\n";
            confFile.write(content.toUtf8());
        }

        confFile.close();
    }

    QString confValue;
    bool readStatus = confFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (readStatus) {
        confValue = confFile.readAll();
    }

    confFile.close();
    return confValue;
}

QString UiTools::toShortString(QString strSrc, int limitCounts, int left)
{
    left = (left >= limitCounts || left <= 0) ? limitCounts / 2 : left;
    int right = limitCounts - left;
    QString displayName = "";
    displayName = strSrc.length() > limitCounts ? strSrc.left(left) + "..." + strSrc.right(right) : strSrc;
    return displayName;
}

ReadOnlyArchiveInterface *UiTools::createInterface(const QString &fileName, bool bWrite, AssignPluginType eType /*, bool bUseLibArchive*/)
{

    const CustomMimeType mimeType = determineMimeType(fileName);

    QVector<Plugin *> offers;
    if (bWrite) {
        offers = PluginManager::get_instance().preferredWritePluginsFor(mimeType);
    } else {
        offers = PluginManager::get_instance().preferredPluginsFor(mimeType);
    }

    if (offers.isEmpty()) {
        qInfo() << "Could not find a plugin to handle" << fileName;
        return nullptr;
    }

    QMimeDatabase db;
    QMimeType mimeFromContent = db.mimeTypeForFile(fileName, QMimeDatabase::MatchContent);
    bool remove7zFlag = false;

    if (eType != APT_Cli7z) {
        if (((!bWrite) && (mimeType.name() == QString("application/zip") || mimeType.name() == QString("application/x-tar")))
            || ("application/x-tzo" == mimeType.name() && "application/x-lzop" == mimeFromContent.name())) {
            remove7zFlag = true;
        }
    }

    ReadOnlyArchiveInterface *pIface = nullptr;
    for (Plugin *plugin : offers) {
        if (remove7zFlag && plugin->metaData().name().contains("7zip", Qt::CaseInsensitive)) {
            continue;
        }

        switch (eType) {
        case APT_Auto:
            pIface = createInterface(fileName, mimeType, plugin);
            break;
        case APT_Cli7z:
            if (plugin->metaData().name().contains("7zip", Qt::CaseInsensitive)) {
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        case APT_Libarchive:
            if (plugin->metaData().name().contains("libarchive", Qt::CaseInsensitive)) {
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        case APT_Libzip:
            if (plugin->metaData().name().contains("libzip", Qt::CaseInsensitive)) {
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        case APT_Libpigz:
            if (plugin->metaData().name().contains("pigz", Qt::CaseInsensitive)) {
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        }

        // Use the first valid plugin, according to the priority sorting.
        if (pIface) {
            qInfo() << "select plugin: " << plugin->metaData().pluginId();
            break;
        }
    }

    return pIface;
}

ReadOnlyArchiveInterface *UiTools::createInterface(const QString &fileName, const CustomMimeType &mimeType, Plugin *plugin)
{
    Q_ASSERT(plugin);

    KPluginFactory *factory = KPluginLoader(plugin->metaData().fileName()).factory();
    if (!factory) {
        return nullptr;
    }

    const QVariantList args = { QVariant(QFileInfo(fileName).absoluteFilePath()),
                                QVariant().fromValue(plugin->metaData()),
                                QVariant::fromValue(mimeType) };

    ReadOnlyArchiveInterface *iface = factory->create<ReadOnlyArchiveInterface>(nullptr, args);
    return iface;
}

void UiTools::transSplitFileName(QString &fileName, UnCompressParameter::SplitType &eSplitType)
{
    if (fileName.contains(".7z.")) {
        QRegularExpression reg("^([\\s\\S]*\\.)7z\\.[0-9]{3}$");
        QRegularExpressionMatch match = reg.match(fileName);
        if (match.hasMatch()) {
            fileName = match.captured(1) + "7z.001";
            eSplitType = UnCompressParameter::ST_Other;
        }
    } else if (fileName.contains(".part") && fileName.endsWith(".rar")) {
        int x = fileName.lastIndexOf("part");
        int y = fileName.lastIndexOf(".");

        if ((y - x) > 5) {
            fileName.replace(x, y - x, "part01");
        } else {
            fileName.replace(x, y - x, "part1");
        }

        eSplitType = UnCompressParameter::ST_Other;
    } else if (fileName.contains(".zip.")) {
        QRegularExpression reg("^([\\s\\S]*\\.)zip\\.[0-9]{3}$");
        QRegularExpressionMatch match = reg.match(fileName);
        if (match.hasMatch()) {
            QFileInfo fi(match.captured(1) + "zip.001");
            if (fi.exists() == true) {
                fileName = match.captured(1) + "zip.001";
                eSplitType = UnCompressParameter::ST_Zip;
            }
        }
    } else if (fileName.endsWith(".zip")) {
        QFileInfo tmp(fileName.left(fileName.length() - 2) + "01");
        if (tmp.exists()) {
            eSplitType = UnCompressParameter::ST_Zip;
        }
    } else if (fileName.contains(".z")) {
        QRegularExpression reg("^([\\s\\S]*\\.)z[0-9]+$");   //
        QRegularExpressionMatch match = reg.match(fileName);
        if (match.hasMatch()) {
            fileName = match.captured(1) + "zip";
            QFileInfo fi(fileName);
            if (fi.exists()) {
                eSplitType = UnCompressParameter::ST_Zip;
            }
        }
    }

    QFileInfo info(fileName);
    if (info.isSymLink()) {
        fileName = info.symLinkTarget();
    }
}

QString UiTools::handleFileName(const QString &strFileName)
{
    QFileInfo info(strFileName);
    QString strName = info.completeBaseName();

    if (info.filePath().contains(".tar.")) {
        strName = strName.remove(".tar");
    } else if (info.filePath().contains(".7z.")) {
        strName = strName.remove(".7z");
    } else if (info.filePath().contains(".part01.rar")) {
        strName = strName.remove(".part01");
    } else if (info.filePath().contains(".part1.rar")) {
        strName = strName.remove(".part1");
    } else if (info.filePath().contains(".zip.")) {
        strName = strName.remove(".zip");
    }

    return strName;
}

bool UiTools::isLocalDeviceFile(const QString &strFileName)
{
    QStorageInfo info(strFileName);
    return info.device().startsWith("/dev/");
}

QStringList UiTools::removeSameFileName(const QStringList &listFiles)
{
    QStringList listResult;
    QStringList listName;

    for (int i = 0; i < listFiles.count(); ++i) {
        QFileInfo info(listFiles[i]);
        if (listName.contains(info.fileName())) {
            continue;
        } else {
            listResult << listFiles[i];
            listName << info.fileName();
        }
    }

    return listResult;
}

bool UiTools::isWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else {
        return false;
    }
}
