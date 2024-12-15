// SPDX-FileCopyrightText: 2020 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include <QFrame>
#include <QFileSystemWatcher>

#include <DStackedWidget>
#include <DAnchors>
#include <DIconButton>
#include <dshadowline.h>

#include <bitset>

#include "scen/imagegraphicsview.h"
#include "slideshow/slideshowpanel.h"

class AbstractTopToolbar;

DWIDGET_USE_NAMESPACE
class LibImageInfoWidget;
class ExtensionPanel;
class NavigationWidget;
class LibBottomToolbar;
class LibTopToolbar;
class OcrInterface;
class LibSlideShowPanel;
class QPropertyAnimation;
class LockWidget;
class ThumbnailWidget;
class AIEnhanceFloatWidget;

class LibViewPanel : public QFrame
{

    Q_OBJECT
public:
    explicit LibViewPanel(AbstractTopToolbar *customToolbar = nullptr, QWidget *parent = nullptr);
    ~LibViewPanel() override;

    void loadImage(const QString &path, QStringList paths);

    void initConnect();
    //初始化标题栏
    void initTopBar();
    //初始化ocr
    void initOcr();
    //初始化缩放比和导航窗口
    void initFloatingComponent();
    //初始化缩放比例的窗口
    void initScaleLabel();
    //初始化导航窗口
    void initNavigation();
    //初始化右键菜单
    void initRightMenu();
    //初始化详细信息
    void initExtensionPanel();
    //幻灯片初始化
    void initSlidePanel();
    //损坏图片初始化
    void initLockPanel();
    //删除图片初始化
    void initThumbnailWidget();
    //初始化快捷键
    void initShortcut();
    //更新右键菜单
    void updateMenuContent(const QString &path = "");
    //控制全屏和返回全屏
    void toggleFullScreen();
    //全屏
    void showFullScreen();
    //退出全屏
    void showNormal();
    //启动幻灯片播放
    void startSlideShow(const ViewInfo &info);
    /**
     * @brief appendAction  添加右键菜单按钮
     * @param id            按钮枚举ID
     * @param text          按钮名称
     * @param shortcut      按钮快捷键
     */
    QAction *appendAction(int id, const QString &text, const QString &shortcut = "", bool enable = true);

    //设置壁纸
    void setWallpaper(const QImage &img);

    //设置壁纸
    void setWallpaper(const QString &imgPath);

    //drag事件打开图片
    bool startdragImage(const QStringList &paths, const QString &firstPath = "");

    //设置topBar的显示和隐藏
    void setTopBarVisible(bool visible);

    //设置Bottomtoolbar的显示和隐藏
    void setBottomtoolbarVisible(bool visible);

    //设置右键菜单显隐：false为永久隐藏，true为跟随原有策略显示或隐藏
    void setContextMenuItemVisible(imageViewerSpace::NormalMenuItemId id, bool visible);

    //设置下方工具栏按钮的显示和隐藏，false为永久隐藏，true为跟随原有策略显示或隐藏
    void setBottomToolBarButtonAlawysNotVisible(imageViewerSpace::ButtonType id, bool notVisible);

    //获得工具栏按钮
    DIconButton *getBottomtoolbarButton(imageViewerSpace::ButtonType type);

    //获得当前展示图片路径
    QString getCurrentPath();

    //show
    void showTopBottom();

    void showAnimationTopBottom();

    //hide
    void hideTopBottom();

    void hideAnimationTopBottom();

    void loadThumbnails(const QString &path);
private slots:
    void onMenuItemClicked(QAction *action);

    //存在图片刷新
    void slotOneImgReady(QString path, imageViewerSpace::ItemInfo itemInfo);

    //设置当前图片应该的界面
    void setCurrentWidget(const QString &path);

    //图片显示超过标题栏
    void slotsImageOutTitleBar(bool bRet);

    //文件改变
    void slotsDirectoryChanged(const QString &path);
public slots:
    //刷新底部工具栏大小与位置
    void resetBottomToolbarGeometry(bool visible);
    //打开图片
    void openImg(int index, QString path);

    //旋转图片
    void slotRotateImage(int angle);

    //适应窗口和图片
    void slotResetTransform(bool bRet);

    //ocr连接
    bool slotOcrPicture();

