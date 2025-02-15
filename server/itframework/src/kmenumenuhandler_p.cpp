/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Olivier Goffart <ogoffart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmenumenuhandler_p.h"

#include "debug.h"
#include "kactioncollection.h"
#include "kmainwindow.h"
#include "kshortcutwidget.h"
#include "ktoolbar.h"
#include "kxmlguibuilder.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDomDocument>
#include <QDomNode>
#include <QMenu>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KSelectAction>

namespace KDEPrivate
{
KMenuMenuHandler::KMenuMenuHandler(KXMLGUIBuilder *builder)
    : QObject()
    , m_builder(builder)
{
    m_toolbarAction = new KSelectAction(i18n("Add to Toolbar"), this);
    connect(m_toolbarAction, &KSelectAction::indexTriggered, this, &KMenuMenuHandler::slotAddToToolBar);
}

void KMenuMenuHandler::insertMenu(QMenu *popup)
{
    popup->installEventFilter(this);
}

bool KMenuMenuHandler::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        if (m_contextMenu && m_contextMenu->isVisible()) {
            m_contextMenu->hide();
            return true;
        }
        break;

    case QEvent::MouseButtonRelease:
        if (m_contextMenu && m_contextMenu->isVisible()) {
            return true;
        }
        break;

    case QEvent::ContextMenu: {
        QContextMenuEvent *e = static_cast<QContextMenuEvent *>(event);
        QMenu *menu = static_cast<QMenu *>(watched);
        if (e->reason() == QContextMenuEvent::Mouse) {
            showContextMenu(menu, e->pos());
        } else if (menu->activeAction()) {
            showContextMenu(menu, menu->actionGeometry(menu->activeAction()).center());
        }
    }
        event->accept();
        return true;

    default:
        break;
    }

    return false;
}

void KMenuMenuHandler::buildToolbarAction()
{
    KMainWindow *window = qobject_cast<KMainWindow *>(m_builder->widget());
    if (!window) {
        return;
    }
    QStringList toolbarlist;
    const auto toolbars = window->toolBars();
    toolbarlist.reserve(toolbars.size());
    for (KToolBar *b : toolbars) {
        toolbarlist << (b->windowTitle().isEmpty() ? b->objectName() : b->windowTitle());
    }
    m_toolbarAction->setItems(toolbarlist);
}

static KActionCollection *findParentCollection(KXMLGUIFactory *factory, QAction *action)
{
    const auto clients = factory->clients();
    for (KXMLGUIClient *client : clients) {
        KActionCollection *collection = client->actionCollection();
        // if the call to actions() is too slow, add KActionCollection::contains(QAction*).
        if (collection->actions().contains(action)) {
            return collection;
        }
    }
    return nullptr;
}

void KMenuMenuHandler::slotSetShortcut()
{
    if (!m_popupMenu || !m_popupAction) {
        return;
    }

    QDialog dialog(m_builder->widget());
    auto *layout = new QVBoxLayout(&dialog);

    KShortcutWidget swidget(&dialog);
    swidget.setShortcut(m_popupAction->shortcuts());
    layout->addWidget(&swidget);

    QDialogButtonBox box(&dialog);
    box.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(&box);

    KActionCollection *parentCollection = nullptr;
    if (dynamic_cast<KXMLGUIClient *>(m_builder)) {
        QList<KActionCollection *> checkCollections;
        KXMLGUIFactory *factory = dynamic_cast<KXMLGUIClient *>(m_builder)->factory();
        parentCollection = findParentCollection(factory, m_popupAction);
        const auto clients = factory->clients();
        checkCollections.reserve(clients.size());
        for (KXMLGUIClient *client : clients) {
            checkCollections += client->actionCollection();
        }
        swidget.setCheckActionCollections(checkCollections);
    }

    if (dialog.exec()) {
        m_popupAction->setShortcuts(swidget.shortcut());
        swidget.applyStealShortcut();
        if (parentCollection) {
            parentCollection->writeSettings();
        }
    }
}

void KMenuMenuHandler::slotAddToToolBar(int tb)
{
    KMainWindow *window = qobject_cast<KMainWindow *>(m_builder->widget());
    if (!window) {
        return;
    }

    if (!m_popupMenu || !m_popupAction) {
        return;
    }

    KXMLGUIFactory *factory = dynamic_cast<KXMLGUIClient *>(m_builder)->factory();
    QString actionName = m_popupAction->objectName(); // set by KActionCollection::addAction
    KActionCollection *collection = nullptr;
    if (factory) {
        collection = findParentCollection(factory, m_popupAction);
    }
    if (!collection) {
        qCWarning(DEBUG_KXMLGUI) << "Cannot find the action collection for action " << actionName;
        return;
    }

    KToolBar *toolbar = window->toolBars().at(tb);
    toolbar->addAction(m_popupAction);

    const KXMLGUIClient *client = collection->parentGUIClient();
    QString xmlFile = client->localXMLFile();
    QDomDocument document;
    document.setContent(KXMLGUIFactory::readConfigFile(client->xmlFile(), client->componentName()));
    QDomElement elem = document.documentElement().toElement();

    const QLatin1String tagToolBar("ToolBar");
    const QLatin1String attrNoEdit("noEdit");
    const QLatin1String attrName("name");

    QDomElement toolbarElem;
    QDomNode n = elem.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        QDomElement elem = n.toElement();
        if (!elem.isNull() && elem.tagName() == tagToolBar && elem.attribute(attrName) == toolbar->objectName()) {
            if (elem.attribute(attrNoEdit) == QLatin1String("true")) {
                qCWarning(DEBUG_KXMLGUI) << "The toolbar is not editable";
                return;
            }
            toolbarElem = elem;
            break;
        }
    }
    if (toolbarElem.isNull()) {
        toolbarElem = document.createElement(tagToolBar);
        toolbarElem.setAttribute(attrName, toolbar->objectName());
        elem.appendChild(toolbarElem);
    }

    KXMLGUIFactory::findActionByName(toolbarElem, actionName, true);
    KXMLGUIFactory::saveConfigFile(document, xmlFile);
}

void KMenuMenuHandler::showContextMenu(QMenu *menu, const QPoint &pos)
{
    Q_ASSERT(!m_popupMenu);
    Q_ASSERT(!m_popupAction);
    Q_ASSERT(!m_contextMenu);

    auto *action = menu->actionAt(pos);
    if (!action || action->isSeparator()) {
        return;
    }

    m_popupMenu = menu;
    m_popupAction = action;

    m_contextMenu = new QMenu;
    m_contextMenu->addAction(i18nc("@action:inmenu", "Configure Shortcut..."), this, &KMenuMenuHandler::slotSetShortcut);

    KMainWindow *window = qobject_cast<KMainWindow *>(m_builder->widget());
    if (window) {
        m_contextMenu->addAction(m_toolbarAction);
        buildToolbarAction();
    }

    m_contextMenu->exec(menu->mapToGlobal(pos));
    delete m_contextMenu;
    m_contextMenu = nullptr;

    m_popupAction = nullptr;
    m_popupMenu = nullptr;
}

} // END namespace KDEPrivate
