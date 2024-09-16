// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALSTATUS_H
#define GLOBALSTATUS_H

#include "types.h"

#include <QObject>

class FileControl;
class GlobalStatus : public QObject
{
    Q_OBJECT
    // Constant properties.
    Q_PROPERTY(int minHeight READ minHeight CONSTANT)
    Q_PROPERTY(int minWidth READ minWidth CONSTANT)
    Q_PROPERTY(int minHideHeight READ minHideHeight CONSTANT)
    Q_PROPERTY(int floatMargin READ floatMargin CONSTANT)
    Q_PROPERTY(int titleHeight READ titleHeight CONSTANT)
    Q_PROPERTY(int thumbnailViewHeight READ thumbnailViewHeight CONSTANT)
    Q_PROPERTY(int showBottomY READ showBottomY CONSTANT)
    Q_PROPERTY(int switchImageHotspotWidth READ switchImageHotspotWidth CONSTANT)
    Q_PROPERTY(int actionMargin READ actionMargin CONSTANT)
    Q_PROPERTY(int rightMenuItemHeight READ rightMenuItemHeight CONSTANT)
    Q_PROPERTY(double animationDefaultDuration READ animationDefaultDuration CONSTANT)
    Q_PROPERTY(int pathViewItemCount READ pathViewItemCount CONSTANT)

    Q_PROPERTY(int rightMenuSeparatorHeight READ rightMenuSeparatorHeight CONSTANT)
    Q_PROPERTY(int needHideSideBarWidth READ needHideSideBarWidth CONSTANT)
    Q_PROPERTY(int sideBarWidth READ sideBarWidth CONSTANT)
    Q_PROPERTY(int statusBarHeight READ statusBarHeight CONSTANT)
    Q_PROPERTY(int collectionTopMargin READ collectionTopMargin CONSTANT)
    Q_PROPERTY(int thumbnailViewTitleHieght READ thumbnailViewTitleHieght CONSTANT)
    Q_PROPERTY(int verticalScrollBarWidth READ verticalScrollBarWidth CONSTANT)
    Q_PROPERTY(int rectSelScrollStep READ rectSelScrollStep CONSTANT)
    Q_PROPERTY(int thumbnailListRightMargin READ thumbnailListRightMargin CONSTANT)
    Q_PROPERTY(int thumbnialListCellSpace READ thumbnialListCellSpace CONSTANT)
    Q_PROPERTY(int animationDuration READ animationDuration CONSTANT)
    Q_PROPERTY(int largeImagePreviewAnimationDuration READ largeImagePreviewAnimationDuration CONSTANT)

public:
    explicit GlobalStatus(QObject *parent = nullptr);
    ~GlobalStatus() override;

    // 切换全屏显示图片 (ImageViewer)
    Q_PROPERTY(bool showFullScreen READ showFullScreen WRITE setShowFullScreen NOTIFY showFullScreenChanged)
    bool showFullScreen() const;
    void setShowFullScreen(bool value);
    Q_SIGNAL void showFullScreenChanged();

    // 允许显示导航窗口
    Q_PROPERTY(bool enableNavigation READ enableNavigation WRITE setEnableNavigation NOTIFY enableNavigationChanged)
    bool enableNavigation() const;
    void setEnableNavigation(bool value);
    Q_SIGNAL void enableNavigationChanged();

    // 显示右键菜单
    Q_PROPERTY(bool showRightMenu READ showRightMenu WRITE setShowRightMenu NOTIFY showRightMenuChanged)
    bool showRightMenu() const;
    void setShowRightMenu(bool value);
    Q_SIGNAL void showRightMenuChanged();

    // 显示详细图像信息
    Q_PROPERTY(bool showImageInfo READ showImageInfo WRITE setShowImageInfo NOTIFY showImageInfoChanged)
    bool showImageInfo() const;
    void setShowImageInfo(bool value);
    Q_SIGNAL void showImageInfoChanged();