    //回到视图界面
    void backImageView(const QString &path = "");

    //选择打开窗口
    bool startChooseFileDialog();

    //刷新底部工具栏在全屏的隐藏与否
    void slotBottomMove();

    void noAnimationBottomMove();

    //下一张图片
    void showNext();

    //上一张图片
    void showPrevious();

    //更新当前照片的自定义相册包含情况
    void updateCustomAlbum(const QMap<QString, bool> &map, bool isFav);
    void updateCustomAlbumAndUID(const QMap<int, std::pair<QString, bool> > &map, bool isFav);
    //缩略图打开界面是否是从自定义界面打开
    void setIsCustomAlbum(bool isCustom, const QString &album = "");
    void setIsCustomAlbumWithUID(bool isCustom, const QString &album = "", int UID = -1);

    void slotChangeShowTopBottom();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;//

    void leaveEvent(QEvent *event) override;

    void hideEvent(QHideEvent *e) override;

    // 捕获窗口的状态变更事件
    bool eventFilter(QObject *o, QEvent *e) override;

signals:
    void imageChanged(const QString &path);

    //刷新缩略图
    void updateThumbnail(QPixmap pix, const QSize &originalSize);

private:
    // AI图像增强相关接口
    void addAIMenu();
    void createAIBtn();
    void setAIBtnVisible(bool visible);
    void triggerImageEnhance(const QString &filePath, int modelID);
    void blockInputControl(bool block);
    Q_SLOT void resetAIEnhanceImage();
    Q_SLOT void onEnhanceStart();
    Q_SLOT void onEnhanceReload(const QString &output);
    Q_SLOT void onEnhanceEnd(const QString &source, const QString &output, int state);
    void updateTitleShadow(bool toShow);

private:
    DStackedWidget *m_stack = nullptr;
    LibImageGraphicsView *m_view = nullptr;
    LockWidget *m_lockWidget = nullptr;
    ThumbnailWidget *m_thumbnailWidget = nullptr;
    LibBottomToolbar *m_bottomToolbar = nullptr;
    LibImageInfoWidget *m_info = nullptr;
    DShadowLine *titleShadow = nullptr;
    ExtensionPanel  *m_extensionPanel {nullptr};

    DAnchors<NavigationWidget> m_nav ;

    //ocr接口
    OcrInterface *m_ocrInterface{nullptr};

    AbstractTopToolbar *m_topToolbar = nullptr;
    bool m_topToolBarIsAlwaysHide = false;

    DMenu *m_menu = nullptr;
    std::bitset<imageViewerSpace::MenuItemCount> m_menuItemDisplaySwitch;

    bool m_isMaximized = false;

    QTimer *m_tSaveImage = nullptr;//保存旋转图片定时器

    LibSlideShowPanel *m_sliderPanel{nullptr};

    QPropertyAnimation *m_bottomAnimation{nullptr};
    QPropertyAnimation *m_topBarAnimation{nullptr};

    bool m_isBottomBarVisble = true;

    QString m_currentPath = "";
    //当前相册所有自定义相册名以及是否包含当前照片
    QMap<QString, bool> m_CustomAlbumName;
    QMap<int, std::pair<QString, bool> >  m_CustomAlbumAndUID;
    bool m_isFav = false;
    bool m_useUID = false; //是否使用UID机制
    //当前打开图片是否从自定义打开，自定义相册名
    bool m_isCustomAlbum = false;
    QString m_CurrentCustomName = "";
    int m_CurrentCustomUID = -2;//自定义相册名对应的UID，-2表示从老版本相册进入

    int m_hideCursorTid{0};//隐藏时间

    //图片是否超过标题栏
    bool m_ImageOutTitleBar = false;

    QFileSystemWatcher *m_dirWatcher = nullptr;

    bool m_isShowTopBottom = true;

    QSize m_windowSize;
    int m_windowX = 0;
    int m_windowY = 0;

    // AI 功能
    bool m_AIEnhancing = false;
    bool notNeedNotifyEnhanceSave = false;          // 用于控制是否需要提示保存增强图片
    AIEnhanceFloatWidget *m_AIFloatBar = nullptr;   // 按钮浮动窗口
};
#endif  // VIEWPANEL_H
