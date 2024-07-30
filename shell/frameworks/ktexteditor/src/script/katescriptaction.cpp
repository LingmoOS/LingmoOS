/*
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katescriptaction.h"
#include "kateabstractinputmode.h"
#include "katecmd.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katepartdebug.h"
#include "katescriptmanager.h"
#include "kateview.h"
#include "kateviewhelpers.h"

#include <QJsonObject>
#include <QMenu>

#include <KActionCollection>
#include <KLocalizedString>
#include <KXMLGUIFactory>

// BEGIN KateScriptAction
KateScriptAction::KateScriptAction(const QString &cmd, const QJsonObject &action, KTextEditor::ViewPrivate *view)
    : QAction(i18nc("Script command name", action.value(QStringLiteral("name")).toString().toUtf8().data()), view)
    , m_view(view)
    , m_command(cmd)
    , m_interactive(action.value(QStringLiteral("interactive")).toBool())
{
    const QString icon = action.value(QStringLiteral("icon")).toString();
    if (!icon.isEmpty()) {
        setIcon(QIcon::fromTheme(icon));
    }

    connect(this, &KateScriptAction::triggered, this, &KateScriptAction::exec);
}

void KateScriptAction::exec()
{
    if (m_interactive) {
        m_view->currentInputMode()->launchInteractiveCommand(m_command + QLatin1Char(' '));
    } else {
        KTextEditor::Command *p = KateCmd::self()->queryCommand(m_command);
        if (p) {
            QString msg;
            p->exec(m_view, m_command, msg);
        }
    }
}
// END KateScriptAction

// BEGIN KateScriptActionMenu
KateScriptActionMenu::KateScriptActionMenu(KTextEditor::ViewPrivate *view, const QString &text)
    : KActionMenu(QIcon::fromTheme(QStringLiteral("code-context")), text, view)
    , m_view(view)
{
    repopulate();
    setPopupMode(QToolButton::InstantPopup);

    // on script-reload signal, repopulate script menu
    connect(KTextEditor::EditorPrivate::self()->scriptManager(), &KateScriptManager::reloaded, this, &KateScriptActionMenu::repopulate);
}

KateScriptActionMenu::~KateScriptActionMenu()
{
    cleanup();
}

void KateScriptActionMenu::cleanup()
{
    // delete menus and actions for real
    qDeleteAll(m_menus);
    m_menus.clear();

    qDeleteAll(m_actions);
    m_actions.clear();
}

void KateScriptActionMenu::repopulate()
{
    // if the view is already hooked into the GUI, first remove it
    // now and add it later, so that the changes we do here take effect
    KXMLGUIFactory *viewFactory = m_view->factory();
    if (viewFactory) {
        viewFactory->removeClient(m_view);
    }

    // remove existing menu actions
    cleanup();

    // now add all command line script commands
    QHash<QString, QMenu *> menus;
    const auto scripts = KTextEditor::EditorPrivate::self()->scriptManager()->commandLineScripts();
    for (KateCommandLineScript *script : scripts) {
        // traverse actions
        const auto &actions = script->commandHeader().actions();
        for (const auto &value : actions) {
            // action is a value
            const auto action = value.toObject();

            // get command
            const QString cmd = action.value(QStringLiteral("function")).toString();

            // show in a category submenu?
            QMenu *m = menu();
            QString category = action.value(QStringLiteral("category")).toString();
            if (!category.isEmpty()) {
                m = menus[category];
                if (!m) {
                    m = menu()->addMenu(i18nc("Script command category", category.toUtf8().data()));
                    menus.insert(category, m);
                    m_menus.append(m);
                    m_view->actionCollection()->addAction(QLatin1String("tools_scripts_") + category, m->menuAction());
                }
            }

            // create action + add to menu
            QAction *a = new KateScriptAction(cmd, action, m_view);
            m->addAction(a);
            m_view->actionCollection()->addAction(QLatin1String("tools_scripts_") + cmd, a);
            const QString shortcut = action.value(QStringLiteral("shortcut")).toString();
            if (!shortcut.isEmpty()) {
                m_view->actionCollection()->setDefaultShortcut(a, QKeySequence(shortcut, QKeySequence::PortableText));
            }

            m_actions.append(a);
        }
    }

    // finally add the view to the xml factory again, if it initially was there
    if (viewFactory) {
        viewFactory->addClient(m_view);
    }
}

// END KateScriptActionMenu
