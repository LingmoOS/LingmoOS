/*
    SPDX-FileCopyrightText: 2003-2005 Ralf Hoelzer <ralf@well.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "sweeper.h"

#include "privacyaction.h"
#include "privacyfunctions.h"
#include "sweeperadaptor.h"

#include <KActionCollection>
#include <KConfig>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardAction>
#include <QDBusConnection>

Sweeper::Sweeper(bool automatic)
   : KXmlGuiWindow(nullptr)
   , m_generalCLI(new QTreeWidgetItem(QStringList(i18nc("General system content", "General"))))
   , m_webbrowsingCLI(new QTreeWidgetItem(QStringList(i18nc("Web browsing content", "Web Browsing"))))
   , m_privacyConfGroup(KSharedConfig::openConfig(QStringLiteral("kprivacyrc"), KConfig::NoGlobals), "Cleaning")
   , m_automatic(automatic)
{
   auto mainWidget = new QWidget(this);
   ui.setupUi(mainWidget);
   setCentralWidget(mainWidget);

   QTreeWidget *sw = ui.privacyListView;

   KStandardAction::quit(this, &Sweeper::close, actionCollection());

   createGUI(QStringLiteral("sweeperui.rc"));

   setAutoSaveSettings();

   sw->addTopLevelItem(m_generalCLI);
   sw->addTopLevelItem(m_webbrowsingCLI);

   m_generalCLI->setExpanded(true);
   m_webbrowsingCLI->setExpanded(true);

   this->InitActions();


   connect(ui.cleanupButton, &QPushButton::clicked, this, &Sweeper::cleanup);
   connect(ui.selectAllButton, &QPushButton::clicked, this, &Sweeper::selectAll);
   connect(ui.selectNoneButton, &QPushButton::clicked, this, &Sweeper::selectNone);

   new KsweeperAdaptor(this);
   QDBusConnection::sessionBus().registerObject(QStringLiteral("/ksweeper"), this);

   load();

   if(automatic) {
      cleanup();
      close();
   }
}


Sweeper::~Sweeper()
{
   save();
}


void Sweeper::load()
{
   for (auto &&act: checklist) {
      act->setCheckState(0, m_privacyConfGroup.readEntry(act->configKey(), true) ? Qt::Checked : Qt::Unchecked);
   }
}

void Sweeper::save()
{
   for (auto &&act: checklist) {
      m_privacyConfGroup.writeEntry(act->configKey(), act->checkState(0) == Qt::Checked);
   }

   m_privacyConfGroup.sync();
}

void Sweeper::selectAll()
{
   for (auto &&act: checklist) {
      act->setCheckState(0, Qt::Checked);
   }
}

void Sweeper::selectNone()
{
   for (auto &&act: checklist) {
      act->setCheckState(0, Qt::Unchecked);
   }
}

void Sweeper::cleanup()
{
   if (!m_automatic) {
      if (KMessageBox::warningContinueCancel(this, i18n("You are deleting data that is potentially valuable to you. Are you sure?")) != KMessageBox::Continue) {
         return;
      }
   }

   ui.statusTextEdit->clear();
   ui.statusTextEdit->setText(i18n("Starting cleanup..."));

   for (auto &&act: checklist) {
      if(act->checkState(0) == Qt::Checked) {
         QString statusText = i18n("Clearing %1...", act->text(0));
         ui.statusTextEdit->append(statusText);

         // actions return whether they were successful
         if(!act->action()) {
            QString errorText = i18n("Clearing of %1 failed: %2", act->text(0), act->getErrMsg());
            ui.statusTextEdit->append(errorText);
         }
      }
   }

   ui.statusTextEdit->append(i18n("Clean up finished."));
}

void Sweeper::InitActions() {
    // store all entries in a list for easy access later on
   if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.klipper"))) {
        checklist.append(new ClearSavedClipboardContentsAction(m_generalCLI));
   }
   checklist.append(new ClearRecentDocumentsAction(m_generalCLI));
   checklist.append(new ClearRunCommandHistoryAction(m_generalCLI));
   if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.ActivityManager"))) {
        checklist.append( new ClearRecentApplicationAction( m_generalCLI ) );
   }
   checklist.append(new ClearThumbnailsAction(m_generalCLI));

   checklist.append(new ClearAllCookiesAction(m_webbrowsingCLI));
   checklist.append(new ClearFaviconsAction(m_webbrowsingCLI));
   checklist.append(new ClearWebHistoryAction(m_webbrowsingCLI));
   checklist.append(new ClearWebCacheAction(m_webbrowsingCLI));
   checklist.append(new ClearFormCompletionAction(m_webbrowsingCLI));
   checklist.append(new ClearAllCookiesPoliciesAction(m_webbrowsingCLI));

   ui.privacyListView->resizeColumnToContents(0);
}

// kate: tab-width 3; indent-mode cstyle; replace-tabs true;
