/*
    SPDX-FileCopyrightText: 2000 Alex Zepeda <zipzippy@sonic.net>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef _KEMAILSETTINGS_H
#define _KEMAILSETTINGS_H

#include <QCoreApplication> // Q_DECLARE_TR_FUNCTIONS
#include <QStringList>

#include <kconfigcore_export.h>

class KEMailSettingsPrivate;

/**
 * \class KEMailSettings kemailsettings.h <KEMailSettings>
 *
 * This is just a small class to facilitate accessing e-mail settings in
 * a sane way, and allowing any program to manage multiple e-mail
 * profiles effortlessly
 *
 * The default profile is automatically selected in the constructor.
 *
 * @author Alex Zepeda zipzippy@sonic.net
 **/
class KCONFIGCORE_EXPORT KEMailSettings
{
    Q_DECLARE_TR_FUNCTIONS(KEMailSettings)
public:
    /**
     * The list of settings that I thought of when I wrote this
     * class.  Any extra settings thought of later can be accessed
     * easily with getExtendedSetting and setExtendedSetting.
     * @see getSetting()
     * @see setSetting()
     * @see getExtendedSetting()
     * @see setExtendedSetting()
     **/
    enum Setting {
        ClientProgram,
        ClientTerminal,
        RealName,
        EmailAddress,
        ReplyToAddress,
        Organization,
        OutServer,
        OutServerLogin,
        OutServerPass,
#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(5, 0)
        /**
         * @deprecated since 5.0
         */
        OutServerType KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 82, 5, 0, "No known users"),
        /**
         * @deprecated since 5.0
         */
        OutServerCommand KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 82, 5, 0, "No known users"),
        /**
         * @deprecated since 5.0
         */
        OutServerTLS KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 82, 5, 0, "No known users"),
#else
        OutServerType_DEPRECATED_DO_NOT_USE,
        OutServerCommand_DEPRECATED_DO_NOT_USE,
        OutServerTLS_DEPRECATED_DO_NOT_USE,
#endif
        InServer,
        InServerLogin,
        InServerPass,
#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(5, 0)
        /**
         * @deprecated since 5.0
         */
        InServerType KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 82, 5, 0, "No known users"),
        /**
         * @deprecated since 5.0
         */
        InServerMBXType KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 82, 5, 0, "No known users"),
        /**
         * @deprecated since 5.0
         */
        InServerTLS KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 82, 5, 0, "No known users")
#endif
    };

    /**
     * The various extensions allowed.
     **/
    enum Extension {
        POP3,
        SMTP,
        OTHER,
    };

    /**
     * Default constructor, just sets things up and sets the default profile
     * as the current profile
     **/
    KEMailSettings();

    KEMailSettings(const KEMailSettings &) = delete;
    KEMailSettings &operator=(const KEMailSettings &) = delete;

    /**
     * Default destructor, nothing to see here.
     **/
    ~KEMailSettings();

    /**
     * List of profiles available.
     * @return the list of profiles
     **/
    QStringList profiles() const;

#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Returns the name of the current profile.
     * @returns what profile we're currently using
     * @deprecated Since 5.0
     **/
    KCONFIGCORE_DEPRECATED_VERSION(5, 0, "API planned to be changed")
    QString currentProfileName() const;
    // see https://git.reviewboard.kde.org/r/111910/
#endif

    /**
     * Change the current profile.
     * @param s the name of the new profile
     **/
    void setProfile(const QString &s);

    /**
     * Returns the name of the default profile.
     * @returns the name of the one that's currently default QString() if none
     **/
    QString defaultProfileName() const;

    /**
     * Sets a new default.
     * @param def the new default
     **/
    void setDefault(const QString &def);

    /**
     * Get one of the predefined "basic" settings.
     * @param s the setting to get
     * @return the value of the setting, or QString() if not
     *         set
     **/
    QString getSetting(KEMailSettings::Setting s) const;

    /**
     * Set one of the predefined "basic" settings.
     * @param s the setting to set
     * @param v the new value of the setting, or QString() to
     *         unset
     **/
    void setSetting(KEMailSettings::Setting s, const QString &v);

private:
    KEMailSettingsPrivate *const p;
};

#endif
