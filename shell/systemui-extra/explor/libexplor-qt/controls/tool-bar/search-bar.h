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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include "fm-window.h"
#include "explor-core_global.h"

#include <QLineEdit>
#include <QStandardItemModel>
#include <QTableView>

class QStringListModel;

namespace Peony {

class SearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchBar(FMWindow *window, QWidget *parent = nullptr);

Q_SIGNALS:
    void searchKeyChanged(const QString &key);
    void searchRequest(const QString &key);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

public Q_SLOTS:
    void init(bool hasTopWindow);
    void initTableModel();
    void updateTableModel();
    void onTableClicked(const QModelIndex &index);
    void clearSearchRecord();
    void hideTableView();

private:
    FMWindow *m_top_window;
    QStandardItemModel *m_model = nullptr;
    QTableView *m_table_view = nullptr;
};

}

#endif // SEARCHBAR_H
