/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kshortcutschemeshelper_p.h"

#include <QAction>
#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include <KConfigGroup>
#include <KSharedConfig>
#include <QDir>

#include "debug.h"
#include "kactioncollection.h"
#include "kxmlguiclient.h"

bool KShortcutSchemesHelper::saveShortcutScheme(const QList<KActionCollection *> &collections, const QString &schemeName)
{
    // Every action collection is associated with a KXMLGUIClient
    // (at least if it was added by KXMLGUIFactory::configureShortcuts()
    // or KXMLGUIFactory::showConfigureShortcutsDialog())

    // Some GUI clients have the same name (e.g. the child client for a mainwindow
    // holding the actions for hiding/showing toolbars), so we need to save them
    // together, otherwise they will overwrite each other's files on disk.

    // For cases like kdevelop (many guiclients not reused in other apps) it's simpler
    // to even save all shortcuts to a single shortcuts file -> set the boolean below to true
    // to create an all-in-one scheme.
    // Maybe we need a checkbox for this? Or an env var for contributors to set, rather? End users don't care.
    const bool saveToApplicationFile = false;

    QMultiMap<QString, KActionCollection *> collectionsByClientName;
    for (KActionCollection *coll : collections) {
        const KXMLGUIClient *client = coll->parentGUIClient();
        if (client) {
            const QString key = saveToApplicationFile ? QCoreApplication::applicationName() : client->componentName();
            collectionsByClientName.insert(key, coll);
        }
    }
    const auto componentNames = collectionsByClientName.uniqueKeys();
    for (const QString &componentName : componentNames) {
        qCDebug(DEBUG_KXMLGUI) << "Considering component" << componentName;
        QDomDocument doc;
        QDomElement docElem = doc.createElement(QStringLiteral("gui"));
        docElem.setAttribute(QStringLiteral("version"), QStringLiteral("1"));
        docElem.setAttribute(QStringLiteral("name"), componentName);
        doc.appendChild(docElem);
        QDomElement elem = doc.createElement(QStringLiteral("ActionProperties"));
        docElem.appendChild(elem);

        const auto componentCollections = collectionsByClientName.values(componentName);
        for (KActionCollection *collection : componentCollections) {
            qCDebug(DEBUG_KXMLGUI) << "Saving shortcut scheme for action collection with" << collection->actions().count() << "actions";

            const auto collectionActions = collection->actions();
            for (QAction *action : collectionActions) {
                if (!action) {
                    continue;
                }

                const QString actionName = action->objectName();
                const QString shortcut = QKeySequence::listToString(action->shortcuts());
                // qCDebug(DEBUG_KXMLGUI) << "action" << actionName << "has shortcut" << shortcut;
                if (!shortcut.isEmpty()) {
                    QDomElement act_elem = doc.createElement(QStringLiteral("Action"));
                    act_elem.setAttribute(QStringLiteral("name"), actionName);
                    act_elem.setAttribute(QStringLiteral("shortcut"), shortcut);
                    elem.appendChild(act_elem);
                }
            }
        }

        const QString schemeFileName = writableShortcutSchemeFileName(componentName, schemeName);
        if (elem.childNodes().isEmpty()) {
            QFile::remove(schemeFileName);
        } else {
            qCDebug(DEBUG_KXMLGUI) << "saving to" << schemeFileName;
            QDir().mkpath(QFileInfo(schemeFileName).absolutePath());
            QFile schemeFile(schemeFileName);
            if (!schemeFile.open(QFile::WriteOnly | QFile::Truncate)) {
                qCDebug(DEBUG_KXMLGUI) << "COULD NOT WRITE" << schemeFileName;
                return false;
            }

            QTextStream out(&schemeFile);
            out << doc.toString(2);
        }
    }
    return true;
}

QString KShortcutSchemesHelper::currentShortcutSchemeName()
{
    return KSharedConfig::openConfig()->group("Shortcut Schemes").readEntry("Current Scheme", "Default");
}

QString KShortcutSchemesHelper::writableShortcutSchemeFileName(const QString &componentName, const QString &schemeName)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/%1/shortcuts/%2").arg(componentName, schemeName);
}

QString KShortcutSchemesHelper::writableApplicationShortcutSchemeFileName(const QString &schemeName)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QLatin1String("/%1/shortcuts/%2").arg(QCoreApplication::applicationName(), schemeName);
}

QString KShortcutSchemesHelper::shortcutSchemeFileName(const QString &componentName, const QString &schemeName)
{
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("%1/shortcuts/%2").arg(componentName, schemeName));
}

QString KShortcutSchemesHelper::applicationShortcutSchemeFileName(const QString &schemeName)
{
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("%1/shortcuts/%2").arg(QCoreApplication::applicationName(), schemeName));
}
