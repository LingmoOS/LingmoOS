// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWEDFRAME_H
#define WINDOWEDFRAME_H

#include "launcherinterface.h"
#include "sharedeventfilter.h"
#include "view/applistview.h"
#include "model/appslistmodel.h"
#include "model/appsmanager.h"
#include "worker/menuworker.h"
#include "delegate/applistdelegate.h"
#include "widgets/searchlineedit.h"
#include "widgets/miniframerightbar.h"
#include "widgets/miniframeswitchbtn.h"
#include "global_util/constants.h"
#include "global_util/calculate_util.h"
#include "miniframebutton.h"
#include "appgridview.h"
#include "searchmodewidget.h"
#include "sortfilterproxymodel.h"
#include "modeswitch.h"

#include "appearance_interface.h"

#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <DSearchEdit>
#include <DToolButton>
#include <dblureffectwidget.h>
#include <dregionmonitor.h>

#include <memory>

DWIDGET_USE_NAMESPACE

using Appearance = org::deepin::dde::Appearance1;
class QLabel;

class AMDBusDockInter;

class WindowedFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit WindowedFrame(QWidget *parent = nullptr);

    enum DisplayMode
    {
        Category,
        All
    };

    enum AnchoredCornor
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Normal
    };

    enum FocusPosition {
        CategoryApp,
        Power,
        Setting,
        Search,
        Favorite,
        AllApp,
        Switch,
        Default
    };

signals:
    void visibleChanged(bool visible);
    void searchApp(const QString &keyword = "");

private:
    void initUi();
    void initConnection();
    void setAccessibleName();
    void searchAppState(bool searched = false);
    void updateFavorateViewHeight(bool searched);

    void showLauncher() override;
    void hideLauncher() override;
    bool visible() override;

    void moveCurrentSelectApp(const int key) override;
    void appendToSearchEdit(const char ch) override;
    void launchCurrentApp() override;

    void handleDefault(const int key);
    void handleCategoryApp(const int key);
    void handlePower(const int key);
    void handleSetting(const int key);
    void handleSearch(const int key);
    void handleFavorite(const int key);
    void handleAllApp(const int key);
    void handleSwitch(const int key);

    void uninstallApp(const QString &desktopPath) override;

    QPainterPath getCornerPath(AnchoredCornor direction);

    void resetWidgetStyle();
    bool searchState() const;
    void setSearchState(bool searching);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void inputMethodEvent(QInputMethodEvent *e) Q_DECL_OVERRIDE;
    QVariant inputMethodQuery(Qt::InputMethodQuery prop) const Q_DECL_OVERRIDE;
    void regionMonitorPoint(const QPoint &point, int flag = 1) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

private slots:
    void initAnchoredCornor();
    void adjustPosition();
    void onToggleFullScreen();
    void onWMCompositeChanged();
    void searchText(const QString &text);
    void showTips(const QString &text);
    void hideTips();
    void prepareHideLauncher();
    void recoveryAll();
    void onOpacityChanged(const double value);
    void onScreenInfoChange();
    void updatePosition();
    void onHideMenu();
    void addViewEvent(AppGridView *pView);
    void onButtonClick(int buttonid);
    void onFavoriteListVisibleChaged();
    void onLayoutChanged();
    void onEnterView();
    void onHandleHoverAction(const QModelIndex &index);

private:
    AMDBusDockInter *m_amDbusDockInter;
    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;
    DPlatformWindowHandle m_windowHandle;
    DWindowManagerHelper *m_wmHelper;

    QWidget *m_maskBg;
    AppsManager *m_appsManager;

    AppsListModel *m_appsModel;
    AppsListModel *m_allAppsModel;
    AppsListModel *m_favoriteModel;
    SortFilterProxyModel *m_filterModel;
    SearchModeWidget *m_searchWidget;

    QWidget *m_rightWidget;
    MiniFrameRightBar *m_bottomBtn;
    AppListView *m_appsView;
    AppGridView *m_favoriteView;
    AppGridView *m_allAppView;

    AppItemDelegate *m_appItemDelegate;

    QLabel *m_favoriteLabel;
    QWidget *m_emptyFavoriteWidget;
    DIconButton *m_emptyFavoriteButton;
    QLabel *m_emptyFavoriteText;

    QLabel *m_allAppLabel;
    QLabel *m_tipsLabel;
    QTimer *m_delayHideTimer;
    QTimer *m_autoScrollTimer;
    Appearance *m_appearanceInter;
    DisplayMode m_displayMode;

    int m_autoScrollStep;
    int m_radius = 0;
    CalculateUtil *m_calcUtil;
    AnchoredCornor m_anchoredCornor = Normal;
    QPainterPath m_cornerPath;
    FocusPosition m_focusPos;
    DIconButton *m_modeToggleBtn;        // 小窗口右上角窗口模式切换按钮
    DSearchEdit *m_searcherEdit;         // 搜索控件
    bool m_enterSearchEdit;
    const QScreen *m_curScreen;
    ModeSwitch *m_modeSwitch;
    bool m_isSearching;
};

#endif // WINDOWEDFRAME_H
