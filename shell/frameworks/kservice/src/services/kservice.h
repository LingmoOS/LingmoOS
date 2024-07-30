/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 1999-2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICE_H
#define KSERVICE_H

#include "kserviceaction.h"
#include <QCoreApplication>
#include <QStringList>
#include <QVariant>
#include <kserviceconversioncheck_p.h>
#include <ksycocaentry.h>

#include <optional>

class QDataStream;
class KDesktopFile;
class QWidget;

class KServicePrivate;

/**
 * @class KService kservice.h <KService>
 *
 * Represents an installed application.
 *
 * To obtain a KService instance for a specific application you typically use serviceByDesktopName(), e.g.:
 *
 * @code
 * KService::Ptr service = KService::serviceByDesktopName("org.kde.kate");
 * @endcode
 *
 * Other typical usage would be in combination with KApplicationTrader to obtain e.g. the default application for a given file type.
 *
 * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/latest/">Desktop Entry Specification</a>
 */
class KSERVICE_EXPORT KService : public KSycocaEntry
{
public:
    /**
     * A shared data pointer for KService.
     */
    typedef QExplicitlySharedDataPointer<KService> Ptr;
    /**
     * A list of shared data pointers for KService.
     */
    typedef QList<Ptr> List;

    /**
     * Construct a temporary service with a given name, exec-line and icon.
     * @param name the name of the service
     * @param exec the executable
     * @param icon the name of the icon
     */
    KService(const QString &name, const QString &exec, const QString &icon);

    /**
     * Construct a service and take all information from a .desktop file.
     *
     * @param fullpath Full path to the .desktop file.
     */
    explicit KService(const QString &fullpath);

    /**
     * Construct a service and take all information from a desktop file.
     * @param config the desktop file to read
     * @param optional relative path to store for findByName
     */
    explicit KService(const KDesktopFile *config, const QString &entryPath = QString());

    KService(const KService &other);

    ~KService() override;

    /**
     * Whether this service is an application
     * @return true if this service is an application, i.e. it has Type=Application in its
     * .desktop file and exec() will not be empty.
     */
    bool isApplication() const;

    /**
     * Returns the executable.
     * @return the command that the service executes,
     *         or QString() if not set
     */
    QString exec() const;

    /**
     * Returns the name of the icon.
     * @return the icon associated with the service,
     *         or QString() if not set
     */
    QString icon() const;
    /**
     * Checks whether the application should be run in a terminal.
     *
     * This corresponds to `Terminal=true` in the .desktop file.
     *
     * @return @c true if the application should be run in a terminal.
     */
    bool terminal() const;

    /**
     * Returns any options associated with the terminal the application
     * runs in, if it requires a terminal.
     *
     * The application must be a TTY-oriented program.
     * @return the terminal options,
     *         or QString() if not set
     */
    QString terminalOptions() const;

    /**
     * Returns @c true if the application indicates that it's preferred to run
     * on a discrete graphics card, otherwise return @c false.
     *
     * In releases older than 5.86, this method checked for the @c X-KDE-RunOnDiscreteGpu
     * key in the .desktop file represented by this service; starting from 5.86 this method
     * now also checks for @c PrefersNonDefaultGPU key (added to the Freedesktop.org desktop
     * entry spec in version 1.4 of the spec).
     *
     * @since 5.30
     */
    bool runOnDiscreteGpu() const;

    /**
     * @brief Checks whether the application needs to run under a different UID.
     * @return @c true if the application needs to run under a different UID.
     * @see username()
     */
    bool substituteUid() const;
    /**
     * Returns the user name if the application runs with a
     * different user id.
     * @return the username under which the service has to be run,
     *         or QString() if not set
     * @see substituteUid()
     */
    QString username() const;

    /**
     * Returns the filename of the desktop entry without any
     * extension, e.g. "org.kde.kate"
     * @return the name of the desktop entry without path or extension,
     *         or QString() if not set
     */
    QString desktopEntryName() const;

    /**
     * Returns the menu ID of the application desktop entry.
     * The menu ID is used to add or remove the entry to a menu.
     * @return the menu ID
     */
    QString menuId() const;

    /**
     * Returns a normalized ID suitable for storing in configuration files.
     * It will be based on the menu-id when available and otherwise falls
     * back to entryPath()
     * @return the storage ID
     */
    QString storageId() const;

    /**
     * @return the working directory to run the program in,
     *         or QString() if not set
     * @since 5.63
     */
    QString workingDirectory() const;

