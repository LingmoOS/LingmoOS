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
     * class.
     * @see getSetting()
     * @see setSetting()
     **/
    enum Setting {
        ClientProgram,
        ClientTerminal,
        RealName,
        EmailAddress,
        ReplyToAddress,
        Organization,
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
