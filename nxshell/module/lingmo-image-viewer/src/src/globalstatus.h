// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALSTATUS_H
#define GLOBALSTATUS_H

#include "types.h"

#include <QObject>

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
    Types::StackPage storestackPage = Types::OpenImagePage;
};

#endif  // GLOBALSTATUS_H
