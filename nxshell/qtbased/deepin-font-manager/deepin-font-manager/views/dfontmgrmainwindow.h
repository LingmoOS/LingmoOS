// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTMGRMAINWINDOW_H
#define DFONTMGRMAINWINDOW_H

#include "dfontpreviewlistview.h"
#include "fontmanagercore.h"
#include "views/dfontspinnerwidget.h"
#include "signalmanager.h"
#include "dsplitlistwidget.h"
#include "views/dfdeletedialog.h"
#include "utils.h"
#include "getuseraddfontthread.h"

#include <DSearchEdit>
#include <DSlider>
#include <DHorizontalLine>
#include <DMainWindow>
#include <DFrame>
#include <DGuiApplicationHelper>

#include <QSettings>
#include <QDir>
#include <QShortcut>
#include <QScreen>

DWIDGET_USE_NAMESPACE

//类声明
class DFQuickInstallWindow;
class DFInstallNormalWindow;
class DFontMgrMainWindowPrivate;

class DFontMgrMainWindowPrivate
{
public:
    explicit DFontMgrMainWindowPrivate(DFontMgrMainWindow *q)
        : settingsQsPtr(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                                      QSettings::IniFormat))
        , q_ptr(q)
    {
    }

    //~DFontMgrMainWindowPrivate() {}
    QWidget *titleActionArea {nullptr};
    DIconButton *addFontButton {nullptr};
    DSearchEdit *searchFontEdit {nullptr};

    QWidget *fontShowArea {nullptr};

    //Shadow line of StateBar
    DHorizontalLine  *sbarShadowLine {nullptr};

    QWidget *stateBar {nullptr};
    DLineEdit *textInputEdit {nullptr};
    DSlider *fontScaleSlider {nullptr};
    DLabel *fontSizeLabel {nullptr};

    DSplitter *mainWndSpliter {nullptr};
    QWidget *leftBarHolder {nullptr};
    QWidget *rightViewHolder {nullptr};

    // Menu
    DMenu *toolBarMenu {nullptr};
    DMenu *rightKeyMenu {nullptr};

    DSplitListWidget *leftSiderBar {nullptr};

    QScopedPointer<QSettings> settingsQsPtr;
    DFontMgrMainWindow *q_ptr;

    Q_DECLARE_PUBLIC(DFontMgrMainWindow)
};


/*************************************************************************
 <Class>         DFontMgrMainWindow
 <Description>   mainwindow，承担了字体管理器主页面初始化、重绘、字体安装、删除、收藏/取消收藏、启用/禁用等功能。
 <Author>
 <Note>          null
*************************************************************************/
class DFontMgrMainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit DFontMgrMainWindow(bool isQuickMode = false, QWidget *parent = nullptr);
    ~DFontMgrMainWindow() override;

    QThread *m_thread;

    //静态变量
    static constexpr char const *FMT_FONT_SIZE = "%1px";
    static constexpr int MIN_FONT_SIZE = 6;
    static constexpr int MAX_FONT_SIZE = 60;
    static constexpr int DEFAULT_FONT_SIZE = FTM_DEFAULT_PREVIEW_FONTSIZE;

    /*************************************************************************
     <Enum>          DeleteStatus
     <Description>   删除状态
     <Author>
     <Value>
        <Value1>     UnDeleting                                      Description:不在删除
        <Value2>     Delete_Cacheing                                 Description:正在fc-cache执行
        <Value2>     Delete_Deleting                                 Description:正在删除
        <Value2>     Deleting = Delete_Cacheing | Delete_Deleting    Description:正在fc-cache执行或正在删除
     <Note>          null
    *************************************************************************/
    enum DeleteStatus {
        UnDeleting = 0,
        Delete_Cacheing = 1,
        Delete_Deleting = 2,
        Deleting = Delete_Cacheing | Delete_Deleting,
    };
    //通过styles来决定标签显示内容
    void showSpinner(DFontSpinnerWidget::SpinnerStyles styles, bool force = false);
    //设置快速安装标志位
    void setQuickInstallMode(bool isQuick);
    //隐藏快速安装页面
    void hideQucikInstallWindow();
    //快速安装
    void InitQuickWindowIfNeeded();
    //安装进程被打断
    void forceNoramlInstalltionQuitIfNeeded();
    //删除字体后恢复标志位
    void setDeleteFinish();
    //删除字体验证页面取消恢复标志位
    void cancelDelete();
    //获得DFontManager类的实例对象m_fontManager
    inline FontManagerCore *getFontManager()
    {
        return m_fontManager;
    }
    //获得DFontPreviewListView类的实例对象m_fontPreviewListView
    inline DFontPreviewListView *getPreviewListView()
    {
        return m_fontPreviewListView;
    }
    //获得删除标志位m_fIsDeleting状态
    inline qint8 isDeleting()
    {
        return m_fIsDeleting;
    }
    //获得当前菜单分组名
    inline DSplitListWidget::FontGroup currentFontGroup()
    {
        return filterGroup;
    }
    //加载完成后停止加载图标标签
    inline void stopLoading()
    {
        m_fontLoadingSpinner->spinnerStop();
        m_fontLoadingSpinner->hide();
    }
    //获取高度
    inline int getWinHeight()
    {
        return m_winHight;
    }
    //获取宽度
    inline int getWinWidth()
    {
        return m_winWidth;
    }
    //获取是否最大化
    inline bool getIsMaximized()
    {
        return m_IsWindowMax;
    }


    //安装完成后切换至用户字体页面
    void showInstalledFiles();
