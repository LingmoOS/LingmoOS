/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_MODEMANAGER_H
#define KATE_MODEMANAGER_H

#include "ktexteditor_export.h"

#include <QHash>
#include <QPointer>
#include <QStringList>

#include <KLocalizedString>

namespace KTextEditor
{
class Document;
}

class KateFileType
{
public:
    QString name;
    QString section;
    QStringList wildcards;
    QStringList mimetypes;
    int priority = 0;
    QString varLine;
    QString hl;
    bool hlGenerated = false;
    QString version;
    QString indenter;

    QString translatedName;
    QString translatedSection;

    QString nameTranslated() const
    {
        return translatedName.isEmpty() ? name : translatedName;
    }

    QString sectionTranslated() const
    {
        return translatedSection.isEmpty() ? section : translatedSection;
    }

    KateFileType()

    {
    }
};

class KateModeManager
{
public:
    KateModeManager();
    ~KateModeManager();

    KateModeManager(const KateModeManager &) = delete;
    KateModeManager &operator=(const KateModeManager &) = delete;

    /**
     * File Type Config changed, update all docs (which will take care of views/renderers)
     */
    void update();

    void save(const QList<KateFileType *> &v);

    /**
     * @return the right KateFileType name for the given document or an empty string if none found
     */
    QString fileType(KTextEditor::Document *doc, const QString &fileToReadFrom);

    /**
     * Don't store the pointer somewhere longer times, won't be valid after the next update()
     */
    const KateFileType &fileType(const QString &name) const;

    /**
     * Don't modify
     */
    const QList<KateFileType *> &list() const
    {
        return m_types;
    }

private:
    friend class KateModeManagerTest;
    friend class KateModeManagerBenchmark;

    KTEXTEDITOR_EXPORT QString wildcardsFind(const QString &fileName) const; // exported for testing
    KTEXTEDITOR_EXPORT QString mimeTypesFind(const QString &mimeTypeName) const; // exported for testing

    QList<KateFileType *> m_types;
    QHash<QString, KateFileType *> m_name2Type;
};

#endif
