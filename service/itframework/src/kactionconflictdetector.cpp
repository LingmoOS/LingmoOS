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
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QAction>
#include <QCoreApplication>
#include <QShortcutEvent>

#include <KLocalizedString>
#include <KMessageBox>

class KActionConflictDetector : public QObject
{
    Q_OBJECT
public:
    explicit KActionConflictDetector(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Shortcut && qobject_cast<QAction *>(watched)) {
            QShortcutEvent *se = static_cast<QShortcutEvent *>(event);
            if (se->isAmbiguous()) {
                KMessageBox::information(nullptr, // No widget to be seen around here
                                         i18n("The key sequence '%1' is ambiguous. Use 'Configure Keyboard Shortcuts'\n"
                                              "from the 'Settings' menu to solve the ambiguity.\n"
                                              "No action will be triggered.",
                                              se->key().toString(QKeySequence::NativeText)),
                                         i18nc("@title:window", "Ambiguous shortcut detected"));
                return true;
            }
        }

        return QObject::eventFilter(watched, event);
    }
};

void _k_installConflictDetector()
{
    QCoreApplication *app = QCoreApplication::instance();
    app->installEventFilter(new KActionConflictDetector(app));
}

Q_COREAPP_STARTUP_FUNCTION(_k_installConflictDetector)

#include "kactionconflictdetector.moc"
