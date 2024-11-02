/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_CONFIG_H
#define LINGMO_QUICK_CONFIG_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QStringList>
#include <QHash>

namespace LingmoUIQuick {

class ConfigIFace : public QObject
{
    Q_OBJECT
public:
    explicit ConfigIFace(QObject *parent = nullptr);

    Q_INVOKABLE virtual QStringList keys() const = 0;
    Q_INVOKABLE virtual QVariant getValue(const QString &key) const = 0;
    Q_INVOKABLE virtual void setValue(const QString &key, const QVariant &value) = 0;

    /**
     * 从数据源同步数据或写入数据
     */
    Q_INVOKABLE virtual void forceSync() = 0;

Q_SIGNALS:
    void configChanged(const QString &key);
};

class Config;

/**
 * 一个组必须是一个数组，数组中的对象顺序不限
 * 第一个String是组的名称
 * 第二个String是组内对象的主键名称，唯一标识一个对象
 */
typedef QHash<QString, QString> ConfigGroupInfo;
typedef QList<Config*> ConfigList;

/**
 * 一个配置文件节点。
 * 如果从文件或者一个Map生成，那么就是一个根节点。
 * 如果由父配置构造，那么就是一个子节点，并且有一个指向父节点的指针。
 *
 * 构造方式：
 * 1.从文件加载
 * 2.从VariantMap加载
 * 3.从父节点获取
 *
 */
class ConfigPrivate;

class Config : public ConfigIFace
{
    Q_OBJECT
public:
    /**
      * 从文件加载数据
      * @param file 一个json文件
      * @param groupInfo 分组信息
      * @param parent 父对象
      */
    explicit Config(const QString &file, QObject *parent = nullptr);

    /**
     * 从VariantMap加载数据
     * @param data 数据
     * @param groupInfo 分组信息
     * @param parent 父节点
     */
    explicit Config(const QVariantMap &data, QObject *parent = nullptr);

    ~Config() override;

    // Apis
    const QVariantMap &data() const;
    bool contains(const QString &key) const;
    QStringList keys() const override;
    QVariant getValue(const QString &key) const override;
    void setValue(const QString &key, const QVariant &value) override;
    void forceSync() override;

    bool isRoot() const;
    bool isNull() const;
    const QVariant &id() const;
    const QString &group() const;
    const ConfigGroupInfo &groupInfo() const;
    // void setGroupInfo(const ConfigGroupInfo &groupInfo);
    void addGroupInfo(const QString &group, const QString &key);
    // TODO: remove group
    // void removeGroupInfo(const QString &group);

    ConfigList children(const QString &group) const;
    int numberOfChildren(const QString &group) const;
    Config *child(const QString &group, const QVariant &id) const;

    /**
     * 添加子节点
     * @param group 组名
     * @param childData 数据
     */
    void addChild(const QString &group, const QVariantMap &childData);

    /**
     * 删除一个子节点
     * @param group 组名
     * @param id 主键值
     */
    void removeChild(const QString &group, const QVariant &id);

    /**
     * 删除一个key
     * @param key 要删除的键
     */
    void removeKey(const QString &key);

Q_SIGNALS:
    void childAdded(const QString &group, const QVariant &id);
    void childRemoved(const QString &group, const QVariant &id);

public:
    bool event(QEvent *event) override;

private:
    /**
     * 从父节点继承数据
     * 作为私有构造函数,由内部函数调用
     * @param data 数据
     * @param groupInfo 分组信息
     * @param parent 父节点
     */
    explicit Config(const QVariantMap &data, Config *parent = nullptr);
    void extractChildren();
    void extractGroup(const QString &group, const QString &key);

    void setId(const QVariant &id);
    void setGroup(const QString &group);

    void requestUpdate();
private:
    ConfigPrivate *d {nullptr};
    friend class ConfigPrivate;
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_CONFIG_H
