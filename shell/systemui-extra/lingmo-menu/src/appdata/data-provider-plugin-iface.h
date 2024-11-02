/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

#ifndef LINGMO_MENU_DATA_PROVIDER_PLUGIN_IFACE_H
#define LINGMO_MENU_DATA_PROVIDER_PLUGIN_IFACE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

#include "commons.h"

namespace LingmoUIMenu {

class PluginGroup
{
    Q_GADGET
public:
    enum Group {
        Button = 0,
        SortMenuItem,
        Search,
    };
    Q_ENUM(Group)
};

class DataUpdateMode
{
    Q_GADGET
public:
    enum Mode {
        Reset = 0,
        Append,
        Prepend,
        Insert,
        Update
    };
    Q_ENUM(Mode)
};

class DataProviderPluginIFace : public QObject
{
    Q_OBJECT
public:
    explicit DataProviderPluginIFace() : QObject(nullptr) {}
    virtual int index() = 0;
    virtual QString id() = 0;
    virtual QString name() = 0;
    virtual QString icon() = 0;
    virtual QString title() = 0;
    virtual PluginGroup::Group group() = 0;

    /**
     * 获取插件存放的数据
     * @return 数据容器
     */
    virtual QVector<DataEntity> data() = 0;
    virtual QVector<LabelItem> labels() { return {}; };

    /**
     * 强制刷新数据，刷新完成后发送dataChange信号
     */
    virtual void forceUpdate() = 0;
    virtual void forceUpdate(QString &key) {};

public Q_SLOTS:
    virtual void update(bool isShowed) {};

Q_SIGNALS:
    /**
     * 数据变化
     */
    void dataChanged(QVector<DataEntity> data, DataUpdateMode::Mode mode = DataUpdateMode::Reset, quint32 index = 0);
    void labelChanged();
};

} // LingmoUIMenu

//Q_DECLARE_METATYPE(LingmoUIMenu::DataUpdateMode::Mode)

#endif //LINGMO_MENU_DATA_PROVIDER_PLUGIN_IFACE_H
