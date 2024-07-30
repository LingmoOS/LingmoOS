/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Julius Künzel <jk.kdedev@smartlab.uber.space>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KABOUT_APPLICATION_COMPONENT_MODEL_H
#define KABOUT_APPLICATION_COMPONENT_MODEL_H

#include <KAboutData>

#include <QAbstractListModel>

namespace KDEPrivate
{
enum {
    LINK_HEIGHT = 32,
};
class KAboutApplicationComponentProfile;

class KAboutApplicationComponentModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit KAboutApplicationComponentModel(const QList<KAboutComponent> &personList, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 1;
    }
    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    const QList<KAboutComponent> m_componentList;
    QList<KAboutApplicationComponentProfile> m_profileList;
};

class KAboutApplicationComponentProfile
{
public:
    KAboutApplicationComponentProfile()
        : m_name()
        , m_description()
        , m_version()
        , m_webAdress()
        , m_license()
    {
    } // needed for QVariant

    KAboutApplicationComponentProfile(const QString &name,
                                      const QString &description,
                                      const QString &version,
                                      const QString &webAdress,
                                      const KAboutLicense &license)
        : m_name(name)
        , m_description(description)
        , m_version(version)
        , m_webAdress(webAdress)
        , m_license(license)
    {
    }

    const QString &name() const
    {
        return m_name;
    }
    const QString &description() const
    {
        return m_description;
    }
    const QString &version() const
    {
        return m_version;
    }
    const QString &webAdress() const
    {
        return m_webAdress;
    }
    const KAboutLicense &license() const
    {
        return m_license;
    }

private:
    QString m_name;
    QString m_description;
    QString m_version;
    QString m_webAdress;
    KAboutLicense m_license;
};

} // namespace KDEPrivate

Q_DECLARE_METATYPE(KDEPrivate::KAboutApplicationComponentProfile)
#endif // KABOUT_APPLICATION_COMPONENT_MODEL_H
