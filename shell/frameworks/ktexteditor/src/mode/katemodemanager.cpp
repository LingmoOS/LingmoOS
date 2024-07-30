/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "katemodemanager.h"
#include "katemodemenulist.h"
#include "katestatusbar.h"

#include "document.h"
#include "kateconfig.h"
#include "kateglobal.h"
#include "katepartdebug.h"
#include "katesyntaxmanager.h"
#include "kateview.h"

#include <KConfigGroup>
#include <KSyntaxHighlighting/WildcardMatcher>

#include <QFileInfo>
#include <QMimeDatabase>

#include <algorithm>
#include <limits>
// END Includes

static QStringList vectorToList(const QList<QString> &v)
{
    QStringList l;
    l.reserve(v.size());
    std::copy(v.begin(), v.end(), std::back_inserter(l));
    return l;
}

KateModeManager::KateModeManager()
{
    update();
}

KateModeManager::~KateModeManager()
{
    qDeleteAll(m_types);
}

bool compareKateFileType(const KateFileType *const left, const KateFileType *const right)
{
    int comparison = left->translatedSection.compare(right->translatedSection, Qt::CaseInsensitive);
    if (comparison == 0) {
        comparison = left->translatedName.compare(right->translatedName, Qt::CaseInsensitive);
    }
    return comparison < 0;
}

//
// read the types from config file and update the internal list
//
void KateModeManager::update()
{
    KConfig config(QStringLiteral("katemoderc"), KConfig::NoGlobals);

    QStringList g(config.groupList());

    KateFileType *normalType = nullptr;
    qDeleteAll(m_types);
    m_types.clear();
    m_name2Type.clear();
    for (int z = 0; z < g.count(); z++) {
        KConfigGroup cg(&config, g[z]);

        KateFileType *type = new KateFileType();
        type->name = g[z];
        type->wildcards = cg.readXdgListEntry(QStringLiteral("Wildcards"));
        type->mimetypes = cg.readXdgListEntry(QStringLiteral("Mimetypes"));
        type->priority = cg.readEntry(QStringLiteral("Priority"), 0);
        type->varLine = cg.readEntry(QStringLiteral("Variables"));
        type->indenter = cg.readEntry(QStringLiteral("Indenter"));

        type->hl = cg.readEntry(QStringLiteral("Highlighting"));

        // only for generated types...
        type->hlGenerated = cg.readEntry(QStringLiteral("Highlighting Generated"), false);

        // the "Normal" mode will be added later
        if (type->name == QLatin1String("Normal")) {
            if (!normalType) {
                normalType = type;
            }
        } else {
            type->section = cg.readEntry(QStringLiteral("Section"));
            type->version = cg.readEntry(QStringLiteral("Highlighting Version"));

            // already add all non-highlighting generated user types
            if (!type->hlGenerated) {
                m_types.append(type);
            }
        }

        // insert into the hash...
        // NOTE: "katemoderc" could have modes that do not exist or are invalid (for example, custom
        // XML files that were deleted or renamed), so they will be added to the list "m_types" later
        m_name2Type.insert(type->name, type);
    }

    // try if the hl stuff is up to date...
    const auto modes = KateHlManager::self()->modeList();
    for (int i = 0; i < modes.size(); ++i) {
        // filter out hidden languages; and
        // filter out "None" hl, we add that later as "Normal" mode.
        // Hl with empty names will also be filtered. The
        // KTextEditor::DocumentPrivate::updateFileType() function considers
        // hl with empty names as invalid.
        if (modes[i].isHidden() || modes[i].name().isEmpty() || modes[i].name() == QLatin1String("None")) {
            continue;
        }

        KateFileType *type = nullptr;
        bool newType = false;
        if (m_name2Type.contains(modes[i].name())) {
            type = m_name2Type[modes[i].name()];

            // add if hl generated, we skipped that stuff above
            if (type->hlGenerated) {
                m_types.append(type);
            }
        } else {
            newType = true;
            type = new KateFileType();
            type->name = modes[i].name();
            type->priority = 0;
            type->hlGenerated = true;
            m_types.append(type);
            m_name2Type.insert(type->name, type);
        }

        if (newType || type->version != QString::number(modes[i].version())) {
            type->name = modes[i].name();
            type->section = modes[i].section();
            type->wildcards = vectorToList(modes[i].extensions());
            type->mimetypes = vectorToList(modes[i].mimeTypes());
            type->priority = modes[i].priority();
            type->version = QString::number(modes[i].version());
            type->indenter = modes[i].indenter();
            type->hl = modes[i].name();
        }

        type->translatedName = modes[i].translatedName();
        type->translatedSection = modes[i].translatedSection();
    }

    // sort the list...
    std::sort(m_types.begin(), m_types.end(), compareKateFileType);

    // add the none type...
    if (!normalType) {
        normalType = new KateFileType();
    }

    // marked by hlGenerated
    normalType->name = QStringLiteral("Normal");
    normalType->translatedName = i18n("Normal");
    normalType->hl = QStringLiteral("None");
    normalType->hlGenerated = true;

    m_types.prepend(normalType);

    // update the mode menu of the status bar, for all views.
    // this menu uses the KateFileType objects
    for (auto *view : KTextEditor::EditorPrivate::self()->views()) {
        if (view->statusBar() && view->statusBar()->modeMenu()) {
            view->statusBar()->modeMenu()->reloadItems();
        }
    }
}

