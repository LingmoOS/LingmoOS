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

#ifndef LINGMO_MENU_APP_CATEGORY_MODEL_H
#define LINGMO_MENU_APP_CATEGORY_MODEL_H

#include "app-list-plugin.h"
#include <QAction>
#include <QSortFilterProxyModel>

namespace LingmoUIMenu {

class AppCategoryModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum Mode {
        FirstLatter, /**> 按首字母排序 */
        Category     /**> 按类型排序 */
    };
    Q_ENUM(Mode)

    explicit AppCategoryModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;

    Mode mode() const;
    void setMode(Mode mode);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    static QString getFirstLatterUpper(const QString &pinyinName) ;
    inline QString getCategoryName(const QString &categories) const;
    inline int getCategoryIndex(const QString &categories) const;
    QString getCategoryKey(const QString &categories) const;

private:
    Mode m_mode { Category };
    QMap<QString, QPair<QString, int> > m_mainCategories;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_CATEGORY_MODEL_H