    // 滑动视图是否响应操作 (ImageViewer ListView)
    Q_PROPERTY(bool viewInteractive READ viewInteractive WRITE setViewInteractive NOTIFY viewInteractiveChanged)
    bool viewInteractive() const;
    void setViewInteractive(bool value);
    Q_SIGNAL void viewInteractiveChanged();

    // 滑动视图是否处于轻弹状态 (ImageViewer ListView)
    Q_PROPERTY(bool viewFlicking READ viewFlicking WRITE setViewFlicking NOTIFY viewFlickingChanged) bool viewFlicking() const;
    void setViewFlicking(bool value);
    Q_SIGNAL void viewFlickingChanged();

    // 屏蔽标题栏/底部栏动画效果
    Q_PROPERTY(bool animationBlock READ animationBlock WRITE setAnimationBlock NOTIFY animationBlockChanged)
    bool animationBlock() const;
    void setAnimationBlock(bool value);
    Q_SIGNAL void animationBlockChanged();

    // 处于全屏动画状态标识，动画前后部分控件需重置，例如缩略图栏重新居中设置
    Q_PROPERTY(bool fullScreenAnimating READ fullScreenAnimating WRITE setFullScreenAnimating NOTIFY fullScreenAnimatingChanged)
    bool fullScreenAnimating() const;
    void setFullScreenAnimating(bool value);
    Q_SIGNAL void fullScreenAnimatingChanged();

    // 缩略图列表允许的宽度
    Q_PROPERTY(int thumbnailVaildWidth READ thumbnailVaildWidth WRITE setThumbnailVaildWidth NOTIFY thumbnailVaildWidthChanged)
    int thumbnailVaildWidth() const;
    void setThumbnailVaildWidth(int value);
    Q_SIGNAL void thumbnailVaildWidthChanged();

    // 当前所处的界面索引
    Q_PROPERTY(Types::StackPage stackPage READ stackPage WRITE setStackPage NOTIFY stackPageChanged)
    Types::StackPage stackPage() const;
    void setStackPage(Types::StackPage value);
    Q_SIGNAL void stackPageChanged();

    // 显示导出窗口
    Q_PROPERTY(bool showExportDialog READ showExportDialog WRITE setShowExportDialog NOTIFY showExportDialogChanged)
    bool showExportDialog() const;
    void setShowExportDialog(bool value);
    Q_SIGNAL void showExportDialogChanged();

    // 相册相关接口

    void setFileControl(FileControl* fc);

    // 侧边栏X坐标
    Q_PROPERTY(qreal sideBarX READ sideBarX WRITE setSideBarX NOTIFY sideBarXChanged)
    qreal sideBarX() const;
    void setSideBarX(const qreal& value);
    Q_SIGNAL void sideBarXChanged();

    // 已选路径
    Q_PROPERTY(QVariantList selectedPaths READ selectedPaths WRITE setSelectedPaths NOTIFY selectedPathsChanged)
    QVariantList selectedPaths() const;
    void setSelectedPaths(const QVariantList& value);
    Q_SIGNAL void selectedPathsChanged();

    // 刷新收藏图标
    Q_PROPERTY(bool bRefreshFavoriteIconFlag READ bRefreshFavoriteIconFlag WRITE setBRefreshFavoriteIconFlag NOTIFY bRefreshFavoriteIconFlagChanged)
    bool bRefreshFavoriteIconFlag() const;
    void setBRefreshFavoriteIconFlag(const bool& value);
    Q_SIGNAL void bRefreshFavoriteIconFlagChanged();

    // 刷新显示比例图标激活状态
    Q_PROPERTY(bool refreshRangeBtnState READ refreshRangeBtnState WRITE setRefreshRangeBtnState NOTIFY refreshRangeBtnStateChanged)
    bool refreshRangeBtnState() const;
    void setRefreshRangeBtnState(const bool& value);
    Q_SIGNAL void refreshRangeBtnStateChanged();

    // 当前视图类型
    Q_PROPERTY(Types::ThumbnailViewType currentViewType READ currentViewType WRITE setCurrentViewType NOTIFY currentViewTypeChanged)
    Types::ThumbnailViewType currentViewType() const;
    void setCurrentViewType(const Types::ThumbnailViewType& value);
    Q_SIGNAL void currentViewTypeChanged();

