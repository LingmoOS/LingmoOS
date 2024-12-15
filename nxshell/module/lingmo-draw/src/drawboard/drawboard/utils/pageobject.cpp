// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pageobject.h"
#include "pageitem.h"
#include "pagescene.h"

PageVariant::PageVariant(const QVariant &var): QVariant(var)
{

}

PageVariant::PageVariant(PageObject *p): QVariant(fromValue<PageObject*>(p))
{

}

PageVariant::PageVariant(const Unit &unit): QVariant(fromValue<Unit>(unit))
{

}

PageObject *PageVariant::toPageObjectPoint() const
{
    return value<PageObject *>();
}

PageScene *PageVariant::toPageScene() const
{
    if (canConvert<PageObject *>()) {
        return dynamic_cast<PageScene *>(toPageObjectPoint());
    }
    return nullptr;
}

PageItem *PageVariant::toPageItem() const
{
    if (canConvert<PageObject *>()) {
        return dynamic_cast<PageItem *>(toPageObjectPoint());
    }
    return nullptr;
}

Unit PageVariant::toUnit() const
{
    if (canConvert<Unit>()) {
        return value<Unit>();
    }
    return Unit();
}

PageVariantList::PageVariantList()
{

}

PageVariantList::PageVariantList(const PageVariant &var)
{
    append(var);
}

PageVariantList::PageVariantList(const QList<PageVariant> &list): QList<PageVariant>(list)
{

}

PageVariantList::PageVariantList(const QList<QVariant> &list)
{
    foreach (auto va, list) {
        push_back(va);
    }
}

PageVariantList::PageVariantList(const QList<PageObject *> &list)
{
    foreach (auto it, list) {
        append(it);
    }
}

PageVariantList::PageVariantList(const QList<Unit> &list)
{
    foreach (auto it, list) {
        append(it);
    }
}

//PageVariantList::PageVariantList(const PageVariant &var1, const PageVariant &var2,
//                                 const PageVariant &var3, const PageVariant &var4,
//                                 const PageVariant &var5): QList<PageVariant>()
//{
//    if (var1.isValid())
//        QList<PageVariant>::push_back(var1);
//    if (var2.isValid())
//        QList<PageVariant>::push_back(var2);
//    if (var3.isValid())
//        QList<PageVariant>::push_back(var3);
//    if (var4.isValid())
//        QList<PageVariant>::push_back(var4);
//    if (var5.isValid())
//        QList<PageVariant>::push_back(var5);

//}

PageVariantList::PageVariantList(const PageVariantList &var1, const PageVariantList &var2,
                                 const PageVariantList &var3, const PageVariantList &var4,
                                 const PageVariantList &var5)
{
    foreach (auto it, var1) {
        append(it);
    }
    foreach (auto it, var2) {
        append(it);
    }
    foreach (auto it, var3) {
        append(it);
    }
    foreach (auto it, var4) {
        append(it);
    }
    foreach (auto it, var5) {
        append(it);
    }
}

