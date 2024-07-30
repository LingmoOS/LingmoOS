/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SDDMSETTINGSBASE_H
#define SDDMSETTINGSBASE_H

#include <KConfigSkeleton>

// The configuration of SDDM works as follows:
// - The system configuration comes from files in /usr/lib/sddm.conf.d/
// - User supplied config files are in /etc/sddm/conf.d/
// - And the most specific file is /etc/sdddm/conf
// Overwrite order is in the opposite order of mention.
// Because KConfig can't model this we use two KConfig objects, one to read the defaults from
// /usr/lib/sddm.conf.d/* supplied here, and another one passed via the constructor to the KConfigSkeleton
// to read the user configuration. The generated skeleton inherits this class and calls these methods
// to access the defaults
class SddmSettingsBase : public KConfigSkeleton
{
    Q_OBJECT
public:
    SddmSettingsBase(KSharedConfigPtr config, QObject *parent = nullptr);
    Q_PROPERTY(QString defaultUser READ defaultUser CONSTANT)
protected:
    QString defaultCurrent() const;
    unsigned int defaultMinimumUid() const;
    unsigned int defaultMaximumUid() const;
    QString defaultUser() const;
    QString defaultSession() const;
    bool defaultRelogin() const;
    QString defaultHaltCommand() const;
    QString defaultRebootCommand() const;

private:
    KSharedConfigPtr m_defaultConfig;
};

#endif