protected:
    //初始化应用数据
    void initData();
    //页面初始化入口
    void initUI();
    //初始化信号和槽的连接函数
    void initConnections();
    //初始化快捷键和处理函数
    void initShortcuts();
    //初始化titleBar
    void initTileBar();
    //初始化标题栏
    void initTileFrame();
    //初始化主页面
    void initMainVeiws();
    //初始化页面左侧菜单部分
    void initLeftSideBar();
    //初始化页面右部分
    void initRightFontView();
    //初始化状态栏-包含预览输入框和字体大小调节滑块和字体大小标签
    void initStateBar();
    //初始化右键菜单
    void initRightKeyMenu();
    //初始化字体预览页面ListView
    void initFontPreviewListView(QWidget *parent);
    //SP3--设置tab顺序--安装事件过滤器
    void installEventFilters();//SP3--设置tab顺序(539)
    //添加字体文管页面处理函数
    void handleAddFontEvent();
    //安装字体入口函数
    bool installFont(const QStringList &files, bool isAddBtnHasTabs);
    //从系统中加载字体
    void installFontFromSys(const QStringList &files);
    //右键菜单：在文件管理器中显示字体文件位置
    void showFontFilePostion();
    //字体删除处理函数
    void delCurrentFont(bool activatedByRightmenu = true);
    //字体导出处理函数
    void exportFont();
    //显示导出字体时提示内容
    void showExportFontMessage(int successCount, int abandonFilesCount);
    //调用显示快捷键的说明页面
    void showAllShortcut();
    //判断是否需要隐藏旋转进度图标
    void hideSpinner();

    void afterAllStartup();

    //更新待安装列表
    void waitForInsert();
    //根据输入内容调整预览字体
    void onPreviewTextChanged();
    //获得硬盘剩余空间
    qint64 getDiskSpace(bool m_bInstall = true);
    //检查空间盈余
    QStringList checkFilesSpace(const QStringList &files, bool m_bInstall = true);
    //对主窗口中的focusout事件进行检查后对相关控件标志位等进行处理
    void mainwindowFocusOutCheck(QObject *obj, QEvent *event);
    //对主窗口中的focusin事件进行检查后对相关控件标志位等进行处理
    void mainwindowFocusInCheck(QObject *obj, QEvent *event);
    //调节右下角字体大小显示label显示内容
    void autoLabelWidth(QString text, DLabel *lab, QFontMetrics fm);
    //设置tab聚焦顺序
    void setNextTabFocus(QObject *obj);//SP3--设置tab顺序(539)
    //响应字体大小滑块大小改变
    void respondToValueChanged(int value);
    //响应安装结束
    void respondToInstallOver(int successInstallCount);

    //处理拖放事件
    void dropEvent(QDropEvent *event) override;
    //处理键盘press事件
    void keyPressEvent(QKeyEvent *event)override;
    //处理拖入事件
    void dragEnterEvent(QDragEnterEvent *event) override;
    //处理窗口大小变化事件
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event)Q_DECL_OVERRIDE;
    //事件过滤器：用于过滤事件
    bool eventFilter(QObject *obj, QEvent *event)Q_DECL_OVERRIDE;//SP3--设置tab顺序(539)


