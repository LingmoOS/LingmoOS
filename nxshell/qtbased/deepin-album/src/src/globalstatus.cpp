// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalstatus.h"
#include "filecontrol.h"
#include "albumControl.h"

static const int sc_MinHeight = 300;           // 窗口最小高度
static const int sc_MinWidth = 658;            // 窗口最小宽度
static const int sc_MinHideHeight = 425;       // 调整窗口高度小于425px时，隐藏工具栏和标题栏
static const int sc_FloatMargin = 65;          // 浮动按钮边距
static const int sc_TitleHeight = 50;          // 标题栏栏高度
static const int sc_ThumbnailViewHeight = 70;  // 底部工具栏高度 70px
static const int sc_ShowBottomY = 80;  // 底部工具栏显示时距离底部的高度 80px (工具栏高度 70px + 边距 10px)
static const int sc_SwitchImageHotspotWidth = 100;  // 左右切换图片按钮的热区宽度 100px
static const int sc_ActionMargin = 9;               // 应用图标距离顶栏
static const int sc_RightMenuItemHeight = 32;       // 右键菜单item的高度

static const double sc_AnimationDefaultDuration = 366;  // 默认动画时长
static const int sc_PathViewItemCount = 3;              // 默认 PathView 在路径中的 Item 计数

// 相册相关状态变量
static const int sc_RightMenuSeparatorHeight = 12;   // 右键菜单分割层的高度
static const int sc_NeedHideSideBarWidth = 783;      // 需要隐藏侧边栏的时，主界面宽度
static const int sc_SideBarWidth = 200;              // 侧边栏宽度
static const int sc_StatusBarHeight = 30;            // 状态栏高度
static const int sc_CollectionTopMargin = 25;        // 合集年月视图上边距
static const int sc_ThumbnailViewTitleHieght = 85;   // 缩略图视图区域标题显示区域高度
static const int sc_VerticalScrollBarWidth = 15;     // 垂直滚动条宽度
static const int sc_RectSelScrollStep = 30;          // 框选滚动步进
static const int sc_ThumbnailListRightMargin = 10;   // 框选滚动步进
static const int sc_ThumbnialListCellSpace = 4;      // 框选滚动步进
/**
   @class GlobalStatus
   @brief QML单例类，维护全局状态，同步不同组件间的状态信息
   @details 相较于使用脚本配置的 program Singletion , Qt 更推崇使用 QObject 注册单例
   @link https://doc.qt.io/qt-6/qtquick-performance.html#use-singleton-types-instead-of-pragma-library-scripts
 */

GlobalStatus::GlobalStatus(QObject *parent)
    : QObject(parent)
{
    initConnect();
}

GlobalStatus::~GlobalStatus() { }

/**
   @return 返回是否全屏显示图片
 */
bool GlobalStatus::showFullScreen() const
{
    return storeshowFullScreen;
}

/**
   @brief 设置全屏显示图片
 */
void GlobalStatus::setShowFullScreen(bool value)
{
    if (value != storeshowFullScreen) {
        storeshowFullScreen = value;
        Q_EMIT showFullScreenChanged();
    }
}

/**
   @return 返回是否允许显示导航窗口
 */
bool GlobalStatus::enableNavigation() const
{
    return storeenableNavigation;
}

/**
   @brief 设置是否允许显示导航窗口
 */
void GlobalStatus::setEnableNavigation(bool value)
{
    if (value != storeenableNavigation) {
        storeenableNavigation = value;
        Q_EMIT enableNavigationChanged();
    }
}

/**
   @return 返回是否显示右键菜单
 */
bool GlobalStatus::showRightMenu() const
{
    return storeshowRightMenu;
}

/**
   @brief 设置是否显示右键菜单
 */
void GlobalStatus::setShowRightMenu(bool value)
{
    if (value != storeshowRightMenu) {
        storeshowRightMenu = value;
        Q_EMIT showRightMenuChanged();
    }
}

