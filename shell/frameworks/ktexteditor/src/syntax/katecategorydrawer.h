/*
    SPDX-FileCopyrightText: 2009 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_CATEGORYDRAWER_H
#define KATE_CATEGORYDRAWER_H

#include <KCategoryDrawer>

class QPainter;
class QModelIndex;
class QStyleOption;

class KateCategoryDrawer : public KCategoryDrawer
{
public:
    KateCategoryDrawer();

    void drawCategory(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter) const override;

    int categoryHeight(const QModelIndex &index, const QStyleOption &option) const override;

    int leftMargin() const override;

    int rightMargin() const override;
};

#endif
