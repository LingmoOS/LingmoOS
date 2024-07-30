/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef COMMENT_H
#define COMMENT_H

#include <QStringList>

class Comment : public QStringList
{
public:
    using QStringList::QStringList;

    bool finalize();
};

#endif // COMMENT_H
