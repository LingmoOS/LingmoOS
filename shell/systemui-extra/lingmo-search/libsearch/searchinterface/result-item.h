/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef RESULTITEM_H
#define RESULTITEM_H

#include <QString>
#include "search-result-property.h"
namespace LingmoUISearch {
class ResultItemPrivate;
class ResultItem
{
public:
    explicit ResultItem();
    virtual ~ResultItem();

    explicit ResultItem(const size_t searchId);
    explicit ResultItem(const QString &itemKey);
    ResultItem(const size_t searchId, const QString &itemKey, const SearchResultPropertyMap &map);
    ResultItem(const ResultItem &item);
    ResultItem &operator=(const ResultItem &other);
    ResultItem &operator=(ResultItem &&other) Q_DECL_NOEXCEPT;

    void setSearchId(const size_t searchId);
    void setItemKey(const QString &itemKey);
    size_t getSearchId() const;
    QString getItemKey() const;
    void setValue(SearchProperty::SearchResultProperty property, const QVariant &value);
    QVariant getValue(SearchProperty::SearchResultProperty property) const;
    SearchResultPropertyMap getAllValue() const;

private:
    ResultItemPrivate *d;
};
}

#endif // RESULTITEM_H
