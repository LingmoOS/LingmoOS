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

#ifndef DEVELTABWIDGET_H
#define DEVELTABWIDGET_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QList>
#include <QLineEdit>
#include <QSignalMapper>
#include <QActionGroup>
#include "navigation-tab-bar.h"
#include "file-info.h"
#include "tab-status-bar.h"

class NavigationTabBar;
class QStackedWidget;
class PreviewPageButtonGroups;
class QHBoxLayout;
class QVBoxLayout;

namespace Peony {
class PreviewPageIface;
class DirectoryViewContainer;
class FileInfo;
}

/*!
 * \brief The TabWidget class
 * \details
 * TabWidget is different with QTabWidget, it contains 4 parts.
 * 1. tabbar
 * 2. stackwidget
 * 3. preview button group
 * 4. preview page
 * <br>
 * Tabbar and statck is simmilar to QTabWidget's. Preview page is a docked page
 * which can be added or removed by preview button group.
 */
class TabWidget : public QMainWindow
{
    friend class MainWindow;
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);

    QTabBar *tabBar();

    Peony::DirectoryViewContainer *currentPage();

    const QString getCurrentUri();
    const QStringList getCurrentSelections();

    const QStringList getAllFileUris();
    const QList<std::shared_ptr<Peony::FileInfo>> getCurrentSelectionFileInfos();
    /* Function:Get all file information under the current path */
    const QList<std::shared_ptr<Peony::FileInfo>> getCurrentAllFileInfos();

    const QStringList getBackList();
    const QStringList getForwardList();

    bool canGoBack();
    bool canGoForward();
    bool canCdUp();

    int getSortType();
    Qt::SortOrder getSortOrder();

    bool eventFilter(QObject *obj, QEvent *e);

Q_SIGNALS:
    void currentIndexChanged(int index);
    void tabMoved(int from, int to);
    void tabInserted(int index);
    void tabRemoved(int index);
    void searchRecursiveChanged(bool recursive);

    void activePageSelectionChanged();
    void activePageChanged();
    void activePageLocationChanged();
    void activePageViewTypeChanged();
    void activePageViewSortTypeChanged();

    void viewDoubleClicked(const QString &uri);
    void updateSearch(const QString &uri, const QString &key="", bool updateKey=false);
    void updateWindowLocationRequest(const QString &uri, bool addHistory, bool forceUpdate = false);
    void updateWindowSelectionRequest(const QStringList &uris);

    void menuRequest(const QPoint &pos);
    void zoomRequest(bool zoomIn);

    void closeWindowRequest();

    void clearTrash();
    void closeSearch();
    void recoverFromTrash();
    void currentSelectionChanged();
    void signal_itemAdded(const QString& uri);/* 新增文件（夹），item创建完成 */
    void updateItemsNum(); /*显示隐藏文件，更新项目个数*/

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setPreviewPage(Peony::PreviewPageIface *previewPage = nullptr);
    void addPage(const QString &uri, bool jumpTo = false);

    void goToUri(const QString &uri, bool addHistory, bool forceUpdate = false);
    void switchViewType(const QString &viewId);
    void updateTabPageTitle();

    void goBack();
    void goForward();
    void cdUp();

    void refresh();
    void stopLoading();

    void tryJump(int index);
    void clearHistory();

    void setSortType(int type);
    void setSortOrder(Qt::SortOrder order);

    void setSortFilter(int FileTypeIndex, int FileMTimeIndex, int FileSizeIndex);
    void setShowHidden(bool showHidden = false);
    void setUseDefaultNameSortOrder(bool use);
    void setSortFolderFirst(bool folderFirst);

    //multiple filter conditions for new advance search
    void addFilterCondition(int option, int classify, bool updateNow = false);
    void removeFilterCondition(int option, int classify, bool updateNow = false);
    void clearConditions();
    void updateFilter();
    void updateAdvanceConditions();

    void setCurrentSelections(const QStringList &uris);

    void editUri(const QString &uri);
    void editUris(const QStringList &uris);

    void onViewDoubleClicked(const QString &uri);
    bool getTriggeredPreviewPage() {
        return m_triggered_preview_page;
    }
    void setTriggeredPreviewPage(bool trigger) {
        m_triggered_preview_page = trigger;
    }

    int count();
    int currentIndex();
    void removeTab(int index);
    void updateTrashBarVisible(const QString &uri = "");
    void updateSearchPathButton(const QString &uri = "");
    void updateSearchBar(bool showSearch);
    void updateSearchList();
    void updateButtons();
    void addNewConditionBar();
    void removeConditionBar(int index);
    void searchUpdate();
    void searchChildUpdate();
    void browsePath();

    void handleZoomLevel(int zoomLevel);
    void enableSearchBar(bool enable);
    /* 当设备(U盘、硬盘、光盘、文件保护箱等)在一个或多个文件管理器中打开一个或多个tab页时，设备卸载/弹出/文件保护箱锁定后，
     * 属于该设备：文件管理器的当前标签页跳转到计算机页（保护箱标签除外），其余标签页均关闭；不属于该设备的tab页不处理 */
    void slot_responseUnmounted(const QString &destUri, const QString &sourceUri);

