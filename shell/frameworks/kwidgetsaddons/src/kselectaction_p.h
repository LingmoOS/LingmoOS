/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2006 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2006 Clarence Dang <dang@kde.org>
    SPDX-FileCopyrightText: 2006 Michel Hermier <michel.hermier@gmail.com>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSELECTACTION_P_H
#define KSELECTACTION_P_H

#include <QActionGroup>
#include <QComboBox>

class KSelectActionPrivate
{
    Q_DECLARE_PUBLIC(KSelectAction)

public:
    KSelectActionPrivate(KSelectAction *qq)
        : q_ptr(qq)
    {
        m_edit = false;
        m_menuAccelsEnabled = true;
        m_comboWidth = -1;
        m_maxComboViewCount = -1;

        m_toolBarMode = KSelectAction::ComboBoxMode;
        m_toolButtonPopupMode = QToolButton::InstantPopup; // InstantPopup by default because there is no default action

        m_actionGroup = new QActionGroup(nullptr);
    }

    // virtual so the derived private class for e.g. KRecentFilesAction can be deleted correctly
    // and not leaked
    virtual ~KSelectActionPrivate()
    {
        // unhook the event filter, as the deletion of the actiongroup
        // will trigger it
        for (QComboBox *box : std::as_const(m_comboBoxes)) {
            box->removeEventFilter(q_ptr);
            QObject::disconnect(box, nullptr, q_ptr, nullptr); // To prevent a crash in comboBoxDeleted()
        }

        for (QToolButton *button : std::as_const(m_buttons)) {
            button->removeEventFilter(q_ptr);
        }
        delete m_actionGroup;
    }

    void comboBoxDeleted(QComboBox *combo);
    void comboBoxCurrentIndexChanged(int value);

    void init();

    bool m_edit : 1;
    bool m_menuAccelsEnabled : 1;
    int m_comboWidth;
    int m_maxComboViewCount;

    KSelectAction::ToolBarMode m_toolBarMode;
    QToolButton::ToolButtonPopupMode m_toolButtonPopupMode;

    QActionGroup *m_actionGroup;

    QList<QToolButton *> m_buttons;
    QList<QComboBox *> m_comboBoxes;

    QString makeMenuText(const QString &_text)
    {
        if (m_menuAccelsEnabled) {
            return _text;
        }
        QString text = _text;
        int i = 0;
        while (i < text.length()) {
            if (text[i] == QLatin1Char('&')) {
                text.insert(i, QLatin1Char('&'));
                i += 2;
            } else {
                ++i;
            }
        }
        return text;
    }

    KSelectAction *const q_ptr;
};

#endif // KSELECTACTION_P_H
