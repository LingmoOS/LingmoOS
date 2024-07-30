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

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kfontsizeaction.h"

#include "kselectaction_p.h"

#include "loggingcategory.h"

#include <QFontDatabase>

class KFontSizeActionPrivate : public KSelectActionPrivate
{
    Q_DECLARE_PUBLIC(KFontSizeAction)

public:
    KFontSizeActionPrivate(KFontSizeAction *qq)
        : KSelectActionPrivate(qq)
    {
    }

    void init();
};

// BEGIN KFontSizeAction
KFontSizeAction::KFontSizeAction(QObject *parent)
    : KSelectAction(*new KFontSizeActionPrivate(this), parent)
{
    Q_D(KFontSizeAction);

    d->init();
}

KFontSizeAction::KFontSizeAction(const QString &text, QObject *parent)
    : KSelectAction(*new KFontSizeActionPrivate(this), parent)
{
    Q_D(KFontSizeAction);

    setText(text);
    d->init();
}

KFontSizeAction::KFontSizeAction(const QIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(*new KFontSizeActionPrivate(this), parent)
{
    Q_D(KFontSizeAction);

    setIcon(icon);
    setText(text);
    d->init();
}

KFontSizeAction::~KFontSizeAction() = default;

void KFontSizeActionPrivate::init()
{
    Q_Q(KFontSizeAction);

    q->setEditable(true);
    const QList<int> sizes = QFontDatabase::standardSizes();
    QStringList lst;
    lst.reserve(sizes.count());
    for (QList<int>::ConstIterator it = sizes.begin(), total = sizes.end(); it != total; ++it) {
        lst.append(QString::number(*it));
    }

    q->setItems(lst);
}

void KFontSizeAction::setFontSize(int size)
{
    if (size == fontSize()) {
        const QString test = QString::number(size);
        const auto actions = this->actions();
        for (QAction *action : actions) {
            if (action->text() == test) {
                setCurrentAction(action);
                return;
            }
        }
    }

    if (size < 1) {
        qCWarning(KWidgetsAddonsLog) << "KFontSizeAction: Size " << size << " is out of range";
        return;
    }

    QAction *a = action(QString::number(size));
    if (!a) {
        // Insert at the correct position in the list (to keep sorting)
        QList<int> lst;
        // Convert to list of ints
        QStringListIterator itemsIt(items());
        while (itemsIt.hasNext()) {
            lst.append(itemsIt.next().toInt());
        }
        // New size
        lst.append(size);
        // Sort the list
        std::sort(lst.begin(), lst.end());
        clear();
        for (int it : std::as_const(lst)) {
            QAction *const action = addAction(QString::number(it));
            if (it == size) {
                setCurrentAction(action);
            }
        }

    } else {
        setCurrentAction(a);
    }
}

int KFontSizeAction::fontSize() const
{
    return currentText().toInt();
}

void KFontSizeAction::slotActionTriggered(QAction *action)
{
    Q_EMIT fontSizeChanged(action->text().toInt());
    KSelectAction::slotActionTriggered(action);
}

#include "moc_kfontsizeaction.cpp"
