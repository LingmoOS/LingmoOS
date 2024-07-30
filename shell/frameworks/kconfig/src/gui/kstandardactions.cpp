/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kstandardactions.h"
#include "kstandardactions_p.h"
#include "moc_kstandardactions_p.cpp"

#include <KStandardShortcutWatcher>
#include <QGuiApplication>

namespace KStandardActions
{

QList<StandardAction> actionIds()
{
    QList<StandardAction> result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        result.append(g_rgActionInfo[i].id);
    }

    return result;
}

KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id)
{
    const KStandardActionsInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->idAccel : KStandardShortcut::AccelNone;
}

QAction *_kgui_createInternal(StandardAction id, QObject *parent)
{
    QAction *pAction = new QAction(parent);
    const KStandardActionsInfo *pInfo = infoPtr(id);

    // qCDebug(KCONFIG_WIDGETS_LOG) << "KStandardActions::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << " )"; //
    // ellis

    if (pInfo) {
        QString sLabel;
        QString iconName = pInfo->psIconName.toString();

        switch (id) {
        case Back:
            sLabel = QCoreApplication::translate("go back", "&Back");
            if (QGuiApplication::isRightToLeft()) {
                iconName = QStringLiteral("go-next");
            }
            break;

        case Forward:
            sLabel = QCoreApplication::translate("go forward", "&Forward");
            if (QGuiApplication::isRightToLeft()) {
                iconName = QStringLiteral("go-previous");
            }
            break;

        case Home:
            sLabel = QCoreApplication::translate("home page", "&Home");
            break;
        case Preferences:
        case AboutApp:
        case HelpContents: {
            QString appDisplayName = QGuiApplication::applicationDisplayName();
            if (appDisplayName.isEmpty()) {
                appDisplayName = QCoreApplication::applicationName();
            }
            sLabel = QCoreApplication::translate("KStandardActions", pInfo->psLabel).arg(appDisplayName);
        } break;
        default:
            sLabel = QCoreApplication::translate("KStandardActions", pInfo->psLabel);
        }

        if (QGuiApplication::isRightToLeft()) {
            switch (id) {
            case Prior:
                iconName = QStringLiteral("go-next-view-page");
                break;
            case Next:
                iconName = QStringLiteral("go-previous-view-page");
                break;
            case FirstPage:
                iconName = QStringLiteral("go-last-view-page");
                break;
            case LastPage:
                iconName = QStringLiteral("go-first-view-page");
                break;
            case DocumentBack:
                iconName = QStringLiteral("go-next");
                break;
            case DocumentForward:
                iconName = QStringLiteral("go-previous");
                break;
            default:
                break;
            }
        }

        if (id == Donate) {
            const QString currencyCode = QLocale().currencySymbol(QLocale::CurrencyIsoCode).toLower();
            if (!currencyCode.isEmpty()) {
                iconName = QStringLiteral("help-donate-%1").arg(currencyCode);
            }
        }

        QIcon icon = iconName.isEmpty() ? QIcon() : QIcon::fromTheme(iconName);

        // Set the text before setting the MenuRole, as on OS X setText will do some heuristic role guessing.
        // This ensures user menu items get the intended role out of the list below.
        pAction->setText(sLabel);

        switch (id) {
        case Quit:
            pAction->setMenuRole(QAction::QuitRole);
            break;

        case Preferences:
            pAction->setMenuRole(QAction::PreferencesRole);
            break;

        case AboutApp:
            pAction->setMenuRole(QAction::AboutRole);
            break;

        default:
            pAction->setMenuRole(QAction::NoRole);
            break;
        }

        if (!QCoreApplication::translate("KStandardActions", pInfo->psToolTip).isEmpty()) {
            pAction->setToolTip(QCoreApplication::translate("KStandardActions", pInfo->psToolTip));
        }
        pAction->setIcon(icon);

        QList<QKeySequence> cut = KStandardShortcut::shortcut(pInfo->idAccel);
        if (!cut.isEmpty()) {
            // emulate KActionCollection::setDefaultShortcuts to allow the use of "configure shortcuts"
            pAction->setShortcuts(cut);
            pAction->setProperty("defaultShortcuts", QVariant::fromValue(cut));
        }
        pAction->connect(KStandardShortcut::shortcutWatcher(),
                         &KStandardShortcut::StandardShortcutWatcher::shortcutChanged,
                         pAction,
                         [pAction, shortcut = pInfo->idAccel](KStandardShortcut::StandardShortcut id, const QList<QKeySequence> &newShortcut) {
                             if (id != shortcut) {
                                 return;
                             }
                             pAction->setShortcuts(newShortcut);
                             pAction->setProperty("defaultShortcuts", QVariant::fromValue(newShortcut));
                         });

        pAction->setObjectName(pInfo->psName.toString());
    }

    if (pAction && parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, pAction->objectName()), Q_ARG(QAction *, pAction));
    }

    return pAction;
}

QString name(StandardAction id)
{
    const KStandardActionsInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->psName.toString() : QString();
}
}
