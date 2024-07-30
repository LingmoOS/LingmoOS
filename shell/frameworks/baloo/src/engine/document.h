/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENT_H
#define BALOO_DOCUMENT_H

#include "engine_export.h"
#include <QByteArray>
#include <QDebug>
#include <QVector>

namespace Baloo {

class WriteTransaction;
class TermGeneratorTest;

/**
 * A document represents an indexed file to be stored in the Baloo engine.
 *
 * It is a large collection of words along with their respective positions.
 * One typically never needs to have all of this in memory except when creating the
 * Document for indexing.
 *
 * This is why Documents can be created and saved into the database, but not fetched.
 */
class BALOO_ENGINE_EXPORT Document
{
public:
    Document();

    void addTerm(const QByteArray& term);
    void addPositionTerm(const QByteArray& term, int position = 0);

    void addXattrTerm(const QByteArray& term);
    void addXattrPositionTerm(const QByteArray& term, int position = 0);

    void addFileNameTerm(const QByteArray& term);
    void addFileNamePositionTerm(const QByteArray& term, int position = 0);

    quint64 id() const;
    void setId(quint64 id);
    quint64 parentId() const;
    void setParentId(quint64 parentId);

    QByteArray url() const;
    void setUrl(const QByteArray& url);

    /**
     * This flag is used to signify if the file needs its contents to be indexed.
     * It defaults to false
     */
    void setContentIndexing(bool val);
    bool contentIndexing() const;

    void setMTime(quint32 val) { m_mTime = val; }
    void setCTime(quint32 val) { m_cTime = val; }

    void setData(const QByteArray& data);

private:
    quint64 m_id = 0;
    quint64 m_parentId = 0;

    struct TermData {
        QVector<uint> positions;
    };
    QMap<QByteArray, TermData> m_terms;
    QMap<QByteArray, TermData> m_xattrTerms;
    QMap<QByteArray, TermData> m_fileNameTerms;

    QByteArray m_url;
    bool m_contentIndexing = false;

    quint32 m_mTime = 0; //< modification time, seconds since Epoch
    quint32 m_cTime = 0; //< inode change time, seconds since Epoch
    QByteArray m_data;

    friend class WriteTransaction;
    friend class TermGeneratorTest;
    friend class BasicIndexingJobTest;
};

inline QDebug operator<<(QDebug dbg, const Document &doc) {
    dbg << doc.id() << doc.url();
    return dbg;
}

}

#endif // BALOO_DOCUMENT_H
