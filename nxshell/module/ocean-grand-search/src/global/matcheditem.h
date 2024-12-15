// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATCHEDITEM_H
#define MATCHEDITEM_H

#include <QtCore>

namespace GrandSearch {

//搜索结果
struct MatchedItem {
    QString item;   //匹配的内容
    QString name;   //界面显示的名称
    QString icon;   //界面显示的图标
    QString type;   //界面显示的类型
    QString searcher;   //出自的搜索项
    QVariant extra;    //扩展信息，由各搜索结果补充的特殊属性QVariantHash

    bool operator == (const MatchedItem &other) {
        return item == other.item && name == other.name
                && icon == other.icon && type == other.type
                && searcher == other.searcher && extra == other.extra;
    }
};

typedef QList<MatchedItem> MatchedItems;
typedef QMap<QString, MatchedItems> MatchedItemMap; //组-匹配项列表

}

//序列化
inline QDataStream &operator<<(QDataStream &stream, const GrandSearch::MatchedItem &in)
{
    stream << in.item << in.name << in.icon << in.type << in.searcher << in.extra;
    return stream;
}

inline QDataStream &operator>>(QDataStream &stream, GrandSearch::MatchedItem &out)
{
    stream >> out.item >> out.name >> out.icon >> out.type >> out.searcher >> out.extra;
    return stream;
}

Q_DECLARE_METATYPE(GrandSearch::MatchedItem)

#endif // MATCHEDITEM_H
