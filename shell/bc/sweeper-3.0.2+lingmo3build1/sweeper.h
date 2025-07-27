/*
    SPDX-FileCopyrightText: 2003-2005 Ralf Hoelzer <ralf@well.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SWEEPER_H
#define SWEEPER_H

#include <KConfigGroup>
#include <KXmlGuiWindow>

#include <QVector>

#include "ui_sweeperdialog.h"

class PrivacyAction;
class QTreeWidgetItem;

class Sweeper: public KXmlGuiWindow
{
   public:
      // if automatic is true, no user interaction is required
      explicit Sweeper(bool automatic);
      ~Sweeper() override;

      void cleanup();

   private:
      void selectAll();
      void selectNone();

      /**
       * methods
       */
      void load();
      void save();

      /**
       * Set up all the actions we are going to manage.
       */
      void InitActions();

      /**
       * attributes
       */
      Ui::SweeperDialog ui;

      QVector<PrivacyAction*> checklist;

      QTreeWidgetItem * const m_generalCLI;
      QTreeWidgetItem * const m_webbrowsingCLI;

      KConfigGroup m_privacyConfGroup;

      bool m_automatic;
};

#endif

// kate: tab-width 3; indent-mode cstyle; replace-tabs true;
