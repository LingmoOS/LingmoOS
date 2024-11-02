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

#include "app-category-model.h"
#include "basic-app-model.h"

#include <QDebug>

namespace LingmoUIMenu {

AppCategoryModel::AppCategoryModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_mainCategories.insert(QStringLiteral("Audio"), {tr("Audio"), 0});
    m_mainCategories.insert(QStringLiteral("AudioVideo"), {tr("AudioVideo"), 1});
    m_mainCategories.insert(QStringLiteral("Development"), {tr("Development"), 2});
    m_mainCategories.insert(QStringLiteral("Education"), {tr("Education"), 3});
    m_mainCategories.insert(QStringLiteral("Game"), {tr("Game"), 4});
    m_mainCategories.insert(QStringLiteral("Graphics"), {tr("Graphics"), 5});
    m_mainCategories.insert(QStringLiteral("Network"), {tr("Network"), 6});
    m_mainCategories.insert(QStringLiteral("Office"), {tr("Office"), 7});
    m_mainCategories.insert(QStringLiteral("Science"), {tr("Science"), 8});
    m_mainCategories.insert(QStringLiteral("Settings"), {tr("Settings"), 9});
    m_mainCategories.insert(QStringLiteral("System"), {tr("System"), 10});
    m_mainCategories.insert(QStringLiteral("Utility"), {tr("Utility"), 11});
    m_mainCategories.insert(QStringLiteral("Video"), {tr("Video"), 12});
    m_mainCategories.insert(QStringLiteral("Other"), {tr("Other"), 13});

    QSortFilterProxyModel::setSourceModel(BasicAppModel::instance());
    QSortFilterProxyModel::sort(0);
}

AppCategoryModel::Mode AppCategoryModel::mode() const
{
    return m_mode;
}

void AppCategoryModel::setMode(AppCategoryModel::Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    // TODO: 刷新
    invalidate();
}

QVariant AppCategoryModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    if (role == DataEntity::Group) {
        if (m_mode == FirstLatter) {
            return AppCategoryModel::getFirstLatterUpper(QSortFilterProxyModel::data(index, DataEntity::FirstLetter).toString());
        } else {
            return getCategoryName(QSortFilterProxyModel::data(index, DataEntity::Category).toString());
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

bool AppCategoryModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool AppCategoryModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int result = 0;
    if (m_mode == FirstLatter) {
        QString leftKey = AppCategoryModel::getFirstLatterUpper(source_left.data(DataEntity::FirstLetter).toString());
        QString rightKey = AppCategoryModel::getFirstLatterUpper(source_right.data(DataEntity::FirstLetter).toString());

        result = leftKey.compare(rightKey);

    } else {
        // 按分类进行排序
        int leftIndex = getCategoryIndex(source_left.data(DataEntity::Category).toString());
        int rightIndex = getCategoryIndex(source_right.data(DataEntity::Category).toString());

        result = (leftIndex < rightIndex) ? -1 : (leftIndex > rightIndex) ? 1 : 0;
    }

    if (result == 0) {
        // 分类相同时，按打开次数排序; model使用的升序排序，为了保持使用次数多的在前，所以此处使用 > 比较符，
        return source_left.data(DataEntity::LaunchTimes).toInt() > source_right.data(DataEntity::LaunchTimes).toInt();
    }

    return result < 0;
}

inline QString AppCategoryModel::getCategoryName(const QString &categories) const
{
    return m_mainCategories[getCategoryKey(categories)].first;
}

inline int AppCategoryModel::getCategoryIndex(const QString &categories) const
{
    return m_mainCategories[getCategoryKey(categories)].second;
}

QString AppCategoryModel::getCategoryKey(const QString &categories) const
{
    QStringList list;
    // 某些应用使用空格(" ")进行分隔，此处做一下处理
    if (categories.contains( QStringLiteral(";"))) {
        list = categories.split(QStringLiteral(";"));
    } else {
        list = categories.split(QStringLiteral(" "));
    }

    for (const auto &key : list) {
        if (m_mainCategories.contains(key)) {
            return key;
        }
    }

    return QStringLiteral("Other");
}

QString AppCategoryModel::getFirstLatterUpper(const QString &pinyinName)
{
    if (!pinyinName.isEmpty()) {
        QChar first = pinyinName.at(0).toUpper();
        if (first >= 'A' && first <= 'Z') {
            return first;
        } else if (first >= '0' && first <= '9') {
            return QStringLiteral("#");
        }
    }

    return QStringLiteral("&");
}

} // LingmoUIMenu
