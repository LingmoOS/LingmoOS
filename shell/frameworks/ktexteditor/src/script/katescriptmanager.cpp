/*
    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2006-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2010 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katescriptmanager.h"

#include <ktexteditor_version.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>
#include <QStringList>
#include <QUuid>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include "katecmd.h"
#include "katecommandlinescript.h"
#include "kateglobal.h"
#include "kateindentscript.h"
#include "katepartdebug.h"

KateScriptManager *KateScriptManager::m_instance = nullptr;

KateScriptManager::KateScriptManager()
    : KTextEditor::Command({QStringLiteral("reload-scripts")})
{
    // use cached info
    collect();
}

KateScriptManager::~KateScriptManager()
{
    qDeleteAll(m_indentationScripts);
    qDeleteAll(m_commandLineScripts);
    m_instance = nullptr;
}

KateIndentScript *KateScriptManager::indenter(const QString &language)
{
    KateIndentScript *highestPriorityIndenter = nullptr;
    const auto indenters = m_languageToIndenters.value(language.toLower());
    for (KateIndentScript *indenter : indenters) {
        // don't overwrite if there is already a result with a higher priority
        if (highestPriorityIndenter && indenter->indentHeader().priority() < highestPriorityIndenter->indentHeader().priority()) {
#ifdef DEBUG_SCRIPTMANAGER
            qCDebug(LOG_KTE) << "Not overwriting indenter for" << language << "as the priority isn't big enough (" << indenter->indentHeader().priority() << '<'
                             << highestPriorityIndenter->indentHeader().priority() << ')';
#endif
        } else {
            highestPriorityIndenter = indenter;
        }
    }

#ifdef DEBUG_SCRIPTMANAGER
    if (highestPriorityIndenter) {
        qCDebug(LOG_KTE) << "Found indenter" << highestPriorityIndenter->url() << "for" << language;
    } else {
        qCDebug(LOG_KTE) << "No indenter for" << language;
    }
#endif

    return highestPriorityIndenter;
}

/**
 * Small helper: QJsonValue to QStringList
 */
static QStringList jsonToStringList(const QJsonValue &value)
{
    QStringList list;

    const auto array = value.toArray();
    for (const QJsonValue &value : array) {
        if (value.isString()) {
            list.append(value.toString());
        }
    }

    return list;
}

