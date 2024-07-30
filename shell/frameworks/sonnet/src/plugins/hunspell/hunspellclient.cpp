/*
 * kspell_hunspellclient.cpp
 *
 * SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "hunspellclient.h"
#include "hunspelldebug.h"
#include "hunspelldict.h"

#include <QDir>
#include <QStandardPaths>
#include <QString>

using namespace Sonnet;

HunspellClient::HunspellClient(QObject *parent)
    : Client(parent)
{
    qCDebug(SONNET_HUNSPELL) << " HunspellClient::HunspellClient";

    QStringList dirList;
    // search QStandardPaths
    dirList.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("hunspell"), QStandardPaths::LocateDirectory));

    auto maybeAddPath = [&dirList](const QString &path) {
        if (QFileInfo::exists(path)) {
            dirList.append(path);

            QDir dir(path);
            for (const QString &subDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                dirList.append(dir.absoluteFilePath(subDir));
            }
        }
    };
#ifdef Q_OS_WIN
    maybeAddPath(QStringLiteral(SONNET_INSTALL_PREFIX "/bin/data/hunspell/"));
#else
    maybeAddPath(QStringLiteral("/System/Library/Spelling"));
    maybeAddPath(QStringLiteral("/usr/share/hunspell/"));
    maybeAddPath(QStringLiteral("/usr/share/myspell/"));
#endif

    for (const QString &dirString : dirList) {
        QDir dir(dirString);
        const QList<QFileInfo> dicts = dir.entryInfoList({QStringLiteral("*.aff")}, QDir::Files);
        for (const QFileInfo &dict : dicts) {
            const QString language = dict.baseName();
            if (dict.isSymbolicLink()) {
                const QFileInfo actualDict(dict.canonicalFilePath());
                const QString alias = actualDict.baseName();
                if (language != alias) {
                    qCDebug(SONNET_HUNSPELL) << "Found alias" << language << "->" << alias;
                    m_languageAliases.insert(language, alias);
                    continue;
                }
            } else {
                m_languagePaths.insert(language, dict.canonicalPath());
            }
        }
    }
}

HunspellClient::~HunspellClient()
{
}

SpellerPlugin *HunspellClient::createSpeller(const QString &inputLang)
{
    QString language = inputLang;
    if (m_languageAliases.contains(language)) {
        qCDebug(SONNET_HUNSPELL) << "Using alias" << m_languageAliases.value(language) << "for" << language;
        language = m_languageAliases.value(language);
    }
    std::shared_ptr<Hunspell> hunspell = m_hunspellCache.value(language).lock();
    if (!hunspell) {
        hunspell = HunspellDict::createHunspell(language, m_languagePaths.value(language));
        m_hunspellCache.insert(language, hunspell);
    }
    qCDebug(SONNET_HUNSPELL) << " SpellerPlugin *HunspellClient::createSpeller(const QString &language) ;" << language;
    HunspellDict *ad = new HunspellDict(inputLang, hunspell);
    return ad;
}

QStringList HunspellClient::languages() const
{
    return m_languagePaths.keys() + m_languageAliases.keys();
}

#include "moc_hunspellclient.cpp"
