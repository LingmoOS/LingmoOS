/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_TASK_MANAGER_ACTIONS_H
#define LINGMO_TASK_MANAGER_ACTIONS_H
#include <QString>
#include <QIcon>
#include <QMetaType>
#include <QAction>
namespace TaskManager {
class ActionPrivate;

class ActionsPrivate;

class Action : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Invalid,

        //任务栏图标action
        NewInstance,          //启动新实例
        DesktopAction,        //desktop文件中的action
        RemoveQuickLauncher,  //移除快速启动按钮
        AddQuickLauncher,     //增加快速启动按钮
        Exit,                 //关闭所有窗口

        //单窗口action
        Close,                //关闭窗口
        Minimize,             //最小化
        Maximize,             //最大化
        Restore,              //恢复
        KeepAbove,            //置顶
        UnsetKeepAbove        //取消置顶
    };
    Q_ENUM(Type)

    explicit Action(QObject *parent = nullptr);
    Action(const QString &name, const QString &displayName, const QIcon &icon, Type type, const QVariant &param = {}, QObject *parent = nullptr);
    Action(const Action &other);
    Action &operator=(const Action &other) = delete;
    Action &operator=(Action &&other) = delete;
    bool operator==(const Action &other) const;
    ~Action();

    QString name() const;
    void setName(const QString &name);

    QString text() const;
    void setText(const QString &name);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    Type type() const;
    void setType(const Type &type);

    QVariant param();
    void setParam(const QVariant &param);

    bool enabled() const;
    void setEnabled(bool enabled);

    QAction* internalAction() const;

Q_SIGNALS:
    void actionTriggered(const QVariant& param);

private:
    void initConnection();
    QSharedPointer<ActionPrivate> d;
};

typedef QVector<Action *> Actions;
}
Q_DECLARE_METATYPE(TaskManager::Actions);
#endif //LINGMO_TASK_MANAGER_ACTIONS_H