void KateScriptManager::collect()
{
    // clear out the old scripts and reserve enough space
    qDeleteAll(m_indentationScripts);
    qDeleteAll(m_commandLineScripts);
    m_indentationScripts.clear();
    m_commandLineScripts.clear();

    m_languageToIndenters.clear();
    m_indentationScriptMap.clear();

    // now, we search all kinds of known scripts
    for (const auto &type : {QLatin1String("indentation"), QLatin1String("commands")}) {
        // basedir for filesystem lookup
        const QString basedir = QLatin1String("/katepart5/script/") + type;

        QStringList dirs;

        // first writable locations, e.g. stuff the user has provided
        dirs += QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + basedir;

        // then resources, e.g. the stuff we ship with us
        dirs.append(QLatin1String(":/ktexteditor/script/") + type);

        // then all other locations, this includes global stuff installed by other applications
        // this will not allow global stuff to overwrite the stuff we ship in our resources to allow to install a more up-to-date ktexteditor lib locally!
        const auto genericDataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const QString &dir : genericDataDirs) {
            dirs.append(dir + basedir);
        }

        QStringList list;
        for (const QString &dir : std::as_const(dirs)) {
            const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.js")});
            for (const QString &file : std::as_const(fileNames)) {
                list.append(dir + QLatin1Char('/') + file);
            }
        }

        // iterate through the files and read info out of cache or file, no double loading of same scripts
        QSet<QString> unique;
        for (const QString &fileName : std::as_const(list)) {
            // get file basename
            const QString baseName = QFileInfo(fileName).baseName();

            // only load scripts once, even if multiple installed variants found!
            if (unique.contains(baseName)) {
                continue;
            }

            // remember the script
            unique.insert(baseName);

            // open file or skip it
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                qCDebug(LOG_KTE) << "Script parse error: Cannot open file " << qPrintable(fileName) << '\n';
                continue;
            }

            // search json header or skip this file
            QByteArray fileContent = file.readAll();
            int startOfJson = fileContent.indexOf('{');
            if (startOfJson < 0) {
                qCDebug(LOG_KTE) << "Script parse error: Cannot find start of json header at start of file " << qPrintable(fileName) << '\n';
                continue;
            }

            int endOfJson = fileContent.indexOf("\n};", startOfJson);
            if (endOfJson < 0) { // as fallback, check also mac os line ending
                endOfJson = fileContent.indexOf("\r};", startOfJson);
            }
            if (endOfJson < 0) {
                qCDebug(LOG_KTE) << "Script parse error: Cannot find end of json header at start of file " << qPrintable(fileName) << '\n';
                continue;
            }
            endOfJson += 2; // we want the end including the } but not the ;

            // parse json header or skip this file
            QJsonParseError error;
            const QJsonDocument metaInfo(QJsonDocument::fromJson(fileContent.mid(startOfJson, endOfJson - startOfJson), &error));
            if (error.error || !metaInfo.isObject()) {
                qCDebug(LOG_KTE) << "Script parse error: Cannot parse json header at start of file " << qPrintable(fileName) << error.errorString() << endOfJson
                                 << fileContent.mid(endOfJson - 25, 25).replace('\n', ' ');
                continue;
            }

            // remember type
            KateScriptHeader generalHeader;
            if (type == QLatin1String("indentation")) {
                generalHeader.setScriptType(Kate::ScriptType::Indentation);
            } else if (type == QLatin1String("commands")) {
                generalHeader.setScriptType(Kate::ScriptType::CommandLine);
            } else {
                // should never happen, we dictate type by directory
                Q_ASSERT(false);
            }

            const QJsonObject metaInfoObject = metaInfo.object();
            generalHeader.setLicense(metaInfoObject.value(QStringLiteral("license")).toString());
            generalHeader.setAuthor(metaInfoObject.value(QStringLiteral("author")).toString());
            generalHeader.setRevision(metaInfoObject.value(QStringLiteral("revision")).toInt());
            generalHeader.setKateVersion(metaInfoObject.value(QStringLiteral("kate-version")).toString());

            // now, cast accordingly based on type
            switch (generalHeader.scriptType()) {
            case Kate::ScriptType::Indentation: {
                KateIndentScriptHeader indentHeader;
                indentHeader.setName(metaInfoObject.value(QStringLiteral("name")).toString());
                indentHeader.setBaseName(baseName);
                if (indentHeader.name().isNull()) {
                    qCDebug(LOG_KTE) << "Script value error: No name specified in script meta data: " << qPrintable(fileName) << '\n'
                                     << "-> skipping indenter" << '\n';
                    continue;
                }

                // required style?
                indentHeader.setRequiredStyle(metaInfoObject.value(QStringLiteral("required-syntax-style")).toString());
                // which languages does this support?
                QStringList indentLanguages = jsonToStringList(metaInfoObject.value(QStringLiteral("indent-languages")));
                if (!indentLanguages.isEmpty()) {
                    indentHeader.setIndentLanguages(indentLanguages);
                } else {
                    indentHeader.setIndentLanguages(QStringList() << indentHeader.name());

#ifdef DEBUG_SCRIPTMANAGER
                    qCDebug(LOG_KTE) << "Script value warning: No indent-languages specified for indent "
                                     << "script " << qPrintable(fileName) << ". Using the name (" << qPrintable(indentHeader.name()) << ")\n";
#endif
                }
                // priority
                indentHeader.setPriority(metaInfoObject.value(QStringLiteral("priority")).toInt());

                KateIndentScript *script = new KateIndentScript(fileName, indentHeader);
                script->setGeneralHeader(generalHeader);
                for (const QString &language : indentHeader.indentLanguages()) {
                    m_languageToIndenters[language.toLower()].push_back(script);
                }

                m_indentationScriptMap.insert(indentHeader.baseName(), script);
                m_indentationScripts.append(script);
                break;
            }
            case Kate::ScriptType::CommandLine: {
                KateCommandLineScriptHeader commandHeader;
                commandHeader.setFunctions(jsonToStringList(metaInfoObject.value(QStringLiteral("functions"))));
                commandHeader.setActions(metaInfoObject.value(QStringLiteral("actions")).toArray());
                if (commandHeader.functions().isEmpty()) {
                    qCDebug(LOG_KTE) << "Script value error: No functions specified in script meta data: " << qPrintable(fileName) << '\n'
                                     << "-> skipping script" << '\n';
                    continue;
                }
                KateCommandLineScript *script = new KateCommandLineScript(fileName, commandHeader);
                script->setGeneralHeader(generalHeader);
                m_commandLineScripts.push_back(script);
                break;
            }
            case Kate::ScriptType::Unknown:
            default:
                qCDebug(LOG_KTE) << "Script value warning: Unknown type ('" << qPrintable(type) << "'): " << qPrintable(fileName) << '\n';
            }
        }
    }

#ifdef DEBUG_SCRIPTMANAGER
    // XX Test
    if (indenter("Python")) {
        qCDebug(LOG_KTE) << "Python: " << indenter("Python")->global("triggerCharacters").isValid() << "\n";
        qCDebug(LOG_KTE) << "Python: " << indenter("Python")->function("triggerCharacters").isValid() << "\n";
        qCDebug(LOG_KTE) << "Python: " << indenter("Python")->global("blafldsjfklas").isValid() << "\n";
        qCDebug(LOG_KTE) << "Python: " << indenter("Python")->function("indent").isValid() << "\n";
    }
    if (indenter("C")) {
        qCDebug(LOG_KTE) << "C: " << qPrintable(indenter("C")->url()) << "\n";
    }
    if (indenter("lisp")) {
        qCDebug(LOG_KTE) << "LISP: " << qPrintable(indenter("Lisp")->url()) << "\n";
    }
#endif
}

void KateScriptManager::reload()
{
    collect();
    Q_EMIT reloaded();
}

/// Kate::Command stuff

bool KateScriptManager::exec(KTextEditor::View *view, const QString &_cmd, QString &errorMsg, const KTextEditor::Range &)
{
    Q_UNUSED(view)
    Q_UNUSED(errorMsg)

    const QList<QStringView> args = QStringView(_cmd).split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (args.isEmpty()) {
        return false;
    }

    const auto cmd = args.first();

    if (cmd == QLatin1String("reload-scripts")) {
        reload();
        return true;
    }

    return false;
}

bool KateScriptManager::help(KTextEditor::View *view, const QString &cmd, QString &msg)
{
    Q_UNUSED(view)

    if (cmd == QLatin1String("reload-scripts")) {
        msg = i18n("Reload all JavaScript files (indenters, command line scripts, etc).");
        return true;
    }

    return false;
}

#include "moc_katescriptmanager.cpp"
