/*******************************************************************
 * systeminformation.h
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#ifndef SYSTEMINFORMATION__H
#define SYSTEMINFORMATION__H

#include <QObject>

class SystemInformation : public QObject
{
    Q_OBJECT
public:
    struct Config {
        Config();

        // Overrides m_operatingSystem value
        QString basicOperatingSystem;
        // Path to lsb_release executable
        QString lsbReleasePath;
        // Path to os-release file
        QString osReleasePath;
        // Function pointer to uname override
        void *unameFunc = nullptr;
    };

    explicit SystemInformation(Config infoConfig = Config(), QObject *parent = nullptr);
    ~SystemInformation() override;

    // TODO why is this on here rather than the reportinterface. same for compiled sources. the heck
    Q_PROPERTY(QString bugzillaPlatform READ bugzillaPlatform WRITE setBugzillaPlatform)
    QString bugzillaPlatform() const;
    void setBugzillaPlatform(const QString &);
    Q_SIGNAL void bugzillaPlatformChanged();

    QString operatingSystem() const;
    QString bugzillaOperatingSystem() const;

    QString distributionPrettyName() const;

    Q_PROPERTY(bool compiledSources READ compiledSources WRITE setCompiledSources)
    bool compiledSources() const;
    void setCompiledSources(bool);
    Q_SIGNAL void compiledSourcesChanged();

    QString qtVersion() const;
    QString frameworksVersion() const;
    QString windowSystem() const;

    /// All helpers finished and the data is complete
    bool complete() const;

private Q_SLOTS:
    void lsbReleaseFinished();

private:
    QString fetchOSDetailInformation() const;
    QString fetchOSReleaseInformation();

    QString guessBugzillaPlatform(const QString &) const;

    void tryToSetBugzillaPlatform();
    void tryToSetBugzillaPlatformFromExternalInfo();

    QString m_operatingSystem;
    QString m_bugzillaOperatingSystem;
    QString m_bugzillaPlatform;

    QString m_distributionPrettyName;

    bool m_compiledSources;

    bool m_complete; // all available data retrieved

    Config m_infoConfig;
};

#endif
