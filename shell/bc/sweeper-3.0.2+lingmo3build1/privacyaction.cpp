/*
    SPDX-FileCopyrightText: 2005 Brian S. Stephan <bssteph@irtonline.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "privacyaction.h"

PrivacyAction::PrivacyAction(QTreeWidgetItem * parent, const QString &name, const QString &desc)
   : QTreeWidgetItem(parent)
{
   setText(0, name);
   if (!desc.isEmpty()) {
      setText(1, desc);
   }
   setCheckState(0, Qt::Unchecked);
}

// kate: tab-width 3; indent-mode cstyle; replace-tabs true;