/**
   @return 当前是否弹窗显示详细图像信息
 */
bool GlobalStatus::showImageInfo() const
{
    return storeshowImageInfo;
}

/**
   @brief 设置是否弹窗显示详细图像信息
 */
void GlobalStatus::setShowImageInfo(bool value)
{
    if (value != storeshowImageInfo) {
        storeshowImageInfo = value;
        Q_EMIT showImageInfoChanged();
    }
}

/**
   @return 返回滑动视图是否响应操作
 */
bool GlobalStatus::viewInteractive() const
{
    return storeviewInteractive;
}

/**
   @brief 设置滑动视图是否响应操作
 */
void GlobalStatus::setViewInteractive(bool value)
{
    if (value != storeviewInteractive) {
        storeviewInteractive = value;
        Q_EMIT viewInteractiveChanged();
    }
}

/**
   @return 返回滑动视图是否处于轻弹状态
 */
bool GlobalStatus::viewFlicking() const
{
    return storeviewFlicking;
}

/**
   @brief 设置当前滑动视图是否处于轻弹状态
 */
void GlobalStatus::setViewFlicking(bool value)
{
    if (value != storeviewFlicking) {
        storeviewFlicking = value;
        Q_EMIT viewFlickingChanged();
    }
}

/**
   @return 返回当前是否允许标题栏、底栏动画效果
 */
bool GlobalStatus::animationBlock() const
{
    return storeanimationBlock;
}

/**
   @brief 设置当前允许标题栏、底栏动画效果的标志值为 \a value
 */
void GlobalStatus::setAnimationBlock(bool value)
{
    if (value != storeanimationBlock) {
        storeanimationBlock = value;
        Q_EMIT animationBlockChanged();
    }
}

/**
   @return 返回当前是否允许全屏展示动画
 */
bool GlobalStatus::fullScreenAnimating() const
{
    return storefullScreenAnimating;
}

/**
   @brief 设置当前是否允许全屏展示动画的标志值为 \a value
 */
void GlobalStatus::setFullScreenAnimating(bool value)
{
    if (value != storefullScreenAnimating) {
        storefullScreenAnimating = value;
        Q_EMIT fullScreenAnimatingChanged();
    }
}

/**
   @return 返回当前缩略图列表允许显示的宽度
 */
int GlobalStatus::thumbnailVaildWidth() const
{
    return storethumbnailVaildWidth;
}

/**
   @brief 设置当前缩略图列表允许显示的宽度为 \a value
 */
void GlobalStatus::setThumbnailVaildWidth(int value)
{
    if (value != storethumbnailVaildWidth) {
        storethumbnailVaildWidth = value;
        Q_EMIT thumbnailVaildWidthChanged();
    }
}

/**
   @return 返回当前显示的界面索引
 */
Types::StackPage GlobalStatus::stackPage() const
{
    return storestackPage;
}

/**
   @brief 设置当前显示的界面索引为 \a value ，将切换显示的界面类型
 */
void GlobalStatus::setStackPage(Types::StackPage value)
{
    if (value != storestackPage) {
        storestackPage = value;
        Q_EMIT stackPageChanged();
    }
}

bool GlobalStatus::showExportDialog() const
{
    return storeshowExportDialog;
}

void GlobalStatus::setShowExportDialog(bool value)
{
    if (value != storeshowExportDialog) {
        storeshowExportDialog = value;
        Q_EMIT showExportDialogChanged();
    }
}

int GlobalStatus::minHeight() const
{
    return sc_MinHeight;
}

int GlobalStatus::minWidth() const
{
    return sc_MinWidth;
}

int GlobalStatus::minHideHeight() const
{
    return sc_MinHideHeight;
}

int GlobalStatus::floatMargin() const
{
    return sc_FloatMargin;
}

int GlobalStatus::titleHeight() const
{
    return sc_TitleHeight;
}

