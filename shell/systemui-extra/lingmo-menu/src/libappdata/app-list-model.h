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

#ifndef LINGMO_MENU_APP_LIST_MODEL_H
#define LINGMO_MENU_APP_LIST_MODEL_H

#include "app-list-plugin.h"
#include "context-menu-extension.h"

#include <QAction>
#include <QSortFilterProxyModel>

namespace LingmoUIMenu {

/**
 * @class AppListHeader
 *
 * 应用列表顶部功能区域
 * 显示当前插件的操作选项，如果插件的action为空，那么不显示header.
 */
class AppListHeader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QList<QAction*> actions READ actions NOTIFY actionsChanged)
    friend class AppListModel;
public:
    explicit AppListHeader(QObject *parent = nullptr);

    bool visible() const;
    void setVisible(bool visible);

    QString title() const;
    void setTitle(const QString &title);

    QList<QAction*> actions() const;
    void addAction(QAction *actions);
    void removeAction(QAction *actions);
    void removeAllAction();

Q_SIGNALS:
    void titleChanged();
    void actionsChanged();
    void visibleChanged();

private:
    bool m_visible {false};
    QString m_title;
    QList<QAction*> m_actions;
};

class AppListModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(LingmoUIMenu::AppListHeader *header READ getHeader NOTIFY headerChanged)
    Q_PROPERTY(LingmoUIMenu::LabelBottle *labelBottle READ labelBottle NOTIFY labelBottleChanged)
public:
    explicit AppListModel(QObject *parent = nullptr);

    /**
     * 覆盖全部roles,子model必须返回相同的roles
     * @return
     */
    QHash<int, QByteArray> roleNames() const override;

    AppListHeader *getHeader() const;
    LabelBottle *labelBottle() const;

    void installPlugin(AppListPluginInterface *plugin);
    // reset
    void unInstallPlugin();

    Q_INVOKABLE void openMenu(const int &index, MenuInfo::Location location) const;
    Q_INVOKABLE int findLabelIndex(const QString &label) const;

Q_SIGNALS:
    void headerChanged();
    void labelBottleChanged();

private:
    AppListHeader *m_header {nullptr};
    AppListPluginInterface *m_plugin {nullptr};
};

} // LingmoUIMenu

Q_DECLARE_METATYPE(LingmoUIMenu::AppListModel*)
Q_DECLARE_METATYPE(LingmoUIMenu::AppListHeader*)

#endif //LINGMO_MENU_APP_LIST_MODEL_H
