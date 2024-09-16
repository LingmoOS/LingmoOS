// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEOBJECT_H
#define PAGEOBJECT_H
#include <sitemdata.h>

class PageObject;
class PageItem;
class PageScene;
class PageVariant: public QVariant
{
public:
    using QVariant::QVariant;
    PageVariant(const QVariant &var);
    PageVariant(PageObject *p);
    PageVariant(const Unit &unit);

    PageObject *toPageObjectPoint()const;
    PageScene *toPageScene()const;
    PageItem *toPageItem()const;
    Unit toUnit()const;
};

class PageVariantList: public QList<PageVariant>
{
public:
    PageVariantList();
    PageVariantList(const PageVariant &var);
    PageVariantList(const QList<PageVariant> &list);
    PageVariantList(const QList<QVariant> &list);
    PageVariantList(const QList<PageObject *> &list);
    PageVariantList(const QList<Unit> &list);

    PageVariantList(const PageVariantList &var1,
                    const PageVariantList &var2,
                    const PageVariantList &var3 = PageVariantList(),
                    const PageVariantList &var4 = PageVariantList(),
                    const PageVariantList &var5 = PageVariantList());

    inline PageVariantList &operator<< (const PageVariant &t)
    { append(t); return *this; }

    inline PageVariantList &operator<< (const QList<PageObject *> &t)
    { foreach(auto p, t) {append(p);} return *this; }

    template<class T = PageItem>
    inline PageVariantList & operator<< (const QList<T *> &t)
    { foreach(auto p, t) {append(p);} return *this; }
};

class PageObject
{
public:
    virtual void loadData(const PageVariant &ut) = 0;
    virtual PageVariant getData(int use)const = 0;
};
Q_DECLARE_METATYPE(PageObject *)

#endif // PAGEOBJECT_H