int GlobalStatus::thumbnailViewHeight() const
{
    return sc_ThumbnailViewHeight;
}

int GlobalStatus::showBottomY() const
{
    return sc_ShowBottomY;
}

int GlobalStatus::switchImageHotspotWidth() const
{
    return sc_SwitchImageHotspotWidth;
}

int GlobalStatus::actionMargin() const
{
    return sc_ActionMargin;
}

int GlobalStatus::rightMenuItemHeight() const
{
    return sc_RightMenuItemHeight;
}

double GlobalStatus::animationDefaultDuration() const
{
    return sc_AnimationDefaultDuration;
}

/**
   @brief 默认 PathView 在路径中的 Item 计数
   @note 会影响 PathView 相关的动画效果计算，修改此值需慎重考虑
 */
int GlobalStatus::pathViewItemCount() const
{
    return sc_PathViewItemCount;
}

void GlobalStatus::setFileControl(FileControl *fc)
{
    m_fileControl = fc;

    if (!m_fileControl)
        return;

    bool bRet = false;
    m_nAnimationDuration = m_fileControl->getConfigValue("", "animationDuration", 400).toInt(&bRet);
    if (!bRet)
        m_nAnimationDuration = 400;
    m_nLargeImagePreviewAnimationDuration = m_fileControl->getConfigValue("", "largeImagePreviewAnimationDuration", 800).toInt(&bRet);
    if (!bRet)
        m_nLargeImagePreviewAnimationDuration = 800;
}

int GlobalStatus::rightMenuSeparatorHeight() const
{
    return sc_RightMenuSeparatorHeight;
}

int GlobalStatus::sideBarWidth() const
{
    return sc_SideBarWidth;
}

int GlobalStatus::statusBarHeight() const
{
    return sc_StatusBarHeight;
}

int GlobalStatus::collectionTopMargin() const
{
    return sc_CollectionTopMargin;
}

int GlobalStatus::thumbnailViewTitleHieght() const
{
    return sc_ThumbnailViewTitleHieght;
}

int GlobalStatus::verticalScrollBarWidth() const
{
    return sc_VerticalScrollBarWidth;
}

int GlobalStatus::rectSelScrollStep() const
{
    return sc_RectSelScrollStep;
}

int GlobalStatus::thumbnailListRightMargin() const
{
    return sc_ThumbnailListRightMargin;
}

int GlobalStatus::thumbnialListCellSpace() const
{
    return sc_ThumbnialListCellSpace;
}

int GlobalStatus::needHideSideBarWidth() const
{
    return sc_NeedHideSideBarWidth;
}

int GlobalStatus::animationDuration() const
{
    return m_nAnimationDuration;
}

int GlobalStatus::largeImagePreviewAnimationDuration() const
{
    return m_nLargeImagePreviewAnimationDuration;
}

qreal GlobalStatus::sideBarX() const
{
    return m_sideBar_X;
}

void GlobalStatus::setSideBarX(const qreal& value)
{
    if (!qFuzzyCompare(m_sideBar_X, value)) {
        m_sideBar_X = value;
        Q_EMIT sideBarXChanged();
    }
}

QVariantList GlobalStatus::selectedPaths() const
{
    return m_selectedPaths;
}

void GlobalStatus::setSelectedPaths(const QVariantList& value)
{
    if (m_selectedPaths != value) {
        m_selectedPaths = value;
        Q_EMIT selectedPathsChanged();
    }
}

bool GlobalStatus::bRefreshFavoriteIconFlag() const
{
    return m_bRefreshFavoriteIconFlag;
}

void GlobalStatus::setBRefreshFavoriteIconFlag(const bool& value)
{
    if (m_bRefreshFavoriteIconFlag != value) {
        m_bRefreshFavoriteIconFlag = value;
        Q_EMIT bRefreshFavoriteIconFlagChanged();
    }
}

bool GlobalStatus::refreshRangeBtnState() const
{
    return m_bRefreshRangeBtnState;
}

