/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

class QToolButton;
namespace Peony {

class AdvancedLocationBar;

class SearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchWidget(QWidget *parent = nullptr);
    void searchButtonClicked();
    void setSearchMode(bool mode);
    void updateCloseSearch(QString icon);
    bool isSearchMode();

Q_SIGNALS:
    void updateLocationRequest(const QString &uri, bool addHistory = true, bool force = true);
    void updateSearchRequest(bool showSearch);
    void refreshRequest();
    void updateFileTypeFilter(const int &index);
    void updateLocation(const QString &uri);
    void cancelEdit();
    void finishEdit();
    void clearSearchBox();
    void changeSearchMode(bool mode);
    void updateSearch(const QString &uri, const QString &key = "", bool updateKey = false);

public Q_SLOTS:
    void startEdit(bool bSearch = false);
    void updateSearchRecursive(bool recursive);
    void closeSearch();
    void setGlobalFlag(bool isGlobal);
    void updateTabletModeValue(bool isTabletMode);
    void updateSearchProgress(bool isSearching);

private:
    void initAnimation();

    QToolButton *m_closeSearchButton;
    QToolButton *m_searchButton;
    AdvancedLocationBar *m_locationBar;

    bool m_searchMode = false;
    bool m_searchGlobal = false;
    bool m_searchRecursive = true;
};

}

#endif // SEARCHWIDGET_H
