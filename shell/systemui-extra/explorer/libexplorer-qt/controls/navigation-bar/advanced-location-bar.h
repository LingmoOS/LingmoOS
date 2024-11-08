/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef ADVANCEDLOCATIONBAR_H
#define ADVANCEDLOCATIONBAR_H

#include <QWidget>
#include "explorer-core_global.h"

class QStackedLayout;

namespace Peony {

class LocationBar;
class PathEdit;
class SearchBarContainer;

class PEONYCORESHARED_EXPORT AdvancedLocationBar : public QWidget
{
    Q_OBJECT
public:
    explicit AdvancedLocationBar(QWidget *parent = nullptr);
    bool isEditing();
    void setAnimationMode(bool isAnimation);

Q_SIGNALS:
    void updateWindowLocationRequest(const QString &uri, bool addHistory = true, bool forceUpdate = false);
    void refreshRequest();
    void searchRequest(const QString &path, const QString &key);
    void updateFileTypeFilter(const int &index);
//    void clearTrash();
    void updateSearchProgress(bool searching);

public Q_SLOTS:
    void updateLocation(const QString &uri);
    void startEdit();
    void cancelEdit();
    void finishEdit();
    void switchEditMode(bool bSearchMode);
    void clearSearchBox();
    void deselectSearchBox();
    QString processSpecialChar(QString key);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    QStackedLayout *m_layout;

    LocationBar *m_bar;
    PathEdit *m_edit;
    SearchBarContainer *m_search_bar;

    QString m_text;
    QString m_last_non_search_path;
    QString m_last_key = "";

    bool m_in_search_mode = false;

    const QStringList SPECIAL_CHARS = {"\$", "\|", "\^", "\*"};
};

}

#endif // ADVANCEDLOCATIONBAR_H
