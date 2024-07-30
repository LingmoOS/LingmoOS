/*
 * SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "kopenaction_p.h"

#include <KRecentFilesAction>
#include <KStandardAction>

#include <QMenu>
#include <QPointer>

class KOpenActionPrivate
{
public:
    KOpenActionPrivate(KOpenAction *q);

    void updatePopupMode();
    void onPopupMenuAboutToShow();

    KOpenAction *q;
    QPointer<KRecentFilesAction> recentFilesAction;
};

KOpenActionPrivate::KOpenActionPrivate(KOpenAction *q)
    : q(q)
{
}

KOpenAction::~KOpenAction() = default;

void KOpenActionPrivate::updatePopupMode()
{
    if (recentFilesAction && recentFilesAction->isEnabled()) {
        q->setPopupMode(KToolBarPopupAction::MenuButtonPopup);
    } else {
        q->setPopupMode(KToolBarPopupAction::NoPopup);
    }
}

void KOpenActionPrivate::onPopupMenuAboutToShow()
{
    q->popupMenu()->clear();

    if (recentFilesAction) {
        // Using the menu explicitly rather than the actions so we also get the "Forget..." entry.
        if (auto *recentMenu = recentFilesAction->menu()) {
            // Trigger a menu update.
            Q_EMIT recentMenu->aboutToShow();

            const auto actions = recentMenu->actions();
            for (QAction *action : actions) {
                q->popupMenu()->addAction(action);
            }
        }
    }
}

KOpenAction::KOpenAction(QObject *parent)
    : KOpenAction(QIcon(), QString(), parent)
{
}

KOpenAction::KOpenAction(const QIcon &icon, const QString &text, QObject *parent)
    : KToolBarPopupAction(icon, text, parent)
    , d(new KOpenActionPrivate(this))
{
    setPopupMode(KToolBarPopupAction::NoPopup);
    connect(popupMenu(), &QMenu::aboutToShow, this, [this] {
        d->onPopupMenuAboutToShow();
    });
}

QWidget *KOpenAction::createWidget(QWidget *parentWidget)
{
    if (!d->recentFilesAction) {
        // Find the accompanying file_open_recent action.
        QAction *action = nullptr;

        if (parent() && parent()->inherits("KActionCollection")) {
            const QString openRecentActionId = KStandardAction::name(KStandardAction::OpenRecent);
            QMetaObject::invokeMethod(parent(), "action", Q_RETURN_ARG(QAction *, action), Q_ARG(QString, openRecentActionId));
        }

        d->recentFilesAction = qobject_cast<KRecentFilesAction *>(action);
        if (d->recentFilesAction) {
            connect(d->recentFilesAction.data(), &QAction::enabledChanged, this, [this] {
                d->updatePopupMode();
            });
        }
    }

    d->updatePopupMode();

    return KToolBarPopupAction::createWidget(parentWidget);
}

#include "moc_kopenaction_p.cpp"
