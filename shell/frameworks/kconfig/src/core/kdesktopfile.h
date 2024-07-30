/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Pietro Iglio <iglio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KDESKTOPFILE_H
#define KDESKTOPFILE_H

#include <kconfig.h>

class KConfigGroup;
class KDesktopFileAction;
class KDesktopFilePrivate;

/**
 * \class KDesktopFile kdesktopfile.h <KDesktopFile>
 *
 * %KDE Desktop File Management.
 * This class implements %KDE's support for the freedesktop.org
 * <em>Desktop Entry Spec</em>.
 *
 * @author Pietro Iglio <iglio@kde.org>
 * @see  KConfigBase  KConfig
 * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html">Desktop Entry Spec</a>
 */
class KCONFIGCORE_EXPORT KDesktopFile : public KConfig
{
public:
    /**
     * Constructs a KDesktopFile object.
     *
     * See QStandardPaths for more information on resources.
     *
     * @param resourceType   Allows you to change what sort of resource
     *                       to search for if @p fileName is not absolute.
     *                       For instance, you might want to specify GenericConfigLocation.
     * @param fileName       The name or path of the desktop file. If it
     *                       is not absolute, it will be located
     *                       using the resource type @p resType.
     */
    explicit KDesktopFile(QStandardPaths::StandardLocation resourceType, const QString &fileName);

    /**
     * Constructs a KDesktopFile object.
     *
     * See QStandardPaths for more information on resources.
     *
     * @param fileName       The name or path of the desktop file. If it
     *                       is not absolute, it will be located
     *                       using the resource type ApplicationsLocation
     */
    explicit KDesktopFile(const QString &fileName);

    /**
     * Destructs the KDesktopFile object.
     *
     * Writes back any changed configuration entries.
     */
    ~KDesktopFile() override;

    /**
     * Checks whether this is really a desktop file.
     *
     * The check is performed looking at the file extension (the file is not
     * opened).
     * Currently, the only valid extension is ".desktop".
     * @param path the path of the file to check
     * @return true if the file appears to be a desktop file.
     */
    static bool isDesktopFile(const QString &path);

    /**
     * Checks whether the user is authorized to run this desktop file.
     * By default users are authorized to run all desktop files but
     * the KIOSK framework can be used to activate certain restrictions.
     * See README.kiosk for more information.
     *
     * Note that desktop files that are not in a standard location (as
     * specified by XDG_DATA_DIRS) must have their executable bit set
     * to be authorized, regardless of KIOSK settings, to prevent users
     * from inadvertently running trojan desktop files.
     *
     * @param path the file to check
     * @return true if the user is authorized to run the file
     */
    static bool isAuthorizedDesktopFile(const QString &path);

    /**
     * Returns the location where changes for the .desktop file @p path
     * should be written to.
     */
    static QString locateLocal(const QString &path);

    /**
     * Returns the main config group (named "Desktop Entry") in a .desktop file.
     */
    KConfigGroup desktopGroup() const;

    /**
     * Returns the value of the "Type=" entry.
     * @return the type or QString() if not specified
     */
    QString readType() const;

    /**
     * Returns the value of the "Icon=" entry.
     * @return the icon or QString() if not specified
     */
    QString readIcon() const;

    /**
     * Returns the value of the "Name=" entry.
     * @return the name or QString() if not specified
     */
    QString readName() const;

    /**
     * Returns the value of the "Comment=" entry.
     * @return the comment or QString() if not specified
     */
    QString readComment() const;

    /**
     * Returns the value of the "GenericName=" entry.
     * @return the generic name or QString() if not specified
     */
    QString readGenericName() const;

    /**
     * Returns the value of the "Path=" entry.
     * @return the path or QString() if not specified
     */
    QString readPath() const;

    /**
     * Returns the value of the "URL=" entry.
     * @return the URL or QString() if not specified
     */
    QString readUrl() const;

    /**
     * Returns a list of the "Actions=" entries.
     * @return the list of actions
     */
    QStringList readActions() const;

    /**
     * Returns a list of the "MimeType=" entries.
     * @return the list of mime types
     * @since 5.15
     */
    QStringList readMimeTypes() const;

    /**
     * Sets the desktop action group.
     * @param group the new action group
     */
    KConfigGroup actionGroup(const QString &group);
    KConfigGroup actionGroup(const QString &group) const;

    /**
     * Returns true if the action group exists, false otherwise
     * @param group the action group to test
     * @return true if the action group exists
     */
    bool hasActionGroup(const QString &group) const;

    /**
     * Checks whether there is a "Type=Link" entry.
     *
     * The link points to the "URL=" entry.
     * @return true if there is a "Type=Link" entry
     */
    bool hasLinkType() const;

    /**
     * Checks whether there is an entry "Type=Application".
     * @return true if there is a "Type=Application" entry
     */
    bool hasApplicationType() const;

    /**
     * Checks whether there is an entry "Type=FSDevice".
     * @return true if there is a "Type=FSDevice" entry
     */
    bool hasDeviceType() const;

    /**
     * Checks whether the TryExec field contains a binary
     * which is found on the local system.
     * @return true if TryExec contains an existing binary
     */
    bool tryExec() const;

    /**
     * Returns the value of the "X-DocPath=" Or "DocPath=" entry.
     * @return The value of the "X-DocPath=" Or "DocPath=" entry.
     */
    QString readDocPath() const;

    /**
     * Whether the entry should be suppressed in menus.
     * This handles the NoDisplay key
     * @return true to suppress this desktop file
     * @since 4.1
     */
    bool noDisplay() const;

    /**
     * Copies all entries from this config object to a new
     * KDesktopFile object that will save itself to @p file.
     *
     * Actual saving to @p file happens when the returned object is
     * destructed or when sync() is called upon it.
     *
     * @param file the new KDesktopFile object it will save itself to.
     */
    KDesktopFile *copyTo(const QString &file) const;

    /**
     * Returns the name of the .desktop file that was used to construct this KDesktopFile.
     */
    QString fileName() const;

    /**
     *
     * @since 6.0
     */
    QList<KDesktopFileAction> actions() const;

private:
    Q_DISABLE_COPY(KDesktopFile)

    Q_DECLARE_PRIVATE(KDesktopFile)
};

#endif
