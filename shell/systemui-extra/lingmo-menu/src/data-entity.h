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

#ifndef LINGMO_MENU_DATA_ENTITY_H
#define LINGMO_MENU_DATA_ENTITY_H

#include <QObject>
#include <QHash>
#include <QVariant>

class DataEntityPrivate;

namespace LingmoUIMenu {

class DataType
{
    Q_GADGET
public:
    enum Type {
        Normal,   // 普通Item
        Label,    // 标签数据
        Folder,    // 应用组
        Files      // 文件夹
    };
    Q_ENUM(Type);
};

// 应用数据
class DataEntity
{
    Q_GADGET
    Q_PROPERTY(DataType::Type type READ type WRITE setType)
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString icon READ icon WRITE setIcon)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString comment READ comment WRITE setComment)
    Q_PROPERTY(QString extraData READ extraData WRITE setExtraData)
public:
    static QHash<int, QByteArray> AppRoleNames();
    enum PropertyName {
        Id = 0,
        Type,
        Icon,
        Name,
        Comment,
        ExtraData,
        Category,         /**> 应用类别 */
        Group,            /**> 应用分组 */
        FirstLetter,      /**> 应用名称首字母 */
        InstallationTime, /**> 安装时间 format: yyyy-MM-dd hh:mm:ss */
        IsLaunched,       /**> 是否启动过该程序 */
        LaunchTimes,      /**> 应用被启动的次数 */
        IsLocked,         /**> 应用是否锁定 */
        Favorite,         /**> 是否被收藏及序号, 小于或等于0表示未被收藏 */
        Top,              /**> 是否被置顶及序号, 小于或等于0表示未被置顶 */
        RecentInstall,
        Entity            /**> 返回自己的拷贝 */
    };
    DataEntity();
    DataEntity(DataType::Type type, const QString& name, const QString& icon, const QString& comment, const QString& extraData);

    virtual ~DataEntity();

    DataEntity(const DataEntity& other);
    DataEntity &operator=(const DataEntity& other);
    DataEntity(DataEntity&& other) noexcept;
    DataEntity &operator=(DataEntity&& other) noexcept;

    QVariant getValue(DataEntity::PropertyName property) const;
    void setValue(DataEntity::PropertyName property, const QVariant &value);

    int top() const;
    void setTop(int top);

    bool isLock() const;
    void setLock(bool lock);

    bool isRecentInstall() const;
    void setRecentInstall(bool recentInsatll);

    int launched() const;
    void setLaunched(int launched);

    int favorite() const;
    void setFavorite(int favorite);

    int launchTimes() const;
    void setLaunchTimes(int launchTimes);

    double priority() const;
    void setPriority(double priority);

    QString insertTime() const;
    void setInsertTime(const QString& insertTime);

    QString id() const;
    void setId(const QString& id);

    void setCategory(const QString& category);
    QString category() const;

    void setFirstLetter(const QString& firstLetter);
    QString firstLetter() const;

    void setType(DataType::Type type);
    DataType::Type type() const;

    void setIcon(const QString& icon);
    QString icon() const;

    void setName(const QString& name);
    QString name() const;

    void setComment(const QString& comment);
    QString comment() const;

    void setExtraData(const QString& extraData);
    QString extraData() const;

    void setGroup(const QString& group);
    QString group() const;

private:
    DataEntityPrivate *d {nullptr};
};

} // LingmoUIMenu

Q_DECLARE_METATYPE(LingmoUIMenu::DataType)
Q_DECLARE_METATYPE(LingmoUIMenu::DataEntity)

#endif //LINGMO_MENU_DATA_ENTITY_H
