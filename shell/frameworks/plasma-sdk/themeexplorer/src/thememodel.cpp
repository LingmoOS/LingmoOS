/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "thememodel.h"
#include "coloreditor.h"
#include "themelistmodel.h"
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include <KAboutData>
#include <KCompressionDevice>
#include <KConfigGroup>
#include <KIO/FileCopyJob>
#include <KIO/MkdirJob>
#include <KProcess>

#include <Plasma/Theme>

ThemeModel::ThemeModel(const KPackage::Package &package, QObject *parent)
    : QAbstractListModel(parent)
    , m_theme(new Plasma::Theme)
    , m_themeName(QStringLiteral("default"))
    , m_package(package)
    , m_themeListModel(new ThemeListModel(this))
    , m_colorEditor(new ColorEditor(this))
{
    m_theme->setUseGlobalSettings(false);
    m_theme->setThemeName(m_themeName);

    m_roleNames.insert(ImagePath, "imagePath");
    m_roleNames.insert(Description, "description");
    m_roleNames.insert(Delegate, "delegate");
    m_roleNames.insert(UsesFallback, "usesFallback");
    m_roleNames.insert(SvgAbsolutePath, "svgAbsolutePath");
    m_roleNames.insert(IsWritable, "isWritable");
    m_roleNames.insert(IconElements, "iconElements");
    m_roleNames.insert(FrameSvgPrefixes, "frameSvgPrefixes");

    load();
}

ThemeModel::~ThemeModel()
{
}

ThemeListModel *ThemeModel::themeList()
{
    return m_themeListModel;
}

ColorEditor *ThemeModel::colorEditor()
{
    return m_colorEditor;
}

QHash<int, QByteArray> ThemeModel::roleNames() const
{
    return m_roleNames;
}

int ThemeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_jsonDoc.array().size();
}

QVariant ThemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() > m_jsonDoc.array().size()) {
        return QVariant();
    }

    const QVariantMap value = m_jsonDoc.array().at(index.row()).toObject().toVariantMap();

    switch (role) {
    case ImagePath:
        return value.value("imagePath");
    case Description:
        return value.value("description");
    case Delegate:
        return value.value("delegate");
    case UsesFallback:
        return !m_theme->currentThemeHasImage(value.value("imagePath").toString());
    case SvgAbsolutePath: {
        QString path = m_theme->imagePath(value.value("imagePath").toString());
        if (!value.value("imagePath").toString().contains("translucent")) {
            path = path.replace("translucent/", "");
        }
        return path;
    }
    case IsWritable:
        return QFile::exists(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/desktoptheme/" + m_themeName);
    case IconElements:
    case FrameSvgPrefixes: {
        QString path = m_theme->imagePath(value.value("imagePath").toString());
        if (!value.value("imagePath").toString().contains("translucent")) {
            path = path.replace("translucent/", "");
        }
        KCompressionDevice file(path, KCompressionDevice::GZip);
        if (!file.open(QIODevice::ReadOnly)) {
            return QVariant();
        }

        QStringList ids, prefixes;
        QXmlStreamReader reader(&file);
        while (!reader.atEnd() && !reader.hasError()) {
            const auto token = reader.readNext();
            if (token != QXmlStreamReader::StartElement) {
                continue;
            }

            const QString id = reader.attributes().value("id").toString();
            if (!id.isEmpty() && !id.contains(QRegularExpression("\\d\\d$")) && id != "base" && !id.contains("layer")) {
                ids << id;
            }
            if (id.endsWith(QLatin1String("-center")) && !id.contains("hint-")) {
                // remove -center
                prefixes << id.mid(0, id.length() - 7);
            }
        }

        if (role == IconElements) {
            return ids;
        } else {
            return prefixes;
        }
    }
    }

    return QVariant();
}

void ThemeModel::load()
{
    beginResetModel();
    qDebug() << "Loading theme description file" << m_package.filePath("data", "themeDescription.json");

    QFile jsonFile(m_package.filePath("data", "themeDescription.json"));
    jsonFile.open(QIODevice::ReadOnly);

    QJsonParseError error;
    m_jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing Json" << error.errorString();
    }

    endResetModel();
}

QString ThemeModel::theme() const
{
    return m_themeName;
}

QString ThemeModel::author() const
{
    const QList<KAboutPerson> authors = m_theme->metadata().authors();
    return authors.isEmpty() ? authors.at(0).name() : QString();
}

QString ThemeModel::email() const
{
    const QList<KAboutPerson> authors = m_theme->metadata().authors();
    return authors.isEmpty() ? authors.at(0).emailAddress() : QString();
}

