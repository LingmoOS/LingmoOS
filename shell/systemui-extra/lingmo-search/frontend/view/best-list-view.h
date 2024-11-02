/*
 *
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
 *
 */
#ifndef BESTLISTVIEW_H
#define BESTLISTVIEW_H
#include <QTreeView>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include "best-list-model.h"
#include "show-more-label.h"
#include "title-label.h"
#include "result-view-delegate.h"

namespace LingmoUISearch {

class BestListView : public QTreeView
{
    Q_OBJECT
public:
    BestListView(QWidget *parent = nullptr);
    ~BestListView() = default;

    bool isSelected();
    int showHeight();
    int getResultNum();
    QModelIndex getModlIndex(int row, int column);
    SearchPluginIface::ResultInfo getIndexResultInfo(QModelIndex &index);
    const QString getPluginInfo(const QModelIndex&index);
    int getResultHeight();

public Q_SLOTS:
    void clearSelectedRow();
    void onRowDoubleClickedSlot(const QModelIndex &);
    void onRowSelectedSlot(const QModelIndex &index);
    void onItemListChanged(const int &);
    void setExpanded(const bool &);
    const bool &isExpanded();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool viewportEvent(QEvent *event) override;

private:
    void initConnections();

    BestListModel * m_model = nullptr;
    bool m_is_selected = false;
    ResultViewDelegate * m_styleDelegate = nullptr;
    int m_count = 0;
    QModelIndex m_tmpCurrentIndex;
    QModelIndex m_tmpMousePressIndex;
    QTimer *m_touchTimer;

Q_SIGNALS:
    void startSearch(const QString &);
    void stopSearch();
    void currentRowChanged(const QString &, const SearchPluginIface::ResultInfo&);
    void sendBestListData(const QString &, const SearchPluginIface::ResultInfo&);
    void listLengthChanged(const int &);
    void rowClicked();

};


class BestListWidget : public QWidget
{
    Q_OBJECT
public:
    BestListWidget(QWidget *parent = nullptr);
    ~BestListWidget() = default;

    QString getWidgetName();
    void setEnabled(const bool&);
    void clearResult();
    int getResultNum();
    void setResultSelection(const QModelIndex &index);
    void clearResultSelection();
    QModelIndex getModlIndex(int row, int column);
    void activateIndex();
    QModelIndex getCurrentSelection();
    bool getExpandState();
    SearchPluginIface::ResultInfo getIndexResultInfo(QModelIndex &index);
    const QString getPluginInfo(const QModelIndex&index);
    int getResultHeight();

private:
    void initUi();
    void initConnections();

    bool m_enabled = true;
    QVBoxLayout * m_mainLyt = nullptr;
    TitleLabel * m_titleLabel = nullptr;
    BestListView * m_bestListView = nullptr;

public Q_SLOTS:
    void expandListSlot();
    void reduceListSlot();
    void onListLengthChanged(const int &);

Q_SIGNALS:
    void startSearch(const QString &);
    void stopSearch();
    void currentRowChanged(const QString &, const SearchPluginIface::ResultInfo&);
    void sendBestListData(const QString &, const SearchPluginIface::ResultInfo&);
    void clearSelectedRow();
    void sizeChanged();
    void rowClicked();

};

}

#endif // BESTLISTVIEW_H
