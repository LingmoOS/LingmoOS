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
#include "application-property-helper.h"
#include <QDateTime>
using namespace LingmoUISearch;
namespace LingmoUISearch {

class ApplicationPropertyHelperPrivate
{
    friend class ApplicationPropertyHelper;
private:
    ApplicationProperty::Property m_property;
    QString m_databaseField;
    QMetaType::Type m_valueType;
};
}
ApplicationPropertyHelper::ApplicationPropertyHelper(): d(new ApplicationPropertyHelperPrivate)
{
    d->m_property = ApplicationProperty::Invalid;
    d->m_valueType = QMetaType::UnknownType;
}

ApplicationPropertyHelper::ApplicationPropertyHelper(ApplicationProperty::Property property): d(new ApplicationPropertyHelperPrivate)
{
    d->m_property = property;
    switch (property) {
    case ApplicationProperty::DesktopFilePath:
        d->m_databaseField = "DESKTOP_FILE_PATH";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::ModifiedTime:
        d->m_databaseField = "MODIFYED_TIME";
        d->m_valueType = QMetaType::QDateTime;
        break;
    case ApplicationProperty::InsertTime:
        d->m_databaseField = "INSERT_TIME";
        d->m_valueType = QMetaType::QDateTime;
        break;
    case ApplicationProperty::LocalName:
        d->m_databaseField = "LOCAL_NAME";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::NameEn:
        d->m_databaseField = "NAME_EN";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::NameZh:
        d->m_databaseField = "NAME_ZH";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::PinyinName:
        d->m_databaseField = "PINYIN_NAME";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::FirstLetterOfPinyin:
        d->m_databaseField = "FIRST_LETTER_OF_PINYIN";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::FirstLetterAll:
        d->m_databaseField = "FIRST_LETTER_ALL";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Icon:
        d->m_databaseField = "ICON";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Type:
        d->m_databaseField = "TYPE";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Category:
        d->m_databaseField = "CATEGORY";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Exec:
        d->m_databaseField = "EXEC";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Comment:
        d->m_databaseField = "CATEGORY";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Md5:
        d->m_databaseField = "MD5";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::LaunchTimes:
        d->m_databaseField = "LAUNCH_TIMES";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::Favorites:
        d->m_databaseField = "FAVORITES";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::Launched:
        d->m_databaseField = "LAUNCHED";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::Top:
        d->m_databaseField = "TOP";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::Lock:
        d->m_databaseField = "LOCK";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::DontDisplay:
        d->m_databaseField = "DONT_DISPLAY";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::AutoStart:
        d->m_databaseField = "AUTO_START";
        d->m_valueType = QMetaType::Int;
        break;
    case ApplicationProperty::StartUpWMClass:
        d->m_databaseField = "START_UP_WMCLASS";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::Keywords:
        d->m_databaseField = "KEYWORDS";
        d->m_valueType = QMetaType::QString;
        break;
    case ApplicationProperty::LocalKeywords:
        d->m_databaseField = "LOCAL_KEYWORDS";
        d->m_valueType = QMetaType::QString;
        break;
    default:
        break;
    }
}

ApplicationPropertyHelper::ApplicationPropertyHelper(const ApplicationPropertyHelper &other): d(new ApplicationPropertyHelperPrivate(*other.d))
{

}

ApplicationPropertyHelper::~ApplicationPropertyHelper()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

ApplicationPropertyHelper &ApplicationPropertyHelper::operator =(const ApplicationPropertyHelper &rhs)
{
    *d = *rhs.d;
    return *this;
}

bool ApplicationPropertyHelper::operator ==(const ApplicationPropertyHelper &rhs) const
{
    return d->m_databaseField == rhs.d->m_databaseField && d->m_valueType == rhs.d->m_valueType;
}

ApplicationProperty::Property ApplicationPropertyHelper::info()
{
    return d->m_property;
}

QString ApplicationPropertyHelper::dataBaseField()
{
    return d->m_databaseField;
}

QMetaType::Type ApplicationPropertyHelper::valueType()
{
    return d->m_valueType;
}

QString ApplicationPropertyHelper::toDataBaseString(QVariant &value)
{
    switch (d->m_valueType) {
    case QMetaType::Type::UnknownType:
        return QString();
    case QMetaType::Type::QString:
        return value.toString();
    case QMetaType::Type::QDateTime:
        return value.toDateTime().toString("yyyy-MM-dd hh:mm:ss");
    default:
        return {};
        break;
    }
}
