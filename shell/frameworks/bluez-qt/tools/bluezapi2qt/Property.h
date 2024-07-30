/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZAPI2QT_PROPERTY_H
#define BLUEZAPI2QT_PROPERTY_H

#include "Comment.h"

class Property
{
public:
    struct Tags {
        bool isOptional = false;
        bool isExperimental = false;
        bool isReadOnly = false;
        bool isServerOnly = false;
    };

    Property();

    bool finalize();

    QString name() const;
    QString type() const;
    Tags tags() const;
    QStringList comment() const;

private:
    QString m_name;
    QString m_type;
    QStringList m_stringTags;
    QString m_limitation;
    Comment m_comment;

    // finalized members
    Tags m_tags;

    friend class Properties;
};

#endif