    // 合集当前索引
    Q_PROPERTY(int currentCollecttionViewIndex READ currentCollecttionViewIndex WRITE setCurrentCollecttionViewIndex NOTIFY currentCollecttionViewIndexChanged)
    int currentCollecttionViewIndex() const;
    void setCurrentCollecttionViewIndex(const int& value);
    Q_SIGNAL void currentCollecttionViewIndexChanged();

    // 合集当前动画切换类型
    Q_PROPERTY(Types::SwitchType currentSwitchType READ currentSwitchType WRITE setCurrentSwitchType NOTIFY currentSwitchTypeChanged)
    Types::SwitchType currentSwitchType() const;
    void setCurrentSwitchType(const int& value);
    Q_SIGNAL void currentSwitchTypeChanged();

    // 当前自定义相册Id
    Q_PROPERTY(int currentCustomAlbumUId READ currentCustomAlbumUId WRITE setCurrentCustomAlbumUId NOTIFY currentCustomAlbumUIdChanged)
    int currentCustomAlbumUId() const;
    void setCurrentCustomAlbumUId(const int& value);
    Q_SIGNAL void currentCustomAlbumUIdChanged();

    // 当前显示的主界面内容， 0:相册界面 1:看图界面 2:幻灯片
    Q_PROPERTY(int stackControlCurrent READ stackControlCurrent WRITE setStackControlCurrent NOTIFY stackControlCurrentChanged)
    int stackControlCurrent() const;
    void setStackControlCurrent(const int& value);
    Q_SIGNAL void stackControlCurrentChanged();

    // 记录上一次显示主界面的索引
    Q_PROPERTY(int stackControlLastCurrent READ stackControlLastCurrent WRITE setStackControlLastCurrent NOTIFY stackControlLastCurrentChanged)
    int stackControlLastCurrent() const;
    void setStackControlLastCurrent(const int& value);
    Q_SIGNAL void stackControlLastCurrentChanged();

    // 缩略图缩放等级
    Q_PROPERTY(int thumbnailSizeLevel READ thumbnailSizeLevel WRITE setThumbnailSizeLevel NOTIFY thumbnailSizeLevelChanged)
    int thumbnailSizeLevel() const;
    void setThumbnailSizeLevel(const int& value);
    Q_SIGNAL void thumbnailSizeLevelChanged();

    // 缩略图网格大小
    Q_PROPERTY(qreal cellBaseWidth READ cellBaseWidth WRITE setCellBaseWidth NOTIFY cellBaseWidthChanged)
    qreal cellBaseWidth() const;
    void setCellBaseWidth(const qreal& value);
    Q_SIGNAL void cellBaseWidthChanged();

    // 状态栏显示的总数文本内容
    Q_PROPERTY(QString statusBarNumText READ statusBarNumText WRITE setStatusBarNumText NOTIFY statusBarNumTextChanged)
    QString statusBarNumText() const;
    void setStatusBarNumText(const QString& value);
    Q_SIGNAL void statusBarNumTextChanged();

    // 搜索框文本内容
    Q_PROPERTY(QString searchEditText READ searchEditText WRITE setSearchEditText NOTIFY searchEditTextChanged)
    QString searchEditText() const;
    void setSearchEditText(const QString& value);
    Q_SIGNAL void searchEditTextChanged();

    // 自动导入相册改变标记
    Q_PROPERTY(bool albumImportChangeList READ albumImportChangeList WRITE setAlbumImportChangeList NOTIFY albumImportChangeListChanged)
    bool albumImportChangeList() const;
    void setAlbumImportChangeList(const bool& value);
    Q_SIGNAL void albumImportChangeListChanged();

    // 自定义相册改变标记
    Q_PROPERTY(bool albumChangeList READ albumChangeList WRITE setAlbumChangeList NOTIFY albumChangeListChanged)
    bool albumChangeList() const;
    void setAlbumChangeList(const bool& value);
    Q_SIGNAL void albumChangeListChanged();

