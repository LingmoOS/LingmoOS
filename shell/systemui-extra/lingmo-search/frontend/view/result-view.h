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
#ifndef RESULTVIEW_H
#define RESULTVIEW_H
#include <QTreeView>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include "search-result-model.h"
#include "show-more-label.h"
#include "title-label.h"
#include "result-view-delegate.h"

namespace LingmoUISearch {

class ResultView : public QTreeView
{
    Q_OBJECT
public:
    ResultView(const QString &plugin_id, QWidget *parent = nullptr);
    ~ResultView() = default;
    bool isSelected();
    int showHeight();
    int getResultNum();
    QModelIndex getModlIndex(int row, int column);
    SearchPluginIface::ResultInfo getIndexResultInfo(QModelIndex &index);

public Q_SLOTS:
    void clearSelectedRow();
    void onRowDoubleClickedSlot(const QModelIndex &);
    void onRowSelectedSlot(const QModelIndex &index);
    void onItemListChanged(const int &);
    void setExpanded(const bool &);
    const bool &isExpanded();
    void onMenuTriggered(QAction *);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool viewportEvent(QEvent *event);

private:
    void initConnections();
    SearchResultModel * m_model = nullptr;
    QString m_plugin_id;
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
    void lableReset();
};

class ResultWidget : public QWidget
{
    Q_OBJECT
public:
    ResultWidget(const QString &plugin_id, QWidget *parent = nullptr);
    ~ResultWidget() = default;
    QString pluginId();
    QString pluginName();
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
    void resetTitleLabel();
    void setTitileLableHide(bool state);

public Q_SLOTS:
    void expandListSlot();
    void reduceListSlot();
    void onListLengthChanged(const int &);

private:
    void initUi();
    void initConnections();

    QString m_plugin_id;
    QString m_plugin_name;
    bool m_enabled = true;
    QVBoxLayout * m_mainLyt = nullptr;
    TitleLabel * m_titleLabel = nullptr;
    ResultView * m_resultView = nullptr;

Q_SIGNALS:
    void startSearch(const QString &);
    void stopSearch();
    void currentRowChanged(const QString &, const SearchPluginIface::ResultInfo&);
    void sendBestListData(const QString &, const SearchPluginIface::ResultInfo&);
    void clearSelectedRow();
    void sizeChanged();
    void rowClicked();
    void resizeWidth(const int &);
    void showMoreClicked();
    void retractClicked();
};
}

#endif // RESULTVIEW_H
