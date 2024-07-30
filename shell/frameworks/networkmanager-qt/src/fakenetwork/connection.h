/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_SETTINGS_CONNECTION_H
#define NETWORKMANAGERQT_FAKE_NETWORK_SETTINGS_CONNECTION_H

#include <QObject>

#include <QDBusObjectPath>

#include "../generictypes.h"

class Connection : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.Settings.Connection")
public:
    explicit Connection(QObject *parent = nullptr, const NMVariantMapMap &settings = NMVariantMapMap());
    ~Connection() override;

    Q_PROPERTY(bool Unsaved READ unsaved)

    bool unsaved() const;

    /* Not part of DBus interface */
    QString connectionPath() const;
    void setConnectionPath(const QString &path);

public Q_SLOTS: // METHODS
    Q_SCRIPTABLE void Delete();
    Q_SCRIPTABLE NMVariantMapMap GetSecrets(const QString &setting_name);
    Q_SCRIPTABLE NMVariantMapMap GetSettings();
    Q_SCRIPTABLE void Save();
    Q_SCRIPTABLE void Update(const NMVariantMapMap &properties);
    Q_SCRIPTABLE void UpdateUnsaved(const NMVariantMapMap &properties);

Q_SIGNALS:
    void connectionRemoved(const QDBusObjectPath &path);
    Q_SCRIPTABLE void Removed();
    Q_SCRIPTABLE void Updated();
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

private:
    bool m_unsaved;
    NMVariantMapMap m_settings;

    /* Not part of DBus interface */
    QString m_connectionPath;
};

#endif