    // 自定义相册改变标记
    Q_PROPERTY(bool sideBarIsVisible READ sideBarIsVisible WRITE setSideBarIsVisible NOTIFY sideBarIsVisibleChanged)
    bool sideBarIsVisible() const;
    void setSideBarIsVisible(const bool& value);
    Q_SIGNAL void sideBarIsVisibleChanged();

    // 当前设备名称
    Q_PROPERTY(QString currentDeviceName READ currentDeviceName WRITE setCurrentDeviceName NOTIFY currentDeviceNameChanged)
    QString currentDeviceName() const;
    void setCurrentDeviceName(const QString& value);
    Q_SIGNAL void currentDeviceNameChanged();

    // 当前设备路径
    Q_PROPERTY(QString currentDevicePath READ currentDevicePath WRITE setCurrentDevicePath NOTIFY currentDevicePathChanged)
    QString currentDevicePath() const;
    void setCurrentDevicePath(const QString& value);
    Q_SIGNAL void currentDevicePathChanged();

    // 窗口是否激活标记
    Q_PROPERTY(bool windowDisactived READ windowDisactived WRITE setWindowDisactived NOTIFY windowDisactivedChanged)
    bool windowDisactived() const;
    void setWindowDisactived(const bool& value);
    Q_SIGNAL void windowDisactivedChanged();

    // 记录相册界面是否处于加载状态
    Q_PROPERTY(bool loading READ loading WRITE setLoading NOTIFY loadingChanged)
    bool loading() const;
    void setLoading(const bool& value);
    Q_SIGNAL void loadingChanged();

    // 比例动画使能
    Q_PROPERTY(bool enableRatioAnimation READ enableRatioAnimation WRITE setEnableRatioAnimation NOTIFY enableRatioAnimationChanged)
    bool enableRatioAnimation() const;
    void setEnableRatioAnimation(const bool& value);
    Q_SIGNAL void enableRatioAnimationChanged();

    // 淡入淡出动画使能
    Q_PROPERTY(bool enableFadeInoutAnimation READ enableFadeInoutAnimation WRITE setEnableFadeInoutAnimation NOTIFY enableFadeInoutAnimationChanged)
    bool enableFadeInoutAnimation() const;
    void setEnableFadeInoutAnimation(const bool& value);
    Q_SIGNAL void enableFadeInoutAnimationChanged();

    // 从大图返回到相册
    Q_PROPERTY(bool enteringImageViewer READ enteringImageViewer WRITE setEnteringImageViewer NOTIFY enteringImageViewerChanged)
    bool enteringImageViewer() const;
    void setEnteringImageViewer(const bool& value);
    Q_SIGNAL void enteringImageViewerChanged();

    // 从大图返回到相册
    Q_PROPERTY(bool backingToMainAlbumView READ backingToMainAlbumView WRITE setBackingToMainAlbumView NOTIFY backingToMainAlbumViewChanged)
    bool backingToMainAlbumView() const;
    void setBackingToMainAlbumView(const bool& value);
    Q_SIGNAL void backingToMainAlbumViewChanged();

    // 根据选中内容获取状态栏文本内容
    Q_INVOKABLE QString getSelectedNumText(const QStringList &paths, const QString &text);

    // Constant properties.
    int minHeight() const;
    int minWidth() const;
    int minHideHeight() const;
    int floatMargin() const;
    int titleHeight() const;
    int thumbnailViewHeight() const;
    int showBottomY() const;
    int switchImageHotspotWidth() const;
    int actionMargin() const;
    int rightMenuItemHeight() const;

    double animationDefaultDuration() const;
    int pathViewItemCount() const;

    int rightMenuSeparatorHeight() const;