signals:
    //信号-文管中选中字体打开
    void fileSelected(const QStringList &files, bool isAddBtnHasTabs) const;
    //信号-文管中选中系统字体打开
    void fileSelectedInSys(const QStringList &files) const;
    //信号-设置正常安装模式
    void quickModeInstall(const QStringList &files) const;
    //信号-加载启动过程中未加载的字体完成
    void singalLoadLeftFontsFinsih();

public slots:
    //右键菜单弹出处理函数
    void handleMenuEvent(QAction *);
    //根据搜索框输入内容刷新搜索结果
    void onSearchTextChanged(const QString &currStr);
    //刷新预览内容
    void onPreviewTextChanged(const QString &text);
    //更新预览页面字号大小
    void onFontSizeChanged(int fontSize);
    //左侧菜单项切换选项
    void onLeftSiderBarItemClicked(int index = 0, bool needClearSelect = true);
    //安装后添加至listview
    void onFontInstallFinished(const QStringList &fileList);
    //刷新用户字体列表
    void onRefreshUserFont();
    //字体删除fc-cache操作后恢复标志位
    void onUninstallFcCacheFinish();
    //切换字体菜单后判断FontListView的结果并显示对应状态
    void onFontListViewRowCountChanged();
    //刷新加载状态
    void onLoadStatus(int type);
    //安装字体提示信息
    void onShowMessage(int totalCount, bool success=true);
    //显示或停止旋转进度图标
    void onShowSpinner(bool bShow, bool force, DFontSpinnerWidget::SpinnerStyles style);
    //删除窗口确认删除槽函数
    void onconfirmDelDlgAccept();
    //安装窗口销毁槽函数
    void onInstallWindowDestroyed(QObject * = nullptr);
    //右键菜单即将显示槽函数
    void onRightMenuAboutToShow();
    //右键菜单即将消失槽函数
    void onRightMenuAboutToHide();
    //卸载字体结束槽函数
    void onUninstallFontFinished(QStringList &files);
    //fc-cache执行结束槽函数
    void onCacheFinish();
    //删除ttc文件确认函数
    void onHandleDeleteTTC(QString filePath, bool &isDelete, bool &isAapplyToAll);
    //ui界面操作结束槽函数
    void onRequestInstFontsUiAdded();
    //系统字体变化槽函数
    void onFontChanged();
    //设置显示的字体大小槽函数
    void onFontSizeRequestToSlider();
    //导出结束槽函数
    void onExportFontFinished(int count);
    //右键菜单消失槽函数
    void onMenuHidden();
    //fontmananger线程结束槽函数
    void onFontManagerFinished();
    //禁用/启用ttc里的某个字体
    void onHandleDisableTTC(const QString &filePath, bool &isEnable, bool &isConfirm, bool &isAapplyToAll);
#ifdef DTKWIDGET_CLASS_DSizeMode
    //紧凑模式切换
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