    /**
     * Returns the descriptive comment for the application, if there is one.
     * @return the descriptive comment for the application, or QString()
     *         if not set
     */
    QString comment() const;

    /**
     * Returns the generic name for the application, if there is one
     * (e.g. "Mail Client").
     * @return the generic name,
     *         or QString() if not set
     */
    QString genericName() const;

    /**
     * Returns the untranslated (US English) generic name
     * for the application, if there is one
     * (e.g. "Mail Client").
     * @return the generic name,
     *         or QString() if not set
     */
    QString untranslatedGenericName() const;

    /**
     * @return untranslated name for the given service
     *
     * @since 6.0
     */
    QString untranslatedName() const;
    /**
     * Returns a list of descriptive keywords for the application, if there are any.
     * @return the list of keywords
     */
    QStringList keywords() const;

    /**
     * Returns a list of VFolder categories.
     * @return the list of VFolder categories
     */
    QStringList categories() const;

    /**
     * Returns the list of MIME types that this application supports.
     * Note that this doesn't include inherited MIME types,
     * only the MIME types listed in the .desktop file.
     * @since 4.8.3
     */
    QStringList mimeTypes() const;

    /**
     * Returns the list of scheme handlers this application supports.
     *
     * For example a web browser could return {"http", "https"}.
     *
     * This is taken from the x-scheme-handler MIME types
     * listed in the .desktop file.
     *
     * @since 6.0
     *
     */
    QStringList schemeHandlers() const;

    /**
     * Returns the list of protocols this application supports.
     *
     * This is taken from the x-scheme-handler MIME types
     * listed in the .desktop file as well as the 'X-KDE-Protocols'
     * entry
     *
     * For example a web browser could return {"http", "https"}.
     * @since 6.0
     */
    QStringList supportedProtocols() const;

    /**
     * Checks whether the application supports this MIME type
     * @param mimeType The name of the MIME type you are
     *        interested in determining whether this service supports.
     * @since 4.6
     */
    bool hasMimeType(const QString &mimeType) const;

    /**
     * Returns the actions defined in this desktop file
     */
    QList<KServiceAction> actions() const;

    /**
     * Checks whether this application can handle several files as
     * startup arguments.
     * @return true if multiple files may be passed to this service at
     * startup. False if only one file at a time may be passed.
     */
    bool allowMultipleFiles() const;

    /**
     * Whether the entry should be hidden from the menu.
     * @return @c true to hide this application from the menu
     *
     * Such services still appear in trader queries, i.e. in
     * "Open With" popup menus for instance.
     */
    bool noDisplay() const;

    /**
     * Whether the application should be shown in the current desktop
     * (including in context menus).
     * @return true if the application should be shown in the current desktop.
     *
     * KApplicationTrader honors this and removes such services
     * from its results.
     *
     * @since 5.0
     */
    bool showInCurrentDesktop() const;

    /**
     * Whether the application should be shown on the current
     * platform (e.g. on xcb or on wayland).
     * @return @c true if the application should be shown on the current platform.
     *
     * @since 5.0
     */
    bool showOnCurrentPlatform() const;

    /**
     * The path to the documentation for this application.
     * @since 4.2
     * @return the documentation path, or QString() if not set
     */
    QString docPath() const;

    /**
     * Returns the requested property.
     *
     * @tparam T the type of the requested property.
     *
     * @param name the name of the property.
     *
     * @since 6.0
     */
    template<typename T>
    T property(const QString &name) const
    {
        KServiceConversionCheck::to_QVariant<T>();
        return property(name, static_cast<QMetaType::Type>(qMetaTypeId<T>())).value<T>();
    }

    /**
     * Returns a path that can be used for saving changes to this
     * application
     * @return path that can be used for saving changes to this application
     */
    QString locateLocal() const;

    /**
     * @internal
     * Set the menu id
     */
    void setMenuId(const QString &menuId);
    /**
     * @internal
     * Sets whether to use a terminal or not
     */
    void setTerminal(bool b);
    /**
     * @internal
     * Sets the terminal options to use
     */
    void setTerminalOptions(const QString &options);

    /**
     * Overrides the "Exec=" line of the service.
     *
     * If @ref exec is not empty, its value will override the one
     * the one set when this application was created.
     *
     * Please note that @ref entryPath is also cleared so the application
     * will no longer be associated with a specific config file.
     *
     * @internal
     * @since 4.11
     */
    void setExec(const QString &exec);

