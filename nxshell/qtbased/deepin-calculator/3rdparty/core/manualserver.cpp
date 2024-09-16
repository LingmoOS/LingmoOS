// This file is part of the SpeedCrunch project
// Copyright (C) 2016 Pol Welter <polwelter@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.


//#include "manualserver.h"
//#include "settings.h"

//#include <QtCore/QDir>
//#include <QtCore/QLocale>
//#include <QFile>
//#include <QtHelp/QHelpEngineCore>
//#include <QString>
//#include <QMap>
//#include <QEvent>

//#define FALLBACK_LANG "en_US"

/*
 * #define COPY_OVERWRITE(source, dest) \
    if (QFile::exists(dest)) \
        QFile::remove(dest); \
    QFile::copy(source, dest);
*/

//#define QHC_NAME(lang) ("manual-" + QString(lang) + ".qhc")
//#define QCH_NAME(lang) ("manual-" + QString(lang) + ".qch")
//#define QHC_RES_PATH(lang) (":/manual/" + QHC_NAME(lang))
//#define QCH_RES_PATH(lang) (":/manual/" + QCH_NAME(lang))

//ManualServer *ManualServer::s_instance = NULL;

//QString ManualServer::deployDocs()
//{
//    QString dest = Settings::getCachePath() + "/manual/";
//    QDir qdir;
//    qdir.mkpath(dest);
//    QString lang = Settings::instance()->language;
//    m_deployedLanguage = lang;
//    if (lang == "C")
//        lang = QLocale().name();

//    if (!isSupportedLanguage(lang)) {
//        // Extract the language from the locale and try to find a generic translation for it
//        QStringList languages = QLocale((QLocale(lang).language())).uiLanguages();
//        lang = FALLBACK_LANG;
//        for (int i = 0; i < languages.size(); ++i) {
//            QString extra_lang = languages[i].replace('-', '_');
//            if (isSupportedLanguage(extra_lang)) {
//                lang = extra_lang;
//                break;
//            }
//        }
//    }

//    QFile::remove(dest + QHC_NAME(lang));
//    QFile::remove(dest + QCH_NAME(lang));

//    QFile::copy(QHC_RES_PATH(lang), dest + QHC_NAME(lang));
//    QFile::copy(QCH_RES_PATH(lang), dest + QCH_NAME(lang));

//    QFile qhc(dest + QHC_NAME(lang));
//    qhc.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
//    QFile qch(dest + QCH_NAME(lang));
//    qch.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

//    return dest + "manual-" + lang + ".qhc";
//}

//bool ManualServer::isSupportedLanguage(const QString &lang)
//{
//    return (QFile(QHC_RES_PATH(lang)).exists() && QFile(QCH_RES_PATH(lang)).exists());
//}

//void ManualServer::setupHelpEngine()
//{
//    QString collectionFile = deployDocs() ;

//    m_helpEngine = new QHelpEngineCore(collectionFile, this);

//    QStringList filters = m_helpEngine->customFilters();
//    if (!filters.isEmpty())
//        m_helpEngine->setCurrentFilter(filters.first());
//}

//ManualServer *ManualServer::instance()
//{
//    if (!s_instance)
//        s_instance = new ManualServer();
//    return s_instance;
//}

//bool ManualServer::URLforKeyword(const QString id, QUrl &result)
//{
//    ensureCorrectLanguage();
//    result = "";
//    if (!m_helpEngine)
//        return 0;
//    QMap<QString, QUrl> l;
//    l = m_helpEngine->linksForIdentifier(id);
//    if (l.isEmpty())
//        return 0;
//    result = l.first();
//    return 1;
//}

//QByteArray ManualServer::fileData(const QUrl &url)
//{
//    ensureCorrectLanguage();
//    return m_helpEngine->fileData(url);
//}

//void ManualServer::languageChanged()
//{
//    setupHelpEngine();
//}

//void ManualServer::ensureCorrectLanguage()
//{
//    if (Settings::instance()->language != m_deployedLanguage)
//        languageChanged();
//}

//ManualServer::ManualServer()
//{
//    m_helpEngine = NULL;
//}
