/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef MIMETYPEDATA_H
#define MIMETYPEDATA_H

#include <QMimeType>

class KConfigGroup;

/**
 * This is a non-gui (data) class, that represents a mimetype.
 * It is a QMimeType plus the changes we made to it.
 */
class MimeTypeData
{
public:
    // Constructor used for groups
    explicit MimeTypeData(const QString &major);
    // Real constructor, used for an existing mimetype.
    explicit MimeTypeData(const QMimeType &mime);
    // Real constructor, used for a new mimetype.
    explicit MimeTypeData(const QString &mimeName, bool /*unused, just to distinguish from the other QString ctor*/);

    QString name() const
    {
        return m_isGroup ? m_major : m_major + QLatin1Char('/') + m_minor;
    }

    QString majorType() const
    {
        return m_major;
    }

    QString minorType() const
    {
        return m_minor;
    }

    void setMinor(const QString &m)
    {
        m_minor = m;
    }

    QString comment() const
    {
        return m_comment;
    }

    void setComment(const QString &c)
    {
        m_comment = c;
    }

    /**
     * Returns true if "this" is a group
     */
    bool isMeta() const
    {
        return m_isGroup;
    }

    /**
     * Returns true if the type is essential, i.e. can't be deleted
     * (see KMimeType::checkEssentialMimeTypes)
     */
    bool isEssential() const;
    QString icon() const;
    void setUserSpecifiedIcon(const QString &icon);
    QStringList patterns() const
    {
        return m_patterns;
    }

    void setPatterns(const QStringList &p);
    QStringList appServices() const;
    void setAppServices(const QStringList &dsl);
    QStringList embedParts() const;
    void setEmbedParts(const QStringList &dsl);

    enum AutoEmbed {
        Yes = 0,
        No = 1,
        UseGroupSetting = 2,
    };
    AutoEmbed autoEmbed() const
    {
        return m_autoEmbed;
    }

    void setAutoEmbed(AutoEmbed a)
    {
        m_autoEmbed = a;
    }

    const QMimeType &mimeType() const
    {
        return m_mimetype;
    }

    bool canUseGroupSetting() const;

    void getAskSave(bool &);
    void setAskSave(bool);

    /**
     * Returns true if the mimetype data has any unsaved changes.
     */
    bool isDirty() const;

    /**
     * Returns true if the mimetype data has any unsaved changes in the service list.
     */
    bool isServiceListDirty() const;

    /**
     * Save changes to disk.
     * Does not check isDirty(), so the common idiom is if (data.isDirty()) { needUpdate = data.sync(); }
     * Returns true if update-mime-database needs to be run afterwards
     */
    bool sync();
    /**
     * Update m_mimetype from the xml when Apply is pressed
     */
    void refresh();

    /**
     * Return true if this is a new mimetype, i.e. one that is not yet on disk
     */
    bool isNew() const
    {
        return m_bNewItem;
    }

    /**
     * Helper method for the filtering in the listview
     */
    bool matchesFilter(const QString &filter) const;

private:
    void initFromQMimeType();
    AutoEmbed readAutoEmbed() const;
    void writeAutoEmbed();
    bool isMimeTypeDirty() const; // whether the mimetype definition file needs saving
    QStringList getAppOffers() const;
    QStringList getPartOffers() const;
    void getMyServiceOffers() const;
    void syncServices();
    void saveServices(KConfigGroup &config, const QStringList &services);
    void saveDefaultApplication(KConfigGroup &config, const QStringList &services);
    void saveRemovedServices(KConfigGroup &config, const QStringList &services, const QStringList &oldServices);

    QMimeType m_mimetype;
    enum AskSave {
        AskSaveYes = 0,
        AskSaveNo = 1,
        AskSaveDefault = 2,
    };
    AskSave m_askSave : 3;
    AutoEmbed m_autoEmbed : 3;
    bool m_bNewItem : 1;
    mutable bool m_bFullInit : 1; // lazy init of m_appServices and m_embedServices
    bool m_isGroup : 1;
    bool m_appServicesModified : 1;
    bool m_embedServicesModified : 1;
    bool m_userSpecifiedIconModified : 1;
    QString m_major, m_minor, m_comment, m_userSpecifiedIcon;
    QStringList m_patterns;
    mutable QStringList m_appServices;
    mutable QStringList m_embedParts;
};

#endif /* MIMETYPEDATA_H */
