/*
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "appimageextractor.h"

// KF
#include <KDesktopFile>
// Qt
#include <QTextDocument>
#include <QDomDocument>
#include <QTemporaryFile>
#include <QLocale>
// libappimage
#include <appimage/appimage.h>

using namespace KFileMetaData;


namespace {
namespace AttributeNames {
QString xml_lang() { return QStringLiteral("xml:lang"); }
}
}


// helper class to extract the interesting data from the appdata file
// prefers localized strings over unlocalized, using system locale
class AppDataParser
{
public:
    AppDataParser(const char* appImageFilePath, const QString& appdataFilePath);

public:
    QString summary() const        { return !m_localized.summary.isEmpty() ? m_localized.summary : m_unlocalized.summary; }
    QString description() const    { return !m_localized.description.isEmpty() ? m_localized.description : m_unlocalized.description; }
    QString developerName() const  { return !m_localized.developerName.isEmpty() ? m_localized.developerName : m_unlocalized.developerName; }
    QString projectLicense() const { return m_projectLicense; }

private:
    void extractDescription(const QDomElement& e, const QString& localeName);

private:
    struct Data {
        QString summary;
        QString description;
        QString developerName;
    };
    Data m_localized;
    Data m_unlocalized;
    QString m_projectLicense;
};


AppDataParser::AppDataParser(const char* appImageFilePath, const QString& appdataFilePath)
{
    if (appdataFilePath.isEmpty()) {
        return;
    }

    unsigned long size = 0L;
    char* buf = nullptr;
    bool ok = appimage_read_file_into_buffer_following_symlinks(appImageFilePath,
                                                                qUtf8Printable(appdataFilePath),
                                                                &buf,
                                                                &size);

    QScopedPointer<char, QScopedPointerPodDeleter> cleanup(buf);

    if (!ok) {
        return;
    }

    QDomDocument domDocument;
    if (!domDocument.setContent(QByteArray::fromRawData(buf, size))) {
        return;
    }

    QDomElement docElem = domDocument.documentElement();
    if (docElem.tagName() != QLatin1String("component")) {
        return;
    }

    const auto localeName = QLocale::system().bcp47Name();

    QDomElement ec = docElem.firstChildElement();
    while (!ec.isNull()) {
        const auto tagName = ec.tagName();
        const auto hasLangAttribute = ec.hasAttribute(AttributeNames::xml_lang());
        const auto matchingLocale = hasLangAttribute && (ec.attribute(AttributeNames::xml_lang()) == localeName);
        if (matchingLocale || !hasLangAttribute) {
            if (tagName == QLatin1String("summary")) {
                Data& data = hasLangAttribute ? m_localized : m_unlocalized;
                data.summary = ec.text();
            } else if (tagName == QLatin1String("description")) {
                extractDescription(ec, localeName);
            } else if (tagName == QLatin1String("developer_name")) {
                Data& data = hasLangAttribute ? m_localized : m_unlocalized;
                data.developerName = ec.text();
            } else if (tagName == QLatin1String("project_license")) {
                m_projectLicense = ec.text();
            }
        }
        ec = ec.nextSiblingElement();
    }
}

using DesriptionDomFilter = std::function<bool(const QDomElement& e)>;

void stripDescriptionTextElements(QDomElement& element, const DesriptionDomFilter& stripFilter)
{
    auto childElement = element.firstChildElement();
    while (!childElement.isNull()) {
        auto nextChildElement = childElement.nextSiblingElement();

        const auto tagName = childElement.tagName();
        const bool isElementToFilter = (tagName == QLatin1String("p")) || (tagName == QLatin1String("li"));
        if (isElementToFilter && stripFilter(childElement)) {
            element.removeChild(childElement);
        } else {
            stripDescriptionTextElements(childElement, stripFilter);
        }

        childElement = nextChildElement;
    }
}

void AppDataParser::extractDescription(const QDomElement& e, const QString& localeName)
{
    // create fake html from it and let QTextDocument transform it to plain text for us
    QDomDocument descriptionDocument;
    auto htmlElement = descriptionDocument.createElement(QStringLiteral("html"));
    descriptionDocument.appendChild(htmlElement);

    // first localized...
    auto clonedE = descriptionDocument.importNode(e, true).toElement();
    clonedE.setTagName(QStringLiteral("body"));
    stripDescriptionTextElements(clonedE, [localeName](const QDomElement& e) {
        return !e.hasAttribute(AttributeNames::xml_lang()) ||
            e.attribute(AttributeNames::xml_lang()) != localeName;
    });
    htmlElement.appendChild(clonedE);

    QTextDocument textDocument;
    textDocument.setHtml(descriptionDocument.toString(-1));

    m_localized.description = textDocument.toPlainText().trimmed();

    if (!m_localized.description.isEmpty()) {
        // localized will be preferred, no need to calculate unlocalized one
        return;
    }

    // then unlocalized if still needed
    htmlElement.removeChild(clonedE); // reuse descriptionDocument
    clonedE = descriptionDocument.importNode(e, true).toElement();
    clonedE.setTagName(QStringLiteral("body"));
    stripDescriptionTextElements(clonedE, [](const QDomElement& e) {
        return e.hasAttribute(AttributeNames::xml_lang());
    });
    htmlElement.appendChild(clonedE);

    textDocument.setHtml(descriptionDocument.toString(-1));

    m_unlocalized.description = textDocument.toPlainText().trimmed();
}


// helper class to extract the interesting data from the desktop file
class DesktopFileParser
{
public:
    DesktopFileParser(const char* appImageFilePath, const QString& desktopFilePath);

public:
    QString name;
    QString comment;
};


DesktopFileParser::DesktopFileParser(const char* appImageFilePath, const QString& desktopFilePath)
{
    if (desktopFilePath.isEmpty()) {
        return;
    }

    unsigned long size = 0L;
    char* buf = nullptr;
    bool ok = appimage_read_file_into_buffer_following_symlinks(appImageFilePath,
                                                                qUtf8Printable(desktopFilePath),
                                                                &buf,
                                                                &size);

    QScopedPointer<char, QScopedPointerPodDeleter> cleanup(buf);

    if (!ok) {
        return;
    }

    // create real file, KDesktopFile needs that
    QTemporaryFile tmpDesktopFile;
    tmpDesktopFile.open();
    tmpDesktopFile.write(buf, size);
    tmpDesktopFile.close();

    KDesktopFile desktopFile(tmpDesktopFile.fileName());
    name = desktopFile.readName();
    comment = desktopFile.readComment();
}


AppImageExtractor::AppImageExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
}

QStringList AppImageExtractor::mimetypes() const
{
    return QStringList{
        QStringLiteral("application/x-iso9660-appimage"),
        QStringLiteral("application/vnd.appimage"),
    };
}

void KFileMetaData::AppImageExtractor::extract(ExtractionResult* result)
{
    const auto appImageFilePath = result->inputUrl().toUtf8();
    const auto appImageType = appimage_get_type(appImageFilePath.constData(), false);
    // not a valid appimage file?
    if (appImageType <= 0) {
        return;
    }

    // find desktop file and appdata file
    // need to scan ourselves, given there are no fixed names in the spec yet defined
    // and we just can try as the other appimage tools to simply use the first file of the type found
    char** filePaths = appimage_list_files(appImageFilePath.constData());
    if (!filePaths) {
        return;
    }

    QString desktopFilePath;
    QString appdataFilePath;
    for (int i = 0; filePaths[i] != nullptr; ++i) {
        const auto filePath = QString::fromUtf8(filePaths[i]);

        if (filePath.startsWith(QLatin1String("usr/share/metainfo/")) &&
            filePath.endsWith(QLatin1String(".appdata.xml"))) {
            appdataFilePath = filePath;
            if (!desktopFilePath.isEmpty()) {
                break;
            }
        }

        if (filePath.endsWith(QLatin1String(".desktop")) && !filePath.contains(QLatin1Char('/'))) {
            desktopFilePath = filePath;
            if (!appdataFilePath.isEmpty()) {
                break;
            }
        }
    }

    appimage_string_list_free(filePaths);

    // extract data from both files...
    const AppDataParser appData(appImageFilePath.constData(), appdataFilePath);

    const DesktopFileParser desktopFileData(appImageFilePath.constData(), desktopFilePath);

    // ... and insert into the result
    result->add(Property::Title, desktopFileData.name);

    if (!desktopFileData.comment.isEmpty()) {
        result->add(Property::Comment, desktopFileData.comment);
    } else if (!appData.summary().isEmpty()) {
        result->add(Property::Comment, appData.summary());
    }
    if (!appData.description().isEmpty()) {
        result->add(Property::Description, appData.description());
    }
    if (!appData.projectLicense().isEmpty()) {
        result->add(Property::License, appData.projectLicense());
    }
    if (!appData.developerName().isEmpty()) {
        result->add(Property::Author, appData.developerName());
    }
}

#include "moc_appimageextractor.cpp"
