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
#include "search-result-property-info.h"
#include <QMetaType>
#include <QVariant>
using namespace LingmoUISearch;
class LingmoUISearch::SearchResultPropertyInfoPrivate
{
public:
    SearchProperty::SearchResultProperty m_property;
    QString m_name;
    QString m_displayName;
    QMetaType::Type m_valueType;
};
SearchResultPropertyInfo::SearchResultPropertyInfo():d(new SearchResultPropertyInfoPrivate)
{
    d->m_property = SearchProperty::SearchResultProperty::Invalid;
    d->m_name = QStringLiteral("Invalid");
    d->m_valueType = QMetaType::UnknownType;
}

SearchResultPropertyInfo::SearchResultPropertyInfo(SearchProperty::SearchResultProperty property): d(new SearchResultPropertyInfoPrivate)
{
    d->m_property = property;
    switch (property) {
    case SearchProperty::SearchResultProperty::FilePath:
        d->m_name = QStringLiteral("FilePath");
        d->m_displayName = tr("file path");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::FileName:
        d->m_name = QStringLiteral("FileName");
        d->m_displayName = tr("file name");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::FileIconName:
        d->m_name = QStringLiteral("FileIconName");
        d->m_displayName = tr("file icon name");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::ModifiedTime:
        d->m_name = QStringLiteral("ModifiedTime");
        d->m_displayName = tr("modified time");
        d->m_valueType = QMetaType::QDateTime;
        break;

    case SearchProperty::SearchResultProperty::ApplicationDesktopPath:
        d->m_name = QStringLiteral("ApplicationDesktopPath");
        d->m_displayName = tr("application desktop file path");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::ApplicationLocalName:
        d->m_name = QStringLiteral("ApplicationLocalName");
        d->m_displayName = tr("application local name");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::ApplicationIconName:
        d->m_name = QStringLiteral("ApplicationIconName");
        d->m_displayName = tr("application icon name");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::ApplicationDescription:
        d->m_name = QStringLiteral("ApplicationDescription");
        d->m_displayName = tr("application description");
        d->m_valueType = QMetaType::QString;
        break;

    case SearchProperty::SearchResultProperty::IsOnlineApplication:
        d->m_name = QStringLiteral("IsOnlineApplication");
        d->m_displayName = tr("is online application");
        d->m_valueType = QMetaType::Int;
        break;

    case SearchProperty::SearchResultProperty::ApplicationPkgName:
        d->m_name = QStringLiteral("ApplicationPkgName");
        d->m_displayName = tr("application package name");
        d->m_valueType = QMetaType::QString;
        break;
    default:
        break;
    }
}

SearchResultPropertyInfo::SearchResultPropertyInfo(const SearchResultPropertyInfo &other): d(new SearchResultPropertyInfoPrivate(*other.d))
{
}

SearchResultPropertyInfo &LingmoUISearch::SearchResultPropertyInfo::operator=(const SearchResultPropertyInfo &rhs)
{
    *d = *rhs.d;
    return *this;
}

bool SearchResultPropertyInfo::operator==(const SearchResultPropertyInfo &rhs) const
{
    return d->m_name == rhs.d->m_name && d->m_displayName == rhs.d->m_displayName &&
           d->m_property == rhs.d->m_property;
}

SearchProperty::SearchResultProperty LingmoUISearch::SearchResultPropertyInfo::property() const
{
    return d->m_property;
}

QString SearchResultPropertyInfo::name() const
{
    return d->m_name;
}

QString SearchResultPropertyInfo::displayName() const
{
    return d->m_displayName;
}
