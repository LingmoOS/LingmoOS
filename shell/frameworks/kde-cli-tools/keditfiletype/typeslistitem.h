/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef TYPESLISTITEM_H
#define TYPESLISTITEM_H

#include "mimetypedata.h"
#include "typeslistitembase.h"

#include <QMimeType>

// TODO different subclasses for mimetypes and groups?
class TypesListItem : public TypesListItemBase
{
public:
    /**
     * Create a filetype group
     */
    TypesListItem(QTreeWidget *parent, const QString &major);

    /**
     * Create a filetype item inside a group, for an existing mimetype
     */
    TypesListItem(TypesListItem *parent, QMimeType mimetype);

    /**
     * Create a filetype item inside a group, for a new mimetype
     */
    TypesListItem(TypesListItem *parent, const QString &newMimetype);

    ~TypesListItem() override;

    void setIcon(const QString &icon);

    QString name() const
    {
        return m_mimetypeData.name();
    }

    const MimeTypeData &mimeTypeData() const
    {
        return m_mimetypeData;
    }

    MimeTypeData &mimeTypeData()
    {
        return m_mimetypeData;
    }

    virtual void loadIcon(bool forceReload = false) override;

private:
    MimeTypeData m_mimetypeData;
};

#endif