QString ThemeModel::license() const
{
    return m_theme->metadata().license();
}

QString ThemeModel::website() const
{
    return m_theme->metadata().website();
}

void ThemeModel::setTheme(const QString &theme)
{
    if (theme == m_themeName) {
        return;
    }

    m_themeName = theme;
    m_theme->setThemeName(theme);
    load();
    m_colorEditor->setTheme(theme);
    emit themeChanged();
}

void ThemeModel::editElement(const QString &imagePath)
{
    QString file = m_theme->imagePath(imagePath);
    if (!file.contains("translucent")) {
        file = file.replace("translucent/", "");
    }

    QString finalFile;

    if (m_theme->currentThemeHasImage(imagePath)) {
        finalFile = file;
    } else {
        finalFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/desktoptheme/" + m_themeName + "/" + imagePath + ".svgz";
        const QString dirPath = QFileInfo(finalFile).absoluteDir().absolutePath();
        KIO::mkdir(QUrl::fromLocalFile(dirPath))->exec();

        KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(file), QUrl::fromLocalFile(finalFile));
        if (!job->exec()) {
            qWarning() << "Error copying" << file << "to" << finalFile;
        }
    }

    // QProcess::startDetached("inkscape", QStringList() << finalFile);
    KProcess *process = new KProcess();
    // TODO: don't use the script to not depend from bash/linux?
    process->setProgram("bash", QStringList() << m_package.filePath("scripts", "openInEditor.sh") << finalFile);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ThemeModel::processFinished);
    process->start();
}

void ThemeModel::processFinished()
{
    /*We increment the microversion of the theme: keeps track and will force the cache to be
      discarded in order to reload immediately the graphics*/
    const QString metadataPath(
        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("plasma/desktoptheme/") % m_themeName % QLatin1String("/metadata.desktop")));
    KConfig c(metadataPath);
    KConfigGroup cg(&c, "Desktop Entry");

    QStringList version = cg.readEntry("X-KDE-PluginInfo-Version", "0.0").split('.');
    if (version.length() < 2) {
        version << QLatin1String("0");
    }
    if (version.length() < 3) {
        version << QLatin1String("0");
    }

    cg.writeEntry("X-KDE-PluginInfo-Version",
                  QString(version.first() + QLatin1String(".") + version[1] + QLatin1String(".") + QString::number(version.last().toInt() + 1)));
    cg.sync();
}

void ThemeModel::editThemeMetaData(const QString &name, const QString &author, const QString &email, const QString &license, const QString &website)
{
    QString compactName = name.toLower();
    compactName.replace(' ', QString());
    const QString metadataPath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QLatin1String("/plasma/desktoptheme/") % compactName
                               % QLatin1String("/metadata.desktop"));
    KConfig c(metadataPath);

    KConfigGroup cg(&c, "Desktop Entry");
    cg.writeEntry("X-KDE-PluginInfo-Name", name);
    cg.writeEntry("X-KDE-PluginInfo-Author", author);
    cg.writeEntry("X-KDE-PluginInfo-Email", email);
    cg.writeEntry("X-KDE-PluginInfo-Website", website);
    cg.writeEntry("X-KDE-PluginInfo-Category", "Plasma Theme");
    cg.writeEntry("X-KDE-PluginInfo-License", license);
    cg.writeEntry("X-KDE-PluginInfo-EnabledByDefault", "true");
    cg.writeEntry("X-Plasma-API", "5.0");
    cg.writeEntry("X-KDE-PluginInfo-Version", "0.1");
    cg.sync();

    KConfigGroup cg2(&c, "ContrastEffect");
    cg2.writeEntry("enabled", "true");
    cg2.writeEntry("contrast", "0.2");
    cg2.writeEntry("intensity", "2.0");
    cg2.writeEntry("saturation", "1.7");
    cg2.sync();
}

void ThemeModel::createNewTheme(const QString &name, const QString &author, const QString &email, const QString &license, const QString &website)
{
    editThemeMetaData(name, author, email, license, website);

    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, +"/plasma/desktoptheme/default/colors");

    QString compactName = name.toLower();
    compactName.replace(' ', QString());
    QString finalFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/desktoptheme/" + compactName + "/colors";

    KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(file), QUrl::fromLocalFile(finalFile));
    if (!job->exec()) {
        qWarning() << "Error copying" << file << "to" << finalFile;
    }

    m_themeListModel->reload();
}

QString ThemeModel::themeFolder()
{
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, +"plasma/desktoptheme/" + m_themeName, QStandardPaths::LocateDirectory);
}

#include "moc_thememodel.cpp"
