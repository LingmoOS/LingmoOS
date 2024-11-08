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

#ifndef FMWINDOW_H
#define FMWINDOW_H

#include <QMainWindow>
#include "explorer-core_global.h"
#include "advanced-location-bar.h"
#include "advance-search-bar.h"
#include "FMWindowIface.h"
#include "fm-window-factory.h"
#include <memory>

#include <QTimer>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolBar>

class QSplitter;

namespace Peony {

class TabPage;
class SideBar;
class NavigationBar;
class ToolBar;
class SearchBar;
class StatusBar;

class DirectoryViewProxyIface;
class DirectoryViewContainer;

class FileInfo;

class PreviewPageIface;
class PreviewPageContainer;

/*!
 * \brief The FMWindow class, the normal window of explorer-qt's file manager.
 * \details
 * This class show the directory as the common window which used by explorer-qt.
 * It contains a tool bar, a navigation bar, a side bar, a preview page and
 * a status bar, and a tab widget container which may conatin over one
 * directory views.
 *
 * The tab view's design is improving and refactoring from explorer's window-pane-slot
 * window framework. There is no concept pane in explorer-qt (stand by preview page).
 * The window can hold over one 'slot' represent a directory view at a FMWindow
 * instance, but there should be only and just one slot is active in this window.
 *
 * \note
 * The tab view's design is not necessary for a file manager, and it might increased
 * the design difficulty. If you plan to develop a file manager application.
 * You should consider wether it is needed.
 */
class PEONYCORESHARED_EXPORT FMWindow : public QMainWindow, public FMWindowIface
{
    Q_OBJECT
public:
    explicit FMWindow(const QString &uri = nullptr, QWidget *parent = nullptr);

    /*!
     * \brief getFactory
     * \return
     * \deprecated
     */
    FMWindowFactory *getFactory() override;

    FMWindowIface * create(const QString &uri) override;
    FMWindowIface * create(const QStringList &uris) override;

    const QString getLastNonSearchUri() override {
        return m_last_non_search_location;
    }
    const QString getCurrentUri() override;
    const QStringList getCurrentSelections() override;
    const QStringList getCurrentAllFileUris() override;
    const QList<std::shared_ptr<FileInfo>> getCurrentSelectionFileInfos() override;
    DirectoryViewContainer *getCurrentPage() override;

    Qt::SortOrder getCurrentSortOrder() override;
    int getCurrentSortColumn() override;

    bool getWindowShowHidden() override {
        return m_show_hidden_file;
    }
    bool getWindowUseDefaultNameSortOrder() override {
        return m_use_default_name_sort_order;
    }
    bool getWindowSortFolderFirst() override {
        return m_folder_first;
    }

    const QString getCurrentPageViewType() override;

    QSize sizeHint() const override {
        return QSize(800, 600);
    }

Q_SIGNALS:
    void activeViewChanged(const DirectoryViewProxyIface *view);
    void tabPageChanged();

    /*!
     * \brief locationChangeStart
     * \details
     * This signal is used to tell the window doing a location change.
     * When a window is excuting a location change, it should not execute another
     * one util the location change finished.
     */
    void locationChangeStart();
    /*!
     * \brief endLocationChange
     * \details
     * This signal is used to tell the window that a location change finished.
     * Once a location change finished, we can start a new location change.
     */
    void locationChangeEnd();

    void windowSelectionChanged();

public Q_SLOTS:
    void goToUri(const QString &uri, bool addHistory, bool forceUpdate = false) override;
    void addNewTabs(const QStringList &uris) override;

    void beginSwitchView(const QString &viewId) override;

    void refresh() override;
    void forceStopLoading() override;
    void advanceSearch() override;
    void clearRecord() override;
    void searchFilter(QString target_path, QString keyWord, bool search_file_name, bool search_content) override;
    void filterUpdate(int type_index=0, int time_index=0, int size_index=0) override;

    void setShowHidden(bool showHidden) override;
    void setUseDefaultNameSortOrder(bool use) override;
    void setSortFolderFirst(bool set) override;

    void onPreviewPageSwitch(const QString &uri) override;

    void setCurrentSelectionUris(const QStringList &uris) override;
    void setCurrentSortOrder (Qt::SortOrder order) override;
    void setCurrentSortColumn (int sortColumn) override;

    void editUri(const QString &uri) override;
    void editUris(const QStringList &uris) override;

protected:
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

public:
    //advance search filter options
    bool m_update_condition = false;

private:
    QSplitter *m_splitter;

    TabPage *m_tab;
    SideBar *m_side_bar;
    NavigationBar *m_navigation_bar;
    ToolBar *m_tool_bar;
    SearchBar *m_search_bar;
    StatusBar *m_status_bar;

    QWidget *m_filter;
    AdvanceSearchBar *m_filter_bar;

    QPushButton *m_advanced_button;
    QPushButton *m_clear_record;

    QTimer m_operation_minimum_interval;
    bool m_is_loading = false;
    bool m_filter_visible = false;
    bool m_show_hidden_file;
    bool m_use_default_name_sort_order;
    bool m_folder_first;

    QString m_last_non_search_location;
    QString m_advance_target_path;

    QStackedWidget *m_side_bar_container = nullptr;
    PreviewPageContainer *m_preview_page_container = nullptr;
};

class PreviewPageContainer : public QStackedWidget
{
    friend class FMWindow;
    Q_OBJECT
    explicit PreviewPageContainer(QWidget *parent = nullptr);

    void setCurrentPage(PreviewPageIface *page);
    void removePage(PreviewPageIface *page);
    PreviewPageIface *getCurrentPage();
    bool isHidden() {
        return isVisible();
    }
};

class ToolBarContainer : public QToolBar
{
    friend class FMWindow;
    explicit ToolBarContainer(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e);
};

}

#endif // FMWINDOW_H
