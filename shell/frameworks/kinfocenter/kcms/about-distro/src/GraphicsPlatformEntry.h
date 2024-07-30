/*
    SPDX-FileCopyrightText: 2021 Méven Car <meven.car@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GRAPHICSPLATFORMENTRY_H
#define GRAPHICSPLATFORMENTRY_H

#include "Entry.h"

class GraphicsPlatformEntry : public Entry
{
public:
    GraphicsPlatformEntry();
    static QString graphicsPlatform();
};

#endif // GRAPHICSPLATFORMENTRY_H
