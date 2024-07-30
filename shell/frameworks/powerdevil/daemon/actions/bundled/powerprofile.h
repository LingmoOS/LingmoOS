/*
 * SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <powerdevilaction.h>

class OrgFreedesktopDBusPropertiesInterface;
class NetHadessPowerProfilesInterface;

namespace PowerDevil
{
namespace BundledActions
{
class PowerProfile : public PowerDevil::Action, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Solid.PowerManagement.Actions.PowerProfile")

    Q_PROPERTY(QStringList profileChoices READ profileChoices NOTIFY profileChoicesChanged)
    Q_PROPERTY(QString configuredProfile READ configuredProfile NOTIFY configuredProfileChanged)
    Q_PROPERTY(QString currentProfile READ currentProfile NOTIFY currentProfileChanged)

public:
    explicit PowerProfile(QObject *parent);
    ~PowerProfile() override;

    bool loadAction(const PowerDevil::ProfileSettings &profileSettings) override;

    QStringList profileChoices() const;
    QString configuredProfile() const;
    QString currentProfile() const;
    void setProfile(const QString &profile);
    QString performanceInhibitedReason() const;
    QString performanceDegradedReason() const;
    QList<QVariantMap> profileHolds() const;
    unsigned int holdProfile(const QString &profile, const QString &reason, const QString &applicationId);
    void releaseProfile(unsigned int cookie);

Q_SIGNALS:
    void currentProfileChanged(const QString &profile);
    void profileChoicesChanged(const QStringList &profiles);
    void performanceInhibitedReasonChanged(const QString &reason);
    void performanceDegradedReasonChanged(const QString &reason);
    void profileHoldsChanged(const QList<QVariantMap> &holds);
    void configuredProfileChanged(const QString &profile);

protected:
    void onProfileLoad(const QString &previousProfile, const QString &newProfile) override;
    void triggerImpl(const QVariantMap &args) override;
    bool isSupported() override;
private Q_SLOTS:
    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

private:
    void readProperties(const QVariantMap &properties);
    void serviceUnregistered(const QString &name);

    NetHadessPowerProfilesInterface *m_powerProfilesInterface;
    OrgFreedesktopDBusPropertiesInterface *m_powerProfilesPropertiesInterface;
    QStringList m_profileChoices;
    QString m_currentProfile;
    QString m_performanceInhibitedReason;
    QString m_degradationReason;
    QList<QVariantMap> m_profileHolds;
    QDBusServiceWatcher *m_holdWatcher;
    QMultiMap<QString, unsigned int> m_holdMap;

    QString m_configuredProfile;
};

} // namespace BundledActions
} // namespace PowerDevil
