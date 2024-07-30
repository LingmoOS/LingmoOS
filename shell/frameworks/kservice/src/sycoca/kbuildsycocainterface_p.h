/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KBUILDSYCOCAINTERFACE_H
#define KBUILDSYCOCAINTERFACE_H

#include <kservice.h>

class KBuildSycocaInterface
{
public:
    virtual ~KBuildSycocaInterface();
    virtual KService::Ptr createService(const QString &path) = 0;
};

#endif /* KBUILDSYCOCAINTERFACE_H */
