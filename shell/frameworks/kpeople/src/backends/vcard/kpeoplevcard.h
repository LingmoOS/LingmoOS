/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPEOPLEVCARD_H
#define KPEOPLEVCARD_H

#include <KDirWatch>
#include <KPeopleBackend/AbstractContact>
#include <KPeopleBackend/AbstractEditableContact>
#include <KPeopleBackend/AllContactsMonitor>
#include <KPeopleBackend/BasePersonsDataSource>

class QFileInfo;

class KPeopleVCard : public KPeople::AllContactsMonitor
{
    Q_OBJECT

public:
    KPeopleVCard();
    ~KPeopleVCard() override;

    QMap<QString, KPeople::AbstractContact::Ptr> contacts() override;
    static QString contactsVCardPath();
    static QString contactsVCardWritePath();

private:
    void processVCard(const QString &path);
    void deleteVCard(const QString &path);
    void processDirectory(const QFileInfo &fi);

    QMap<QString, KPeople::AbstractContact::Ptr> m_contactForUri;
    KDirWatch *m_fs;
};

class VCardDataSource : public KPeople::BasePersonsDataSourceV2
{
public:
    VCardDataSource(QObject *parent, const QVariantList &data);
    ~VCardDataSource() override;
    QString sourcePluginId() const override;

    KPeople::AllContactsMonitor *createAllContactsMonitor() override;
    bool addContact(const QVariantMap &properties) override;
    bool deleteContact(const QString &uri) override;
};

#endif