protected:
    void changeCurrentIndex(int index);
    void moveTab(int from, int to);

    void bindContainerSignal(Peony::DirectoryViewContainer *container);
    void updatePreviewPage();

    void resizeEvent(QResizeEvent *e);
    void updateTabBarGeometry();
    void updateStatusBarGeometry();

    void initAdvanceSearch();

    QStringList getCurrentClassify(int rowCount);

    void updatePreviewPageVisible();
    void updateStatusBarSliderState();
    void updatePreviewButtonStatus(bool status);
    void updateTrashButtonSize();
    bool isMultFile(std::shared_ptr<Peony::FileInfo> info);

private:
    NavigationTabBar *m_tab_bar;
    QToolButton *m_add_page_button;

    QWidget *m_tab_bar_bg;

    QStackedWidget *m_stack;

    PreviewPageButtonGroups *m_buttons;

    QToolBar *m_tool_bar;

    Peony::PreviewPageIface *m_preview_page = nullptr;
    QStackedWidget *m_preview_page_container;

    QAction *m_current_preview_action = nullptr;
    QAction *m_preview_action = nullptr;
    QActionGroup *m_preview_action_group = nullptr;

    QToolBar *m_trash_bar;
    QToolBar *m_search_bar;
    QVBoxLayout *m_top_layout;
    QHBoxLayout *m_trash_bar_layout;
    QHBoxLayout *m_search_bar_layout;
    QLabel *m_trash_label;
    QPushButton *m_clear_button;
    QPushButton *m_recover_button;
    QPushButton *m_search_path;
    QPushButton *m_search_close;
    QPushButton *m_search_child;
    QPushButton *m_search_more;
    QLabel *m_search_title;

    //use qlist for dynamic generated search conditions list
    QList<QHBoxLayout*> m_layout_list;
    QList<QComboBox*> m_conditions_list;
    QList<QComboBox*> m_classify_list;
    QList<QLabel*> m_link_label_list;
    QList<QPushButton*> m_add_button_list;
    QList<QPushButton*> m_remove_button_list;
    QList<QToolBar*> m_search_bar_list;
    QList<QLineEdit*> m_input_list;
    QList<QSignalMapper*> m_add_mappper_list;
    QList<QSignalMapper*> m_remove_mapper_list;

    int m_search_bar_count = 0;
    const int ELIDE_TEXT_LENGTH = 5;

    TabStatusBar *m_status_bar = nullptr;

    bool m_triggered_preview_page = false;
    bool m_show_search_list = false;
    bool m_show_search_bar = false;
    bool m_search_child_flag = false;

    //Button size macro definition
    //change height to 36 to ensure max size font can show complete, link to bug#58824
    const int TRASH_BUTTON_HEIGHT = 36;
    const int TRASH_BUTTON_WIDTH = 65;
    int m_trash_button_height = 36;
    int m_trash_button_width = 65;

    //advance search filter options
    QStringList m_option_list = {tr("type"), tr("file size"), tr("modify time"), tr("name")};
    QStringList m_file_type_list = {tr("all"), tr("file folder"), tr("image"), tr("video"),
                                    tr("text file"), tr("audio"), tr("wps file"), tr("others")};
    QStringList m_file_mtime_list = {tr("all"), tr("today"), tr("this week"), tr("this month"), tr("this year"), tr("year ago")};
    QStringList m_file_size_list = {tr("all"),tr("empty(0K)"), tr("tiny(0-16K)"), tr("small(16k-1M)"), tr("medium(1M-128M)"), tr("big(128M-1G)"),tr("large(1-4G)"),tr("great(>4G)")};

    bool m_first_add_page = true;

    std::shared_ptr<Peony::FileInfo> m_search_button_info;
};

class PreviewPageContainer : public QStackedWidget
{
    friend class TabWidget;
    Q_OBJECT
    explicit PreviewPageContainer(QWidget *parent = nullptr);
    //QSize sizeHint() const {return QSize(200, QStackedWidget::sizeHint().height());}
};

class PreviewPageButtonGroups : public QButtonGroup
{
    Q_OBJECT
public:
    explicit PreviewPageButtonGroups(QWidget *parent = nullptr);

Q_SIGNALS:
    void previewPageButtonTrigger(bool trigger, const QString &pluginId);
};

class PushButtonStyle : public QProxyStyle
{

public:
    static PushButtonStyle *getStyle();

    PushButtonStyle() : QProxyStyle() {}

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;

    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const;
};

#endif // DEVELTABWIDGET_H