void GlobalStatus::setRefreshRangeBtnState(const bool& value)
{
    if (m_bRefreshRangeBtnState != value) {
        m_bRefreshRangeBtnState = value;
        Q_EMIT refreshRangeBtnStateChanged();
    }
}

Types::ThumbnailViewType GlobalStatus::currentViewType() const
{
    return m_currentViewType;
}

void GlobalStatus::setCurrentViewType(const Types::ThumbnailViewType &value)
{
    if (m_currentViewType != value) {
        m_currentViewType = value;

        setEnableRatioAnimation(false);
        setBackingToMainAlbumView(false);
        // 若相册数据库没有图片资源，则调整显示“没有图片“提示视图
        if (AlbumControl::instance()->getAllCount() <= 0) {
            switch (value) {
            case Types::ViewImport:
            case Types::ViewNoPicture:
            case Types::ViewCollecttion:
                break;
            case Types::ViewSearchResult:
                m_currentViewType = Types::ViewNoPicture;
                break;
            default:
                break;

            }
        }
        Q_EMIT currentViewTypeChanged();
    }
}

int GlobalStatus::currentCollecttionViewIndex() const
{
    return m_currentCollecttionViewIndex;
}

void GlobalStatus::setCurrentCollecttionViewIndex(const int &value)
{
    if (m_currentCollecttionViewIndex != value) {
        m_currentCollecttionViewIndex = value;
        setEnableRatioAnimation(false);
        setBackingToMainAlbumView(false);
        Q_EMIT currentCollecttionViewIndexChanged();
    }
}

Types::SwitchType GlobalStatus::currentSwitchType() const
{
    return m_currentSwitchType;
}

void GlobalStatus::setCurrentSwitchType(const int &value)
{
    if (m_currentSwitchType != value) {
        m_currentSwitchType = static_cast<Types::SwitchType>(value);
        Q_EMIT currentSwitchTypeChanged();
    }
}

int GlobalStatus::currentCustomAlbumUId() const
{
    return m_currentCustomAlbumUId;
}

void GlobalStatus::setCurrentCustomAlbumUId(const int &value)
{
    if (m_currentCustomAlbumUId != value) {
        setBackingToMainAlbumView(false);
        m_currentCustomAlbumUId = value;
        Q_EMIT currentCustomAlbumUIdChanged();
    }
}

int GlobalStatus::stackControlCurrent() const
{
    return m_stackControlCurrent;
}

void GlobalStatus::setStackControlCurrent(const int &value)
{
    if (m_stackControlCurrent != value) {

        if (m_stackControlCurrent != 0 && value == 0) {
            setBackingToMainAlbumView(true);
            Q_EMIT sigMoveToAlbumAnimation();
        } else {
            setBackingToMainAlbumView(false);
        }
        m_stackControlCurrent = value;
        Q_EMIT stackControlCurrentChanged();
    }
}

int GlobalStatus::stackControlLastCurrent() const
{
    return m_stackControlLastCurrent;
}

void GlobalStatus::setStackControlLastCurrent(const int &value)
{
    if (m_stackControlLastCurrent != value) {
        m_stackControlLastCurrent = value;
        Q_EMIT stackControlLastCurrentChanged();
    }
}

int GlobalStatus::thumbnailSizeLevel() const
{
    return m_thumbnailSizeLevel;
}

void GlobalStatus::setThumbnailSizeLevel(const int &value)
{
    if (m_thumbnailSizeLevel != value) {
        m_thumbnailSizeLevel = value;

        Q_EMIT thumbnailSizeLevelChanged();

        // 缩放等级有调整， 同步调整网格大小
        qreal newCellBaseWidth = m_thumbnailSizeLevel >= 0 && m_thumbnailSizeLevel <= 9 ? 80 + m_thumbnailSizeLevel * 10 : 80;
        setCellBaseWidth(newCellBaseWidth);
    }
}