//
// save the given list to config file + update
//
void KateModeManager::save(const QList<KateFileType *> &v)
{
    KConfig katerc(QStringLiteral("katemoderc"), KConfig::NoGlobals);

    QStringList newg;
    newg.reserve(v.size());
    for (const KateFileType *type : v) {
        KConfigGroup config(&katerc, type->name);

        config.writeEntry("Section", type->section);
        config.writeXdgListEntry("Wildcards", type->wildcards);
        config.writeXdgListEntry("Mimetypes", type->mimetypes);
        config.writeEntry("Priority", type->priority);
        config.writeEntry("Indenter", type->indenter);

        QString varLine = type->varLine;
        if (!varLine.contains(QLatin1String("kate:"))) {
            varLine.prepend(QLatin1String("kate: "));
        }

        config.writeEntry("Variables", varLine);

        config.writeEntry("Highlighting", type->hl);

        // only for generated types...
        config.writeEntry("Highlighting Generated", type->hlGenerated);
        config.writeEntry("Highlighting Version", type->version);

        newg << type->name;
    }

    const auto groupNames = katerc.groupList();
    for (const QString &groupName : groupNames) {
        if (newg.indexOf(groupName) == -1) {
            katerc.deleteGroup(groupName);
        }
    }

    katerc.sync();

    update();
}

QString KateModeManager::fileType(KTextEditor::Document *doc, const QString &fileToReadFrom)
{
    if (!doc) {
        return QString();
    }

    if (m_types.isEmpty()) {
        return QString();
    }

    QString fileName = doc->url().toString();
    int length = doc->url().toString().length();

    QString result;

    // Try wildcards
    if (!fileName.isEmpty()) {
        static const QLatin1String commonSuffixes[] = {
            QLatin1String(".orig"),
            QLatin1String(".new"),
            QLatin1String("~"),
            QLatin1String(".bak"),
            QLatin1String(".BAK"),
        };

        if (!(result = wildcardsFind(fileName)).isEmpty()) {
            return result;
        }

        QString backupSuffix = KateDocumentConfig::global()->backupSuffix();
        if (fileName.endsWith(backupSuffix)) {
            if (!(result = wildcardsFind(fileName.left(length - backupSuffix.length()))).isEmpty()) {
                return result;
            }
        }

        for (auto &commonSuffix : commonSuffixes) {
            if (commonSuffix != backupSuffix && fileName.endsWith(commonSuffix)) {
                if (!(result = wildcardsFind(fileName.left(length - commonSuffix.size()))).isEmpty()) {
                    return result;
                }
            }
        }
    }

    // either read the file passed to this function (pre-load) or use the normal mimeType() KF KTextEditor API
    QString mtName;
    if (!fileToReadFrom.isEmpty()) {
        mtName = QMimeDatabase().mimeTypeForFile(fileToReadFrom).name();
    } else {
        mtName = doc->mimeType();
    }
    return mimeTypesFind(mtName);
}

template<typename UnaryStringPredicate>
static QString findHighestPriorityTypeNameIf(const QList<KateFileType *> &types, QStringList KateFileType::*list, UnaryStringPredicate anyOfCondition)
{
    const KateFileType *match = nullptr;
    auto matchPriority = std::numeric_limits<int>::lowest();
    for (const KateFileType *type : types) {
        if (type->priority > matchPriority && std::any_of((type->*list).cbegin(), (type->*list).cend(), anyOfCondition)) {
            match = type;
            matchPriority = type->priority;
        }
    }
    return match == nullptr ? QString() : match->name;
}

QString KateModeManager::wildcardsFind(const QString &fileName) const
{
    const auto fileNameNoPath = QFileInfo{fileName}.fileName();
    return findHighestPriorityTypeNameIf(m_types, &KateFileType::wildcards, [&fileNameNoPath](const QString &wildcard) {
        return KSyntaxHighlighting::WildcardMatcher::exactMatch(fileNameNoPath, wildcard);
    });
}

QString KateModeManager::mimeTypesFind(const QString &mimeTypeName) const
{
    return findHighestPriorityTypeNameIf(m_types, &KateFileType::mimetypes, [&mimeTypeName](const QString &name) {
        return mimeTypeName == name;
    });
}

const KateFileType &KateModeManager::fileType(const QString &name) const
{
    for (int i = 0; i < m_types.size(); ++i) {
        if (m_types[i]->name == name) {
            return *m_types[i];
        }
    }

    static KateFileType notype;
    return notype;
}
