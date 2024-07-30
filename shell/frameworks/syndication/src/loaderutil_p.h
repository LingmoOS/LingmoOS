/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOADERUTIL_H
#define LOADERUTIL_H
#include "syndication_private_export.h"
#include <QUrl>
namespace Syndication
{
namespace LoaderUtil
{
Q_REQUIRED_RESULT SYNDICATION_TESTS_EXPORT QUrl parseFeed(const QByteArray &data, const QUrl &url);
}
}

#endif // LOADERUTIL_H
