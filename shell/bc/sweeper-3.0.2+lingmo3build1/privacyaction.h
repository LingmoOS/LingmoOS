/*
    SPDX-FileCopyrightText: 2005 Brian S. Stephan <bssteph@irtonline.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PRIVACYACTION_H
#define PRIVACYACTION_H

#include <QTreeWidgetItem>

class PrivacyAction : public QTreeWidgetItem
{
   public:
      PrivacyAction(QTreeWidgetItem * parent, const QString &name, const QString &desc = QString());
      ~PrivacyAction() override = default;
      
      QString getErrMsg() const { return errMsg; }
      
      virtual bool action() = 0;
      virtual QString configKey() const = 0;
      
   protected:
      QString errMsg;
};

#endif

// kate: tab-width 3; indent-mode cstyle; replace-tabs true;