qreal GlobalStatus::cellBaseWidth() const
{
    return m_cellBaseWidth;
}

void GlobalStatus::setCellBaseWidth(const qreal& value)
{
    if (!qFuzzyCompare(m_cellBaseWidth, value)) {
        m_cellBaseWidth = value;
        Q_EMIT cellBaseWidthChanged();
    }
}

QString GlobalStatus::statusBarNumText() const
{
    return m_statusBarNumText;
}

void GlobalStatus::setStatusBarNumText(const QString &value)
{
    if (m_statusBarNumText != value) {
        m_statusBarNumText = value;
        Q_EMIT statusBarNumTextChanged();
    }
}

QString GlobalStatus::searchEditText() const
{
    return m_searchEditText;
}

void GlobalStatus::setSearchEditText(const QString &value)
{
    if (m_searchEditText != value) {
        m_searchEditText = value;
        Q_EMIT searchEditTextChanged();
    }
}

bool GlobalStatus::albumImportChangeList() const
{
    return m_bAlbumImportChangeList;
}

void GlobalStatus::setAlbumImportChangeList(const bool &value)
{
    if (m_bAlbumImportChangeList != value) {
        m_bAlbumImportChangeList = value;
        Q_EMIT albumImportChangeListChanged();
    }
}

bool GlobalStatus::albumChangeList() const
{
    return  m_bAlbumChangeList;
}

void GlobalStatus::setAlbumChangeList(const bool &value)
{
    if (m_bAlbumChangeList != value) {
        m_bAlbumChangeList = value;
        Q_EMIT albumChangeListChanged();
    }
}

bool GlobalStatus::sideBarIsVisible() const
{
    return  m_bSideBarIsVisible;
}

void GlobalStatus::setSideBarIsVisible(const bool &value)
{
    if (m_bSideBarIsVisible != value) {
        m_bSideBarIsVisible = value;
        Q_EMIT sideBarIsVisibleChanged();
    }
}

QString GlobalStatus::currentDeviceName() const
{
    return m_currentDeviceName;
}

void GlobalStatus::setCurrentDeviceName(const QString &value)
{
    if (m_currentDeviceName != value) {
        m_currentDeviceName = value;
        Q_EMIT currentDeviceNameChanged();
    }
}

QString GlobalStatus::currentDevicePath() const
{
    return m_currentDevicePath;
}

void GlobalStatus::setCurrentDevicePath(const QString &value)
{
    if (m_currentDevicePath != value) {
        m_currentDevicePath = value;
        m_currentDeviceName = AlbumControl::instance()->getDeviceName(m_currentDevicePath);
        Q_EMIT currentDevicePathChanged();
    }
}

bool GlobalStatus::windowDisactived() const
{
    return m_bWindowDisactived;
}

void GlobalStatus::setWindowDisactived(const bool &value)
{
    if (m_bWindowDisactived != value) {
        m_bWindowDisactived = value;
        Q_EMIT windowDisactivedChanged();
    }
}

bool GlobalStatus::loading() const
{
    return m_bLoading;
}

void GlobalStatus::setLoading(const bool &value)
{
    if (m_bLoading != value) {
        m_bLoading = value;
        if (!m_bLoading) {
            setEnableFadeInoutAnimation(true);
        }
        Q_EMIT loadingChanged();
    }
}

bool GlobalStatus::enableRatioAnimation() const
{
    return m_bEnableRatioAnimation;
}

void GlobalStatus::setEnableRatioAnimation(const bool &value)
{
    if (m_bEnableRatioAnimation != value) {
        m_bEnableRatioAnimation = value;
        Q_EMIT enableRatioAnimationChanged();
    }
}

bool GlobalStatus::enableFadeInoutAnimation() const
{
    return m_bEnableFadeInoutAnimation;

}

void GlobalStatus::setEnableFadeInoutAnimation(const bool &value)
{
    if (m_bEnableFadeInoutAnimation != value) {
        m_bEnableFadeInoutAnimation = value;
        Q_EMIT enableFadeInoutAnimationChanged();
    }
}