    /**
     * Overrides the "Path=" line of the application.
     *
     * If @ref workingDir is not empty, its value will override
     * the one set when this application was created.
     *
     * Please note that @ref entryPath is also cleared so the application
     * will no longer be associated with a specific config file.
     *
     * @internal
     * @param workingDir
     * @since 5.79
     */
    void setWorkingDirectory(const QString &workingDir);

    /**
     * Find a application based on its path as returned by entryPath().
     * It's usually better to use serviceByStorageId() instead.
     *
     * @param _path the path of the configuration file
     * @return a pointer to the requested application or @c nullptr if the application is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByDesktopPath(const QString &_path);

    /**
     * Find an application by the name of its desktop file, not depending on
     * its actual location (as long as it's under the applications or application
     * directories). For instance "konqbrowser" or "kcookiejar". Note that
     * the ".desktop" extension is implicit.
     *
     * This is the recommended method (safe even if the user moves stuff)
     * but note that it assumes that no two entries have the same filename.
     *
     * @param _name the name of the configuration file
     * @return a pointer to the requested application or @c nullptr if the application is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByDesktopName(const QString &_name);

    /**
     * Find a application by its menu-id
     *
     * @param _menuId the menu id of the application
     * @return a pointer to the requested application or @c nullptr if the application is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByMenuId(const QString &_menuId);

    /**
     * Find a application by its storage-id or desktop-file path. This
     * function will try very hard to find a matching application.
     *
     * @param _storageId the storage id or desktop-file path of the application
     * @return a pointer to the requested application or @c nullptr if the application is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByStorageId(const QString &_storageId);

    /**
     * Returns the whole list of applications.
     *
     *  Useful for being able to
     * to display them in a list box, for example.
     * More memory consuming than the ones above, don't use unless
     * really necessary.
     * @return the list of all applications
     */
    static List allServices();

    /**
     * Returns a path that can be used to create a new KService based
     * on @p suggestedName.
     * @param showInMenu @c true, if the application should be shown in the KDE menu
     *        @c false, if the application should be hidden from the menu
     *        This argument isn't used anymore, use `NoDisplay=true` to hide the application.
     * @param suggestedName name to base the file on, if an application with such a
     *        name already exists, a suffix will be added to make it unique
     *        (e.g. foo.desktop, foo-1.desktop, foo-2.desktop).
     * @param menuId If provided, menuId will be set to the menu id to use for
     *        the KService
     * @param reservedMenuIds If provided, the path and menu id will be chosen
     *        in such a way that the new menu id does not conflict with any
     *        of the reservedMenuIds
     * @return The path to use for the new KService.
     */
    static QString newServicePath(bool showInMenu, const QString &suggestedName, QString *menuId = nullptr, const QStringList *reservedMenuIds = nullptr);

    /**
     * @brief A desktop file name that this application is an alias for.
     *
     * This is used when a `NoDisplay` application is used to enforce specific handling
     * for an application. In that case the `NoDisplay` application is an `AliasFor` another
     * application and be considered roughly equal to the `AliasFor` application (which should
     * not be `NoDisplay=true`)
     * For example Okular supplies a desktop file for each supported format (e.g. PDF), all
     * of which `NoDisplay` and it is merely there to selectively support specific file formats.
     * A UI may choose to display the aliased entry org.kde.okular instead of the NoDisplay entries.
     *
     * @since 5.96
     *
     * @return QString desktopName of the aliased application (excluding .desktop suffix)
     */
    QString aliasFor() const;

    /**
     * Returns the value of StartupNotify for this service.
     *
     * If the service doesn't define a value nullopt is returned.
     *
     * See StartupNotify in the <a href="https://specifications.freedesktop.org/desktop-entry-spec/latest/">Desktop Entry Specification</a>.
     *
     * @since 6.0
     */
    std::optional<bool> startupNotify() const;

private:
    friend class KBuildServiceFactory;

    QVariant property(const QString &_name, QMetaType::Type t) const;

    void setActions(const QList<KServiceAction> &actions);

    Q_DECLARE_PRIVATE(KService)

    friend class KServiceFactory;

    /**
     * @internal
     * Construct a service from a stream.
     * The stream must already be positioned at the correct offset.
     */
    KSERVICE_NO_EXPORT KService(QDataStream &str, int offset);
};

template<>
KSERVICE_EXPORT QString KService::property<QString>(const QString &name) const;

#endif
