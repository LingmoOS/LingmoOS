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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef SEARCHRESULTPROPERTYINFO_H
#define SEARCHRESULTPROPERTYINFO_H
#include "search-result-property.h"
#include <QObject>
namespace LingmoUISearch {
class SearchResultPropertyInfoPrivate;
class SearchResultPropertyInfo: public QObject
{
    Q_OBJECT
public:
    SearchResultPropertyInfo();
    explicit SearchResultPropertyInfo(SearchProperty::SearchResultProperty property);
    SearchResultPropertyInfo(const SearchResultPropertyInfo &other);
    SearchResultPropertyInfo& operator=(const SearchResultPropertyInfo& rhs);
    bool operator==(const SearchResultPropertyInfo& rhs) const;

    SearchProperty::SearchResultProperty property() const;
    QString name() const;
    QString displayName() const;
private:
    SearchResultPropertyInfoPrivate *d = nullptr;
};
}
#endif // SEARCHRESULTPROPERTYINFO_H
