/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_MODEMENU_H
#define KATE_MODEMENU_H

#include <QHash>
#include <QPointer>
#include <QStringList>

#include "katedialogs.h"
#include "katemodemanager.h"

namespace KTextEditor
{
class DocumentPrivate;
}

class KateModeMenu : public KActionMenu
{
public:
    KateModeMenu(const QString &text, QObject *parent)
        : KActionMenu(text, parent)
    {
        init();
        setPopupMode(QToolButton::InstantPopup);
    }

    void updateMenu(KTextEditor::Document *doc);

private:
    void init();

    QPointer<KTextEditor::DocumentPrivate> m_doc;
    QStringList subMenusName;
    QStringList names;
    QList<QMenu *> subMenus;
    QActionGroup *m_actionGroup;

public:
    void slotAboutToShow();

private:
    void setType(QAction *);
};

#endif