    int needHideSideBarWidth() const;
    int sideBarWidth() const;
    int statusBarHeight() const;
    int collectionTopMargin() const;
    int thumbnailViewTitleHieght() const;
    int verticalScrollBarWidth() const;
    int rectSelScrollStep() const;
    int thumbnailListRightMargin() const;
    int thumbnialListCellSpace() const;
    int animationDuration() const;
    int largeImagePreviewAnimationDuration() const;

// 相册相关信号
Q_SIGNALS:
    void sigThumbnailStateChange();
    void sigRunSearch(int UID, QString keywords);           // 执行搜索
    void sigFlushAllCollectionView();                       // 刷新合集所有项目视图内容
    void sigFlushHaveImportedView();                        // 刷新已导入视图内容
    void sigFlushRecentDelView();                           // 刷新最近删除视图内容
    void sigFlushCustomAlbumView(int UID);                  // 刷新我的收藏/自定义相册视图内容 UID: >= 0 刷新指定视图，-1: 默认刷新所有视图
    void sigFlushSearchView();                              // 刷新搜索结果视图内容
    void sigCustomAlbumNameChaged(int UID, QString name);   // 刷新自定义相册视图的相册名称
    void sigSelectAll(bool bSel);
    void sigPageUp();
    void sigPageDown();
    void sigMoveCenter(int x, int y, int w, int h);
    void sigShowToolBar();
    void sigMoveToAlbumAnimation();

private:
    void initConnect();

private:
    bool storeshowFullScreen = false;
    bool storeenableNavigation = true;
    bool storeshowRightMenu = false;
    bool storeshowImageInfo = false;
    bool storeviewInteractive = true;
    bool storeviewFlicking = false;
    bool storeanimationBlock = false;
    bool storefullScreenAnimating = false;
    int storethumbnailVaildWidth = 0;
    bool storeshowExportDialog = false;
    Types::StackPage storestackPage = Types::OpenImagePage;

    // 相册相关成员变量
    qreal m_sideBar_X = 0;
    QVariantList m_selectedPaths = QVariantList();// 已选路径

    bool m_bRefreshFavoriteIconFlag = false;    // 刷新收藏图标标记，翻转一次，前端图标就刷新一次
    bool m_bRefreshRangeBtnState = false;       // 刷新显示比例图标激活状态标记，翻转一次，前端图标就刷新一次
    Types::ThumbnailViewType m_currentViewType = Types::ViewImport; // 当前显示的视图类型
    int m_currentCollecttionViewIndex = 3;      // 合集当前索引 0:年 1:月 2:日 3:所有图片
    Types::SwitchType m_currentSwitchType = Types::UnknownSwitchType;
    int m_currentCustomAlbumUId = -1;           // 当前自定义相册所在UId，-1：照片库(非我的收藏) 0:我的收藏 1:截图录屏 2:相机 3:画板 其他:自定义相册
    int m_stackControlCurrent = 0;              // 0:相册界面 1:看图界面 2:幻灯片
    int m_stackControlLastCurrent = -1;         // 记录上一次显示的主界面索引 0:相册界面 1:看图界面 2:幻灯片
    int m_thumbnailSizeLevel = -1;              // 缩略图缩放等级
    qreal m_cellBaseWidth;                      // 缩略图网格大小
    QString m_statusBarNumText = "";            // 状态栏显示的总数文本内容
    QString m_searchEditText = "";              // 搜索框文本内容

    bool m_bAlbumImportChangeList = false;      // 自动导入相册改变标记
    bool m_bAlbumChangeList = false;            // 自定义相册改变标记

    bool m_bSideBarIsVisible = true;                // 显示/隐藏侧边栏

    QString m_currentDeviceName = "";           // 当前设备名称
    QString m_currentDevicePath= "";            // 当前设备路径

    bool m_bWindowDisactived = false;           // 窗口是否激活标记
    bool m_bLoading = true;                     // 记录相册界面是否处于加载状态

    bool m_bEnableRatioAnimation = false;       // 比例切换动画使能标识
    bool m_bEnableFadeInoutAnimation = false;   // 淡入淡出动画使能标识
    bool m_bEnteringImageViewer = false;        // 相册进入看图
    bool m_bBackingToMainAlbumView = false;     // 正常看图返回相册界面
    int m_nAnimationDuration = 400;             // 动画持续时间
    int m_nLargeImagePreviewAnimationDuration = 800; // 大图预览动画持续时间
    FileControl* m_fileControl { nullptr };
};

#endif  // GLOBALSTATUS_H
