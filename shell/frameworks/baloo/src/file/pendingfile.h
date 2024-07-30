/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_PENDINGFILE_H
#define BALOO_PENDINGFILE_H

#include <QString>
#include <QObject>

namespace Baloo {

/**
 * Represents a file which needs to be indexed.
 */
class PendingFile
{
public:
    explicit PendingFile(const QString& path);

    QString path() const;

    void setAttributeChanged() { m_attributesChanged = true; }
    void setClosedOnWrite() { m_closedOnWrite = true; }
    void setModified() { m_modified = true; }
    void setCreated() { m_created = true; }
    void setDeleted() { m_deleted = true; }

    bool isNewFile() const;
    bool shouldIndexContents() const;
    bool shouldIndexXAttrOnly() const;
    bool shouldRemoveIndex() const;

    bool operator == (const PendingFile& rhs) const {
        return m_path == rhs.m_path;
    }

    /**
     * Takes a PendingFile \p file and merges its flags into
     * the current PendingFile
     */
    void merge(const PendingFile& file);

private:
    QString m_path;

    bool m_created : 1;
    bool m_closedOnWrite : 1;
    bool m_attributesChanged : 1;
    bool m_deleted : 1;
    bool m_modified : 1;

    void printFlags() const;
};

}

Q_DECLARE_TYPEINFO(Baloo::PendingFile, Q_RELOCATABLE_TYPE);

#endif // BALOO_PENDINGFILE_H
