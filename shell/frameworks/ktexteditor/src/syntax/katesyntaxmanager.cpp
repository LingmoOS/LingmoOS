/*
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003, 2004 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2003 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katesyntaxmanager.h"

#include "katedocument.h"
#include "kateglobal.h"
#include "katehighlight.h"

KateHlManager *KateHlManager::self()
{
    return KTextEditor::EditorPrivate::self()->hlManager();
}

QList<KSyntaxHighlighting::Theme> KateHlManager::sortedThemes() const
{
    // get KSyntaxHighlighting themes
    auto themes = repository().themes();

    // sort by translated name
    std::sort(themes.begin(), themes.end(), [](const KSyntaxHighlighting::Theme &left, const KSyntaxHighlighting::Theme &right) {
        return left.translatedName().compare(right.translatedName(), Qt::CaseInsensitive) < 0;
    });
    return themes;
}

KateHighlighting *KateHlManager::getHl(int n)
{
    // default to "None", must exist
    const auto modeList = this->modeList();

    if (n < 0 || n >= modeList.count()) {
        n = nameFind(QStringLiteral("None"));
        Q_ASSERT(n >= 0);
    }

    const auto &mode = modeList.at(n);

    auto it = m_hlDict.find(mode.name());
    if (it == m_hlDict.end()) {
        std::unique_ptr<KateHighlighting> hl(new KateHighlighting(mode));
        it = m_hlDict.emplace(mode.name(), std::move(hl)).first;
    }
    return it->second.get();
}

int KateHlManager::nameFind(const QString &name) const
{
    const auto modeList = this->modeList();
    int idx = 0;
    for (const auto &mode : modeList) {
        if (mode.name().compare(name, Qt::CaseInsensitive) == 0) {
            return idx;
        }
        idx++;
    }

    return -1;
}

void KateHlManager::reload()
{
    // we need to ensure the KateHighlight objects survive until the end of this function
    std::unordered_map<QString, std::unique_ptr<KateHighlighting>> keepHighlighingsAlive;
    keepHighlighingsAlive.swap(m_hlDict);

    // recreate repository
    // this might even remove highlighting modes known before
    m_repository.reload();

    // let all documents use the new highlighters
    // will be created on demand
    // if old hl not found, use none
    const auto docs = KTextEditor::EditorPrivate::self()->documents();
    for (auto doc : docs) {
        auto hlMode = doc->highlightingMode();
        if (nameFind(hlMode) < 0) {
            hlMode = QStringLiteral("None");
        }
        doc->setHighlightingMode(hlMode);
    }

    // emit reloaded signal for our editor instance
    Q_EMIT KTextEditor::EditorPrivate::self()->repositoryReloaded(KTextEditor::EditorPrivate::self());
}

#include "moc_katesyntaxmanager.cpp"