bool GlobalStatus::enteringImageViewer() const
{
    return m_bEnteringImageViewer;
}

void GlobalStatus::setEnteringImageViewer(const bool &value)
{
    if (m_bEnteringImageViewer != value) {
        m_bEnteringImageViewer = value;
        Q_EMIT enteringImageViewerChanged();
    }
}

bool GlobalStatus::backingToMainAlbumView() const
{
    return m_bBackingToMainAlbumView;
}

void GlobalStatus::setBackingToMainAlbumView(const bool &value)
{
    if (m_bBackingToMainAlbumView != value) {
        m_bBackingToMainAlbumView = value;
        Q_EMIT backingToMainAlbumViewChanged();
    }
}

void GlobalStatus::initConnect()
{
    // 数据库监听-删除图片后通知前端刷新自定义相册视图内容
    connect(AlbumControl::instance(), SIGNAL(sigRefreshCustomAlbum(int)), SIGNAL(sigFlushCustomAlbumView(int)));

    // 数据库监听-删除图片后通知前端刷新合集所有项目
    connect(AlbumControl::instance(), SIGNAL(sigRefreshAllCollection()), SIGNAL(sigFlushAllCollectionView()));

    //数据库监听-删除图片后通知前端刷新已导入视图内容
    connect(AlbumControl::instance(), &AlbumControl::sigRefreshImportAlbum, this, [=]() {
        sigFlushHaveImportedView();
        setRefreshRangeBtnState(!m_bRefreshRangeBtnState);
    });

    // 数据库监听-删除图片后通知前端刷新搜索结果视图内容
    connect(AlbumControl::instance(), &AlbumControl::sigRefreshSearchView, this, [=]() {
        if (m_currentViewType == Types::ViewSearchResult)
            sigFlushSearchView();
    });

    // 自动导入相册有新增相册，通知前端刷新侧边栏自动导入相册列表
    connect(AlbumControl::instance(), &AlbumControl::sigRefreshSlider, this, [=]() {
        setAlbumImportChangeList(!m_bAlbumImportChangeList);
    });
}

QString GlobalStatus::getSelectedNumText(const QStringList &paths, const QString &text)
{
    QList<int> ret = AlbumControl::instance()->getPicVideoCountFromPaths(paths);

    //QML的翻译不支持%n的特性，只能拆成这种代码
    int photoCount = ret[0];
    int videoCount = ret[1];
    QString selectedNumText("");
    if(paths.size() == 0) {
        selectedNumText = text;
    } else if(paths.size() == 1 && photoCount == 1) {
        selectedNumText = tr("1 item selected (1 photo)");
    } else if(paths.size() == 1 && videoCount == 1) {
        selectedNumText = tr("1 item selected (1 video)");
    } else if(photoCount > 1 && videoCount == 0) {
        selectedNumText = tr("%1 items selected (%1 photos)").arg(photoCount);
    } else if(videoCount > 1 && photoCount == 0) {
        selectedNumText = tr("%1 items selected (%1 videos)").arg(videoCount);
    } else if (photoCount == 1 && videoCount == 1) {
        selectedNumText = tr("%1 item selected (1 photo, 1 video)").arg(photoCount + videoCount);
    } else if (photoCount == 1 && videoCount > 1) {
        selectedNumText = tr("%1 items selected (1 photo, %2 videos)").arg(photoCount + videoCount).arg(videoCount);
    } else if (videoCount == 1 && photoCount > 1) {
        selectedNumText = tr("%1 items selected (%2 photos, 1 video)").arg(photoCount + videoCount).arg(photoCount);
    } else if (photoCount > 1 && videoCount > 1){
        selectedNumText = tr("%1 items selected (%2 photos, %3 videos)").arg(photoCount + videoCount).arg(photoCount).arg(videoCount);
    }

    return selectedNumText;
}
