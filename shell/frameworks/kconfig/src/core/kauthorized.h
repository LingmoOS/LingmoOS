/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 1998, 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KAUTHORIZED_H
#define KAUTHORIZED_H

#include <kconfigcore_export.h>

#include <QMetaEnum>
#include <QObject>
#include <QStringList>
#include <QVariant>

class QUrl;
class QString;
class QQmlEngine;
class QJSEngine;

/**
 * The functions in this namespace provide the core of the Kiosk action
 * restriction system; the KIO and KXMLGui frameworks build on this.
 *
 * The relevant settings are read from the application's KSharedConfig
 * instance, so actions can be disabled on a per-application or global
 * basis (by using the kdeglobals file).
 */
class KCONFIGCORE_EXPORT KAuthorized : public QObject
{
    Q_OBJECT
public:
    /**
     * The enum values lower cased represent the action that is authorized
     * For example the SHELL_ACCESS value is converted to the "shell_access" string.
     *
     * @since 5.88
     */
    enum GenericRestriction {
        SHELL_ACCESS = 1, // if the user is authorized to open a shell or execute shell commands
        GHNS, /// if the collaborative data sharing framework KNewStuff is authorized
        // GUI behavior
        LINEEDIT_REVEAL_PASSWORD, /// if typed characters in password fields can be made visible
        LINEEDIT_TEXT_COMPLETION, /// if line edits should be allowed to display completions
        MOVABLE_TOOLBARS, /// if toolbars of apps should be movable
        RUN_DESKTOP_FILES, /// if .desktop files should be run as executables when clicked
    };
    Q_ENUM(GenericRestriction)

    /**
     *
     * @since 5.88
     */
    enum GenericAction {
        OPEN_WITH = 1, /// if the open-with menu should be shown for files etc.
        EDITFILETYPE, /// if mime-type accociations are allowed to be configured

        OPTIONS_SHOW_TOOLBAR, /// if the toolbar should be displayed in apps
        SWITCH_APPLICATION_LANGUAGE, /// if an action to switch the app language should be shown
        BOOKMARKS, /// saving bookmarks is allowed
    };
    Q_ENUM(GenericAction)

    /**
     * Returns whether the user is permitted to perform a certain action.
     *
     * All settings are read from the "[KDE Action Restrictions]" group.
     * For example, if kdeglobals contains
     * @verbatim
       [KDE Action Restrictions][$i]
       shell_access=false
       @endverbatim
     * then
     * @code
     * KAuthorized::authorize("shell_access");
     * @endcode
     * will return @c false.
     *
     * This method is intended for actions that do not necessarily have a
     * one-to-one correspondence with a menu or toolbar item (ie: a QAction
     * in a KXMLGui application).  "shell_access" is an example of such a
     * "generic" action.
     *
     * The convention for actions like "File->New" is to prepend the action
     * name with "action/", for example "action/file_new".  This is what
     * authorizeAction() does.
     *
     * @param action  The name of the action.
     * @return        @c true if the action is authorized, @c false
     *                otherwise.
     *
     * @see authorizeAction()
     */
    Q_INVOKABLE static bool authorize(const QString &action);

    /**
     * Returns whether the user is permitted to perform a common action.
     * The enum values lower cased represent the action that is
     * passed in to @p authorize(QString)
     *
     * @overload
     * @since 5.88
     */
    Q_INVOKABLE static bool authorize(GenericRestriction action);

    /**
     * Returns whether the user is permitted to perform a certain action.
     *
     * This behaves like authorize(), except that "action/" is prepended to
     * @p action.  So if kdeglobals contains
     * @verbatim
       [KDE Action Restrictions][$i]
       action/file_new=false
       @endverbatim
     * then
     * @code
     * KAuthorized::authorizeAction("file_new");
     * @endcode
     * will return @c false.
     *
     * KXMLGui-based applications should not normally need to call this
     * function, as KActionCollection will do it automatically.
     *
     * @param action  The name of a QAction action.
     * @return        @c true if the QAction is authorized, @c false
     *                otherwise.
     * @since 5.24
     *
     * @see authorize()
     */
    Q_INVOKABLE static bool authorizeAction(const QString &action);

    /**
     * Overload to authorize common actions.
     *
     * @overload
     * @since 5.88
     */
    Q_INVOKABLE static bool authorizeAction(GenericAction action);

    /**
     * Returns whether the user is permitted to use a certain control module.
     *
     * All settings are read from the "[KDE Control Module Restrictions]"
     * group.  For example, if kdeglobals contains
     * @verbatim
       [KDE Control Module Restrictions][$i]
       kcm_desktop-settings=false
       @endverbatim
     * then
     * @code
     * KAuthorized::authorizeControlModule("kcm_desktop-settings");
     * @endcode
     * will return @c false.
     *
     * @param pluginId  The desktop menu ID for the control module.
     * @return @c true if access to the module is authorized, @c false otherwise.
     *
     */
    Q_INVOKABLE static bool authorizeControlModule(const QString &pluginId);

    static KAuthorized *create(QQmlEngine *, QJSEngine *)
    {
        return new KAuthorized;
    }

private:
    friend class KConfigQmlPlugin;
    explicit KAuthorized();
};

#endif
