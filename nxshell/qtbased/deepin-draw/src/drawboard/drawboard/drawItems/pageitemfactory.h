// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEITEMFACTORY_H
#define PAGEITEMFACTORY_H
#include "globaldefine.h"
#include "sitemdata.h"

class PageItem;
class DRAWLIB_EXPORT PageItemFactory
{
public:
    PageItemFactory();
    static PageItem *creatItem(int itemType, const Unit &data = Unit());
    static void registerItem(const QString &classname, int classType);
};

#endif // PAGEITEMFACTORY_H
