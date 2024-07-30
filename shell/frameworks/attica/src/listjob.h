/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LISTJOB_H
#define LISTJOB_H

#include "attica_export.h"
#include "getjob.h"

class QNetworkRequest;

namespace Attica
{
class Provider;

/**
 * @class ListJob listjob.h <Attica/ListJob>
 *
 * Represents a list job.
 */
template<class T>
class ATTICA_EXPORT ListJob : public GetJob
{
public:
    typename T::List itemList() const;

protected:
    void parse(const QString &xml) override;

private:
    ListJob(PlatformDependent *internals, const QNetworkRequest &request);
    typename T::List m_itemList;
    friend class Attica::Provider;
};

}

#endif
