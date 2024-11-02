/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "result-item.h"
using namespace LingmoUISearch;
namespace LingmoUISearch {
class ResultItemPrivate
{
    friend class ResultItem;
private:
    size_t m_searchId = 0;
    QString m_itemKey;
    SearchResultPropertyMap m_data;
};
}
ResultItem::ResultItem() : d(new ResultItemPrivate())
{
}

ResultItem::ResultItem(const size_t searchId) : d(new ResultItemPrivate())
{
    d->m_searchId = searchId;
}

ResultItem::ResultItem(const QString &itemKey) : d(new ResultItemPrivate())
{
    d->m_itemKey = itemKey;
}

ResultItem::ResultItem(const size_t searchId, const QString &itemKey, const SearchResultPropertyMap &map) : d(new ResultItemPrivate())
{
    d->m_searchId = searchId;
    d->m_itemKey = itemKey;
    d->m_data = map;
}

void ResultItem::setSearchId(const size_t searchId)
{
    d->m_searchId = searchId;
}

void ResultItem::setItemKey(const QString &itemKey)
{
    d->m_itemKey = itemKey;
}

size_t ResultItem::getSearchId() const
{
    return d->m_searchId;
}

QString ResultItem::getItemKey() const
{
    return d->m_itemKey;
}

void ResultItem::setValue(SearchProperty::SearchResultProperty property, const QVariant &value)
{
    d->m_data.insert(property, value);
}

QVariant ResultItem::getValue(SearchProperty::SearchResultProperty property) const
{
    return d->m_data.value(property);
}

SearchResultPropertyMap ResultItem::getAllValue() const
{
    return d->m_data;
}

ResultItem::~ResultItem()
{
    if (d)
        delete d;
    d = nullptr;
}

ResultItem::ResultItem(const ResultItem &item): d(new ResultItemPrivate(*item.d))
{
}

ResultItem &ResultItem::operator=(const ResultItem &other)
{
    *d = *other.d;
    return *this;
}
ResultItem &ResultItem::operator=(ResultItem &&other) Q_DECL_NOEXCEPT
{
    d = other.d;
    other.d = nullptr;
    return *this;
}
