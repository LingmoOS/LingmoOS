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
 */

#include "data-entity.h"

using namespace LingmoUIMenu;

class DataEntityPrivate
{
public:
    DataEntityPrivate() = default;
    DataEntityPrivate(DataType::Type type, QString name, QString icon, QString comment, QString extraData)
        : type(type), name(std::move(name)), icon(std::move(icon)), comment(std::move(comment)), extraData(std::move(extraData)) {}

    bool lock{false};       // 应用锁定
    bool recentInstall{false};
    int launched{0};        // 是否启动过
    int top{0};             // 置顶状态及序号
    int favorite{0};        // 收藏状态及序号
    int launchTimes{0};     // 启动次数
    double priority{0};
    DataType::Type type {DataType::Normal};
    QString id;             // 应用可执行文件路径
    QString icon;
    QString name;
    QString category;
    QString firstLetter;
    QString comment;        // 应用描述
    QString extraData;      // 额外的数据
    QString insertTime;     //安装的时间
    QString group;          // 分类
};

DataEntity::DataEntity() : d(new DataEntityPrivate) {}

DataEntity::DataEntity(DataType::Type type, const QString& name, const QString& icon, const QString& comment, const QString& extraData)
    : d(new DataEntityPrivate(type, name, icon, comment, extraData))
{

}

int DataEntity::top() const
{
    return d->top;
}

void DataEntity::setTop(int top)
{
    d->top = top;
}

bool DataEntity::isLock() const
{
    return d->lock;
}

void DataEntity::setLock(bool lock)
{
    d->lock = lock;
}

bool DataEntity::isRecentInstall() const
{
    return d->recentInstall;
}

void DataEntity::setRecentInstall(bool recentInsatll)
{
    d->recentInstall = recentInsatll;
}

int DataEntity::launched() const
{
    return d->launched;
}

void DataEntity::setLaunched(int launched)
{
    d->launched = launched;
}

int DataEntity::favorite() const
{
    return d->favorite;
}

void DataEntity::setFavorite(int favorite)
{
    d->favorite = favorite;
}

int DataEntity::launchTimes() const
{
    return d->launchTimes;
}

void DataEntity::setLaunchTimes(int launchTimes)
{
    d->launchTimes = launchTimes;
}

double DataEntity::priority() const
{
    return d->priority;
}

void DataEntity::setPriority(double priority)
{
    d->priority = priority;
}

QString DataEntity::insertTime() const
{
    return d->insertTime;
}

void DataEntity::setInsertTime(const QString &insertTime)
{
    d->insertTime = insertTime;
}

QString DataEntity::id() const
{
    return d->id;
}

void DataEntity::setId(const QString &id)
{
    d->id = id;
}

void DataEntity::setCategory(const QString &category)
{
    d->category = category;
}

QString DataEntity::category() const
{
    return d->category;
}

void DataEntity::setFirstLetter(const QString &firstLetter)
{
    d->firstLetter = firstLetter;
}

QString DataEntity::firstLetter() const
{
    return d->firstLetter;
}

void DataEntity::setType(DataType::Type type)
{
    d->type = type;
}

DataType::Type DataEntity::type() const
{
    return d->type;
}

void DataEntity::setIcon(const QString &icon)
{
    d->icon = icon;
}

QString DataEntity::icon() const
{
    return d->icon;
}

void DataEntity::setName(const QString &name)
{
    d->name = name;
}

QString DataEntity::name() const
{
    return d->name;
}

void DataEntity::setComment(const QString &comment)
{
    d->comment = comment;
}

QString DataEntity::comment() const
{
    return d->comment;
}

void DataEntity::setExtraData(const QString &extraData)
{
    d->extraData = extraData;
}

QString DataEntity::extraData() const
{
    return d->extraData;
}

QHash<int, QByteArray> DataEntity::AppRoleNames()
{
    QHash<int, QByteArray> names;
    names.insert(DataEntity::Id, "id");
    names.insert(DataEntity::Type, "type");
    names.insert(DataEntity::Icon, "icon");
    names.insert(DataEntity::Name, "name");
    names.insert(DataEntity::Comment, "comment");
    names.insert(DataEntity::ExtraData, "extraData");
    names.insert(DataEntity::Category, "category");
    names.insert(DataEntity::Group, "group");
    names.insert(DataEntity::FirstLetter, "firstLetter");
    names.insert(DataEntity::InstallationTime, "installationTime");
    names.insert(DataEntity::IsLaunched, "isLaunched");
    names.insert(DataEntity::LaunchTimes, "launchTimes");
    names.insert(DataEntity::IsLocked, "isLocked");
    names.insert(DataEntity::Favorite, "favorite");
    names.insert(DataEntity::Top, "top");
    names.insert(DataEntity::RecentInstall, "recentInstall");
    names.insert(DataEntity::Entity, "entity");
    return names;
}

DataEntity::DataEntity(const DataEntity &other) : d(new DataEntityPrivate)
{
    *d = *(other.d);
}

DataEntity& DataEntity::operator=(const DataEntity &other)
{
    if (this != &other) {
        *d = *(other.d);
    }
    return *this;
}

DataEntity::DataEntity(DataEntity &&other) noexcept : d(new DataEntityPrivate)
{
    *d = *(other.d);
}

DataEntity &DataEntity::operator=(DataEntity &&other) noexcept
{
    *d = *(other.d);
    return *this;
}

DataEntity::~DataEntity()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QVariant DataEntity::getValue(DataEntity::PropertyName property) const
{
    switch (property) {
        case Id:
            return d->id;
        case Type:
            return d->type;
        case Icon:
            return d->icon;
        case Name:
            return d->name;
        case Comment:
            return d->comment;
        case ExtraData:
            return d->extraData;
        case Category:
            return d->category;
        case Group:
            return d->group;
        case FirstLetter:
            return d->firstLetter;
        case InstallationTime:
            return d->insertTime;
        case IsLaunched:
            return d->launched;
        case LaunchTimes:
            return d->launchTimes;
        case IsLocked:
            return d->lock;
        case Favorite:
            return d->favorite;
        case Top:
            return d->top;
        case RecentInstall:
            return d->recentInstall;
        case Entity:
            return QVariant::fromValue(*this);
        default:
            break;
    }

    return {};
}

void DataEntity::setValue(DataEntity::PropertyName property, const QVariant &value)
{
    switch (property) {
        case Id:
            d->id = value.toString();
            break;
        case Type:
            d->type = value.value<DataType::Type>();
            break;
        case Icon:
            d->icon = value.toString();
            break;
        case Name:
            d->name = value.toString();
            break;
        case Comment:
            d->comment = value.toString();
            break;
        case ExtraData:
            d->extraData = value.toString();
            break;
        case Category:
            d->category = value.toString();
            break;
        case Group:
            d->group = value.toString();
            break;
        case FirstLetter:
            d->firstLetter = value.toString();
            break;
        case InstallationTime:
            d->insertTime = value.toString();
            break;
        case IsLaunched:
            d->launched = value.toBool();
            break;
        case LaunchTimes:
            d->launchTimes = value.toInt();
            break;
        case IsLocked:
            d->lock = value.toBool();
            break;
        case Favorite:
            d->favorite = value.toInt();
            break;
        case Top:
            d->top = value.toInt();
            break;
        case RecentInstall:
            d->recentInstall = value.toBool();
            break;
        default:
            break;
    }
}

void DataEntity::setGroup(const QString &group)
{
    d->group = group;
}

QString DataEntity::group() const
{
    return d->group;
}
