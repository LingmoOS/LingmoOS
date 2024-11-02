/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef HEADERBAR_H
#define HEADERBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QProxyStyle>
#include <QMenuBar>

class MainWindow;
class ViewTypeMenu;
class SortTypeMenu;
class OperationMenu;

namespace Peony {
class SearchWidget;
}

class HeaderBar;
class QHBoxLayout;

class HeaderBarContainer : public QToolBar
{
    Q_OBJECT
public:
    explicit HeaderBarContainer(QWidget *parent = nullptr);

    bool eventFilter(QObject *obj, QEvent *e);

    void addHeaderBar(HeaderBar *headerBar);
    void addMenu(MainWindow *m_window);
    QWidget *m_topMenu = nullptr;

protected:
    void paintEvent(QPaintEvent *e);

private:
    QWidget *m_internal_widget;
    QHBoxLayout *m_layout;

    HeaderBar *m_header_bar = nullptr;

    QToolButton *m_max_or_restore = nullptr;

};

class HeaderBar : public QToolBar
{
    friend class HeaderBarContainer;
    friend class MainWindow;
    friend class TopMenuBar;
    Q_OBJECT
    enum HeaderBarAction {
        GoBack,
        GoForward,
        LocationBar,
        Search,
        ViewType,
        SortType,
        Option,
        Copy,
        Cut,
        SeletcAll,
        Delete,
        TabletSelectAll,  //task#106007 【文件管理器】文件管理器应用做平板UI适配，增加选项控件
        TabletSelectDone,
        TabletMoveTo,
        TabletCopyTo,
        TabletDelete,
        TabletMin,
        TabletClose,
        IconView,
        ListView
    };

public:
    void updatePreviewStatus(bool check);

private:
    explicit HeaderBar(MainWindow *parent = nullptr);
    ~HeaderBar();

Q_SIGNALS:
    void updateLocationRequest(const QString &uri, bool addHistory = true, bool force = true);
    void updateSearch(const QString &uri, const QString &key="", bool updateKey=false);
    void viewTypeChangeRequest(const QString &viewId);
    void updateZoomLevelHintRequest(int zoomLevelHint);
    void updateSearchRequest(bool showSearch);
//    void clearTrash();
    void refreshRequest();
    void updateFileTypeFilter(const int &index);
    void setGlobalFlag(bool isGlobal);
    void updateSearchRecursive(bool recursive);
    void closeSearch();
    void setLocation(const QString &uri);
    void cancelEdit();
    void startEdit(bool bSearch = false);
    void updateSearchProgress(bool searching);

protected:
    void addSpacing(int pixel);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void addTabletMenu();
    void updateSelectStatus(bool autoUpdate);

private Q_SLOTS:
    void updateIcons();
    void updateMaximizeState();
    void openDefaultTerminal();
    void findDefaultTerminal();
    void tryOpenAgain();
    void switchSelectStatus(bool select);
    void cancleSelect();
    void updateSortTypeEnable();
    void updateViewTypeEnable();
    void updatePreviewPageVisible();
    void updateTabletModeValue(bool isTabletMode);
    bool CopyOrMoveTo(bool isCut);
    void quitMultiSelect();
    void setSearchMode(bool isSearching);
    void quitSerachMode();
    void finishEdit();

private:
    const QString m_uri;
    MainWindow *m_window;
    Peony::SearchWidget *m_searchWidget;
    ViewTypeMenu *m_view_type_menu;
    SortTypeMenu *m_sort_type_menu;
    OperationMenu *m_operation_menu;

    QToolButton *m_create_folder;
    QToolButton *m_go_back;
    QToolButton *m_go_forward;
    QToolButton *m_go_up;
    QToolButton *m_search_button;

   // bool m_search_global = false;
    bool m_is_intel = false;
    bool m_tablet_mode = false;
//    bool m_isDone = false;
    bool m_isSelectAll = false;
    // save the actions to show or hide
    QHash<HeaderBarAction, QAction*> m_actions;

    QToolButton *m_maximize_restore_button;
    QAction *m_preview_action = nullptr;
    QActionGroup *m_view_actions;
};

class HeaderBarToolButton : public QToolButton
{
    friend class HeaderBar;
    friend class MainWindow;
    Q_OBJECT
    explicit HeaderBarToolButton(QWidget *parent = nullptr);
};

class HeadBarPushButton : public QPushButton
{
    friend class HeaderBar;
    friend class MainWindow;
    Q_OBJECT
    explicit HeadBarPushButton(QWidget *parent = nullptr);
};

class HeaderBarStyle : public QProxyStyle
{
    friend class HeaderBar;
    friend class HeaderBarContainer;
    static HeaderBarStyle *getStyle();

    HeaderBarStyle() {}

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};

class TopMenuBar : public QMenuBar
{
    Q_OBJECT
public:
    explicit TopMenuBar(HeaderBar *headerBar, MainWindow *parent = nullptr);

    bool eventFilter(QObject *obj, QEvent *e);

protected:
    void addWindowButtons();

private Q_SLOTS:
    void updateTabletMode(bool isTabletMode);

private:
    QWidget *m_top_menu_internal_widget = nullptr;
    QHBoxLayout *m_top_menu_layout = nullptr;
    MainWindow *m_window = nullptr;
    QToolButton *m_max_or_restore = nullptr;
    QToolButton *m_minimize = nullptr;
    QToolButton *m_close = nullptr;
    bool m_tablet_mode = false;

    HeaderBar *m_header_bar = nullptr;
};

#endif // HEADERBAR_H
