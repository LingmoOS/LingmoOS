/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHAREDCONFIG_H
#define KSHAREDCONFIG_H

#include <QExplicitlySharedDataPointer>
#include <kconfig.h>

/**
 * \class KSharedConfig ksharedconfig.h <KSharedConfig>
 *
 * KConfig variant using shared memory
 *
 * KSharedConfig provides a shared (reference counted) variant
 * of KConfig. This allows you to use/manipulate the same configuration
 * files from different places in your code without worrying about
 * accidentally overwriting changes.
 *
 * The openConfig() method is threadsafe: every thread gets a separate repository
 * of shared KConfig objects. This means, however, that you'll be responsible for
 * synchronizing the instances of KConfig for the same filename between threads,
 * using KConfig::reparseConfiguration() after a manual change notification, just like you have
 * to do between processes.
 */
class KCONFIGCORE_EXPORT KSharedConfig : public KConfig, public QSharedData // krazy:exclude=dpointer (only for refcounting)
{
public:
    typedef QExplicitlySharedDataPointer<KSharedConfig> Ptr;

public:
    /**
     * Creates a KSharedConfig object to manipulate a configuration file
     *
     * If an absolute path is specified for @p fileName, that file will be used
     * as the store for the configuration settings.  If a non-absolute path
     * is provided, the file will be looked for in the standard directory
     * specified by @p type.  If no path is provided, a default
     * configuration file will be used based on the name of the main
     * application component.
     *
     * @p mode determines whether the user or global settings will be allowed
     * to influence the values returned by this object.  See KConfig::OpenFlags for
     * more details.
     *
     * @param fileName     the configuration file to open. If empty, it will be determined
     *                     automatically (from --config on the command line, otherwise
     *                     from the application name + "rc")
     * @param mode         how global settings should affect the configuration
     *                     options exposed by this KConfig object
     * @param type         The standard directory to look for the configuration
     *                     file in (see QStandardPaths)
     *
     * @sa KConfig
     */
    static KSharedConfig::Ptr
    openConfig(const QString &fileName = QString(), OpenFlags mode = FullConfig, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    /**
     * Creates a KSharedConfig object to manipulate a configuration file suitable
     * for storing state information. Use this for storing information that is
     * changing frequently and should not be saved by configuration backup
     * utilities.
     *
     * If an absolute path is specified for @p fileName, that file will be used
     * as the store for the configuration settings. If a non-absolute path
     * is provided, the file will be looked for in the standard data directory
     * (QStandardPaths::AppDataLocation). If no path is provided, a default
     * configuration file will be used based on the name of the main
     * application component.
     *
     * @param fileName the configuration file to open. If empty, it will be determined
     *                 automatically from the application name + "staterc"
     *
     * @since 5.67
     *
     * @sa KConfig
     */
    static KSharedConfig::Ptr openStateConfig(const QString &fileName = QString());

    ~KSharedConfig() override;

private:
    Q_DISABLE_COPY(KSharedConfig)
    KConfigGroup groupImpl(const QString &groupName) override;
    const KConfigGroup groupImpl(const QString &groupName) const override;

    KCONFIGCORE_NO_EXPORT KSharedConfig(const QString &file, OpenFlags mode, QStandardPaths::StandardLocation resourceType);
};

typedef KSharedConfig::Ptr KSharedConfigPtr;

#endif // multiple inclusion guard