protected:
    DFontPreviewListView *m_fontPreviewListView;
    DListView *m_noResultListView;
    DListView *m_noInstallListView;
    DFontSpinnerWidget *m_fontLoadingSpinner {nullptr};
    DFMDBManager *m_dbManager {nullptr};
    FontManagerCore *m_fontManager;
    SignalManager *m_signalManager = SignalManager::instance();
    QShortcut *m_scFullScreen;  //全屏快捷键F11
    QShortcut *m_scZoomIn {nullptr};     //放大字体快捷键Ctrl+=
    QShortcut *m_scZoomOut;       //放大字体快捷键Ctrl+-
    QShortcut *m_scDefaultSize;   //默认⼤⼩字体快捷键Ctrl+0
    QString m_previewText;
    // For quick install mode
    bool m_isQuickMode = {false};
    qint8 m_previewFontSize;
    qint8 m_leftIndex {0};
    //selected fonts
    qint8 m_menuCurCnt;
    int m_menuDelCnt;
    int m_menuDisableSysCnt;
    int m_menuSysCnt;
    int m_menuDisableCnt;
    QStringList m_menuDelFontList;
    QModelIndexList m_menuAllIndexList;
    QModelIndexList m_menuDisableIndexList;
    QStringList m_menuAllMinusSysFontList;
    QStringList m_installOutFileList;

    DFontPreviewItemData m_menuCurData;
    DSplitListWidget::FontGroup filterGroup;
    //Main window Size
    short m_winHight;
    short m_winWidth;
    bool m_IsWindowMax = false;

    bool m_searchTextStatusIsEmpty = true;
    bool m_isFromSys = false;
    bool m_isPopInstallErrorDialog = false;
    bool m_isNoResultViewShow = false;
    bool m_isInstallOver = false;
    bool m_openfirst = true;//是否是第一次打开软件
    bool m_cacheFinish = false;
    bool m_installFinish = false;

    bool m_leftListViewTabFocus = false;
    bool m_previewListViewTabFocus = false;
    bool m_needWaitThreadStop = false;

    bool m_isSearchLineEditMenuPoped{false};
    bool m_isInputLineEditMunuPoped{false};
    bool m_lastIsCloseBtn {false};
    bool m_focusInMenu {false};
    //菜单关闭的原因是否为触发action
    bool m_hasMenuTriggered{false};
    //is in installing font flow
    //Avoid start multi-NormalInstalltion window
    bool m_fIsInstalling {false};
    bool m_bLoadLeftFontsFinsihFlag = false; //是否已加载启动过程中未加载的字体
    //is it in uninstalling font flow
    //Avoid start multi delete confirm dialog
    volatile qint8 m_fIsDeleting {UnDeleting};

    FocusStatus m_currentStatus;
    qint64 m_installTm;

    //Stand shortcut
    //Implement by DTK                       //Close window       --> Alt+F4
    QShortcut *m_scShowAllSC     {nullptr};  //Show shortcut      --> Ctrl+Shift+/
    QShortcut *m_scPageUp        {nullptr};  //Show previous page --> PageUp
    QShortcut *m_scPageDown      {nullptr};  //Show next page     --> PageDown
    QShortcut *m_scWndReize      {nullptr};  //Resize Window      --> Ctrl+Alt+F
    QShortcut *m_scFindFont      {nullptr};  //Find font          --> Ctrl+F
    QShortcut *m_scDeleteFont    {nullptr};  //Delete font        --> Delete
    QShortcut *m_scAddNewFont    {nullptr};  //Add Font           --> Ctrl+O
    QShortcut *m_scFontInfo      {nullptr};  //Font information   --> CTRL+I
    QShortcut *m_scAddOrCancelFavFont   {nullptr};  //Add or cancel favorite    -->.
    QShortcut *m_scShowMenu      {nullptr};  //ShowMenu           -->Alt+M//SP3--Alt+M右键菜单

    QStringList m_waitForInstall;

    DFInstallNormalWindow  *m_dfNormalInstalldlg {nullptr};
    QScopedPointer<DFQuickInstallWindow> m_quickInstallWnd;

    QScopedPointer<DFontMgrMainWindowPrivate> m_ptr;
    QString mhistoryDir = ""; //保存上次文件的路径的文件夹

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(m_ptr), DFontMgrMainWindow)
};

#endif  // DFONTMGRMAINWINDOW_H
