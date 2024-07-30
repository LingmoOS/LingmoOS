/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PLATFORMDEPENDENT_V2_H
#define ATTICA_PLATFORMDEPENDENT_V2_H

#include <QList>
#include <QtPlugin>

#include "attica_export.h"
#include "platformdependent.h"

class QByteArray;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;
class QUrl;

namespace Attica
{
class ATTICA_EXPORT PlatformDependentV2 : public PlatformDependent
{
public:
    ~PlatformDependentV2() override;
    virtual QNetworkReply *deleteResource(const QNetworkRequest &request) = 0;
    virtual QNetworkReply *put(const QNetworkRequest &request, QIODevice *data) = 0;
    virtual QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data) = 0;
};

}

Q_DECLARE_INTERFACE(Attica::PlatformDependentV2, "org.kde.Attica.InternalsV2/1.2")

#endif
