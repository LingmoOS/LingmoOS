/*
    SPDX-FileCopyrightText: 2001, 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SYNTAX_MANAGER_H
#define KATE_SYNTAX_MANAGER_H

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include <QObject>

#include <memory>
#include <unordered_map>

class KateHighlighting;

class KateHlManager : public QObject
{
    Q_OBJECT

public:
    KateHlManager() = default;

    static KateHlManager *self();

    KateHighlighting *getHl(int n);
    int nameFind(const QString &name) const;

    void reload();

Q_SIGNALS:
    void changed();

    //
    // methods to get the default style count + names
    //
public:
    /**
     * Get the mode list
     * @return mode list
     */
    QList<KSyntaxHighlighting::Definition> modeList() const
    {
        return m_repository.definitions();
    }

    /**
     * Get repository.
     * @return repository
     */
    KSyntaxHighlighting::Repository &repository()
    {
        return m_repository;
    }

    /**
     * Get repository (const).
     * @return repository
     */
    const KSyntaxHighlighting::Repository &repository() const
    {
        return m_repository;
    }

    /**
     * Sorted list of KSyntaxHighlighting themes.
     * @return list sorted by translated names for e.g. menus/...
     */
    QList<KSyntaxHighlighting::Theme> sortedThemes() const;

private:
    /**
     * Syntax highlighting definitions.
     */
    KSyntaxHighlighting::Repository m_repository;

    /**
     * All loaded highlightings.
     */
    std::unordered_map<QString, std::unique_ptr<KateHighlighting>> m_hlDict;
};
#endif
