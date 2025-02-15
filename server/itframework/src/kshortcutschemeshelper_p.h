/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHORTCUTSCHEMESHELPER_P_H
#define KSHORTCUTSCHEMESHELPER_P_H

#include <QString>

class KActionCollection;
class KXMLGUIClient;

class KShortcutSchemesHelper
{
public:
    /**
     * Saves actions from these collections to the shortcut scheme file.
     *
     * This doesn't save anything for action collections without a parent xmlgui client.
     *
     *  @return true if the shortcut scheme was successfully saved.
     */
    static bool saveShortcutScheme(const QList<KActionCollection *> &collections, const QString &schemeName);

    /**
     * @return the current shortcut scheme name for the application.
     */
    static QString currentShortcutSchemeName();

    /**
     * @return the name of the (writable) file to save the shortcut scheme to.
     */
    static QString writableShortcutSchemeFileName(const QString &componentName, const QString &schemeName);

    /**
     * @return the name of the scheme file for application itself.
     */
    static QString writableApplicationShortcutSchemeFileName(const QString &schemeName);

    /**
     * @return the name of the file to read the shortcut scheme from.
     */
    static QString shortcutSchemeFileName(const QString &componentName, const QString &schemeName);

    /**
     * @return the name of the scheme file for application itself, for reading.
     */
    static QString applicationShortcutSchemeFileName(const QString &schemeName);
};

#endif
