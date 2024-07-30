/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow.h"

#include "guiactivateevent.h"
#include "part.h"

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KEditToolBar>
#include <KHelpMenu>
#include <KSharedConfig>
#include <KXMLGUIFactory>

#include <QAction>
#include <QApplication>
#include <QPointer>
#include <QStatusBar>

using namespace KParts;

namespace KParts
{
class MainWindowPrivate
{
public:
    MainWindowPrivate()
        : m_activePart(nullptr)
    {
    }
    ~MainWindowPrivate()
    {
    }

    QPointer<Part> m_activePart;
    bool m_bShellGUIActivated = false;
    KHelpMenu *m_helpMenu = nullptr;
    bool m_manageWindowTitle = true;
};
}

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f)
    , d(new MainWindowPrivate())
{
    PartBase::setPartObject(this);
}

MainWindow::~MainWindow() = default;

void MainWindow::createGUI(Part *part)
{
#if 0
    // qDebug() << "part=" << part
            << (part ? part->metaObject()->className() : "")
            << (part ? part->objectName() : "");
#endif
    KXMLGUIFactory *factory = guiFactory();

    Q_ASSERT(factory);

    if (d->m_activePart) {
#if 0
        // qDebug() << "deactivating GUI for" << d->m_activePart
                << d->m_activePart->metaObject()->className()
                << d->m_activePart->objectName();
#endif

        GUIActivateEvent ev(false);
        QApplication::sendEvent(d->m_activePart, &ev);

        factory->removeClient(d->m_activePart);

        disconnect(d->m_activePart.data(), &Part::setWindowCaption, this, static_cast<void (MainWindow::*)(const QString &)>(&MainWindow::setCaption));
        disconnect(d->m_activePart.data(), &Part::setStatusBarText, this, &MainWindow::slotSetStatusBarText);
    }

    if (!d->m_bShellGUIActivated) {
        createShellGUI();
        d->m_bShellGUIActivated = true;
    }

    if (part) {
        // do this before sending the activate event
        if (d->m_manageWindowTitle) {
            connect(part, &Part::setWindowCaption, this, static_cast<void (MainWindow::*)(const QString &)>(&MainWindow::setCaption));
        }
        connect(part, &Part::setStatusBarText, this, &MainWindow::slotSetStatusBarText);

        factory->addClient(part);

        GUIActivateEvent ev(true);
        QApplication::sendEvent(part, &ev);
    }

    d->m_activePart = part;
}

void MainWindow::slotSetStatusBarText(const QString &text)
{
    statusBar()->showMessage(text);
}

void MainWindow::createShellGUI(bool create)
{
    Q_ASSERT(d->m_bShellGUIActivated != create);
    d->m_bShellGUIActivated = create;
    if (create) {
        if (isHelpMenuEnabled() && !d->m_helpMenu) {
            d->m_helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);

            KActionCollection *actions = actionCollection();
            QAction *helpContentsAction = d->m_helpMenu->action(KHelpMenu::menuHelpContents);
            QAction *whatsThisAction = d->m_helpMenu->action(KHelpMenu::menuWhatsThis);
            QAction *reportBugAction = d->m_helpMenu->action(KHelpMenu::menuReportBug);
            QAction *switchLanguageAction = d->m_helpMenu->action(KHelpMenu::menuSwitchLanguage);
            QAction *aboutAppAction = d->m_helpMenu->action(KHelpMenu::menuAboutApp);
            QAction *aboutKdeAction = d->m_helpMenu->action(KHelpMenu::menuAboutKDE);
            QAction *donateAction = d->m_helpMenu->action(KHelpMenu::menuDonate);

            if (helpContentsAction) {
                actions->addAction(helpContentsAction->objectName(), helpContentsAction);
            }
            if (whatsThisAction) {
                actions->addAction(whatsThisAction->objectName(), whatsThisAction);
            }
            if (reportBugAction) {
                actions->addAction(reportBugAction->objectName(), reportBugAction);
            }
            if (switchLanguageAction) {
                actions->addAction(switchLanguageAction->objectName(), switchLanguageAction);
            }
            if (aboutAppAction) {
                actions->addAction(aboutAppAction->objectName(), aboutAppAction);
            }
            if (aboutKdeAction) {
                actions->addAction(aboutKdeAction->objectName(), aboutKdeAction);
            }
            if (donateAction) {
                actions->addAction(donateAction->objectName(), donateAction);
            }
        }

        QString f = xmlFile();
        setXMLFile(KXMLGUIClient::standardsXmlFileLocation());
        if (!f.isEmpty()) {
            setXMLFile(f, true);
        } else {
            QString auto_file(componentName() + QLatin1String("ui.rc"));
            setXMLFile(auto_file, true);
        }

        GUIActivateEvent ev(true);
        QApplication::sendEvent(this, &ev);

        guiFactory()->addClient(this);

        checkAmbiguousShortcuts();
    } else {
        GUIActivateEvent ev(false);
        QApplication::sendEvent(this, &ev);

        guiFactory()->removeClient(this);
    }
}

void KParts::MainWindow::setWindowTitleHandling(bool enabled)
{
    d->m_manageWindowTitle = enabled;
}

void KParts::MainWindow::saveNewToolbarConfig()
{
    createGUI(d->m_activePart);
    KConfigGroup cg(KSharedConfig::openConfig(), QString());
    applyMainWindowSettings(cg);
}

void KParts::MainWindow::configureToolbars()
{
    // No difference with base class anymore.
    KXmlGuiWindow::configureToolbars();
}

#include "moc_mainwindow.cpp"
