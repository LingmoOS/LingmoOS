/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "katemodemenu.h"

#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katepartdebug.h"

#include <QActionGroup>
#include <QMenu>
// END Includes

void KateModeMenu::init()
{
    m_doc = nullptr;

    connect(menu(), &QMenu::triggered, this, &KateModeMenu::setType);

    connect(menu(), &QMenu::aboutToShow, this, &KateModeMenu::slotAboutToShow);

    m_actionGroup = new QActionGroup(menu());
}

void KateModeMenu::updateMenu(KTextEditor::Document *doc)
{
    m_doc = static_cast<KTextEditor::DocumentPrivate *>(doc);
}

void KateModeMenu::slotAboutToShow()
{
    KTextEditor::DocumentPrivate *doc = m_doc;
    int count = KTextEditor::EditorPrivate::self()->modeManager()->list().count();

    for (int z = 0; z < count; z++) {
        QString nameRaw = KTextEditor::EditorPrivate::self()->modeManager()->list().at(z)->name;
        QString hlName = KTextEditor::EditorPrivate::self()->modeManager()->list().at(z)->nameTranslated();
        QString hlSection = KTextEditor::EditorPrivate::self()->modeManager()->list().at(z)->sectionTranslated();

        if (hlName.isEmpty()) {
            continue;
        }
        if (!hlSection.isEmpty() && !names.contains(hlName)) {
            if (!subMenusName.contains(hlSection)) {
                subMenusName << hlSection;
                // pass proper parent for cleanup + Wayland correctness
                QMenu *qmenu = new QMenu(hlSection, menu());
                connect(qmenu, &QMenu::triggered, this, &KateModeMenu::setType);
                subMenus.append(qmenu);
                menu()->addMenu(qmenu);
            }

            int m = subMenusName.indexOf(hlSection);
            names << hlName;
            QAction *action = subMenus.at(m)->addAction(hlName);
            m_actionGroup->addAction(action);
            action->setCheckable(true);
            action->setData(nameRaw);
        } else if (!names.contains(hlName)) {
            names << hlName;

            disconnect(menu(), &QMenu::triggered, this, &KateModeMenu::setType);
            connect(menu(), &QMenu::triggered, this, &KateModeMenu::setType);

            QAction *action = menu()->addAction(hlName);
            m_actionGroup->addAction(action);
            action->setCheckable(true);
            action->setData(nameRaw);
        }
    }

    if (!doc) {
        return;
    }

    for (int i = 0; i < subMenus.count(); i++) {
        QList<QAction *> actions = subMenus.at(i)->actions();
        for (int j = 0; j < actions.count(); ++j) {
            actions[j]->setChecked(false);
        }
    }

    QList<QAction *> actions = menu()->actions();
    for (int i = 0; i < actions.count(); ++i) {
        actions[i]->setChecked(false);
    }

    if (doc->fileType().isEmpty() || doc->fileType() == QLatin1String("Normal")) {
        for (int i = 0; i < actions.count(); ++i) {
            if (actions[i]->data().toString() == QLatin1String("Normal")) {
                actions[i]->setChecked(true);
            }
        }
    } else {
        if (!doc->fileType().isEmpty()) {
            const KateFileType &t = KTextEditor::EditorPrivate::self()->modeManager()->fileType(doc->fileType());
            int i = subMenusName.indexOf(t.sectionTranslated());
            if (i >= 0 && subMenus.at(i)) {
                QList<QAction *> actions = subMenus.at(i)->actions();
                for (int j = 0; j < actions.count(); ++j) {
                    if (actions[j]->data().toString() == doc->fileType()) {
                        actions[j]->setChecked(true);
                    }
                }
            } else {
                QList<QAction *> actions = menu()->actions();
                for (int j = 0; j < actions.count(); ++j) {
                    if (actions[j]->data().toString().isEmpty()) {
                        actions[j]->setChecked(true);
                    }
                }
            }
        }
    }
}

void KateModeMenu::setType(QAction *action)
{
    KTextEditor::DocumentPrivate *doc = m_doc;

    if (doc) {
        doc->updateFileType(action->data().toString(), true);
    }
}
