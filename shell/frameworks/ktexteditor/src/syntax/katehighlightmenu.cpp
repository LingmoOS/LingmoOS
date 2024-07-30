/*
    SPDX-FileCopyrightText: 2001-2003 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "katehighlightmenu.h"

#include "katedocument.h"
#include "katesyntaxmanager.h"

#include <KLocalizedString>

#include <QActionGroup>
#include <QMenu>
// END Includes

void KateHighlightingMenu::init()
{
    m_doc = nullptr;

    connect(menu(), &QMenu::aboutToShow, this, &KateHighlightingMenu::slotAboutToShow);
    m_actionGroup = new QActionGroup(menu());
}

void KateHighlightingMenu::updateMenu(KTextEditor::DocumentPrivate *doc)
{
    m_doc = doc;
}

void KateHighlightingMenu::slotAboutToShow()
{
    const auto modeList = KateHlManager::self()->modeList();
    for (const auto &hl : modeList) {
        QString hlName = hl.translatedName();
        QString hlSection = hl.translatedSection();
        if (hlName == QLatin1String("None")) {
            hlName = i18n("None");
        }

        if (!hl.isHidden() && !hlName.isEmpty()) {
            const bool namesHaveHlName = std::find(names.begin(), names.end(), hlName) != names.end();

            if (!hlSection.isEmpty() && !namesHaveHlName) {
                auto it = std::find(subMenusName.begin(), subMenusName.end(), hlSection);
                if (it == subMenusName.end()) {
                    subMenusName.push_back(hlSection);
                    // pass proper parent for cleanup + Wayland correctness
                    subMenus.emplace_back(new QMenu(QLatin1Char('&') + hlSection, menu()));
                    menu()->addMenu(subMenus.back());

                    // last element is the one we just inserted
                    it = --subMenusName.end();
                }

                const auto m = std::distance(subMenusName.begin(), it);
                names.push_back(hlName);
                QAction *a = subMenus.at(m)->addAction(QLatin1Char('&') + hlName, this, SLOT(setHl()));
                m_actionGroup->addAction(a);
                a->setData(hl.name());
                a->setCheckable(true);
                subActions.push_back(a);
            } else if (!namesHaveHlName) {
                names.push_back(hlName);
                QAction *a = menu()->addAction(QLatin1Char('&') + hlName, this, SLOT(setHl()));
                m_actionGroup->addAction(a);
                a->setData(hl.name());
                a->setCheckable(true);
                subActions.push_back(a);
            }
        }
    }

    if (!m_doc) {
        return;
    }
    const QString mode = m_doc->highlightingMode();
    for (auto subAction : subActions) {
        subAction->setChecked(subAction->data().toString() == mode);
    }
}

void KateHighlightingMenu::setHl()
{
    if (!m_doc || !sender()) {
        return;
    }
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    QString mode = action->data().toString();
    m_doc->setHighlightingMode(mode);

    // use change, honor this
    m_doc->setDontChangeHlOnSave();
}

#include "moc_katehighlightmenu.cpp"
