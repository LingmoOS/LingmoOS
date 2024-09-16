// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <DWidget>
#include <DIconButton>

#include "image-viewer_global.h"

class AbstractTopToolbar;

DWIDGET_USE_NAMESPACE

class ImageViewerPrivate;
class IMAGEVIEWERSHARED_EXPORT ImageViewer : public DWidget
{
    Q_OBJECT
public:
    // ImgViewerType:图片展示类型, savePath:缩略图保存位置，customTopToolbar:自定义标题栏，nullptr的时候使用内置方案
    explicit ImageViewer(imageViewerSpace::ImgViewerType imgViewerType,
                         QString savePath,
                         AbstractTopToolbar *customTopToolbar = nullptr,
                         QWidget *parent = nullptr);
    ~ImageViewer() override;

    // 调用文件选择窗口
    bool startChooseFileDialog();

    // 传入路径加载图片 paths：所有照片 firstPath：第一张 isCustom：是否是自定义相册 album：自定义相册名称
    bool
    startdragImage(const QStringList &paths, const QString &firstPath = "", bool isCustom = false, const QString &album = "");

    // 传入路径加载图片 paths：所有照片 firstPath：第一张 isCustom：是否是自定义相册 album：自定义相册名称 UID：相册的UID
    bool startdragImageWithUID(
        const QStringList &paths, const QString &firstPath = "", bool isCustom = false, const QString &album = "", int UID = -1);

    // 启动图片展示入口
    void startImgView(const QString &currentPath, const QStringList &paths = QStringList());

    // 设置topbar的显示和隐藏
    void setTopBarVisible(bool visible);

    // 设置Bottomtoolbar的显示和隐藏
    void setBottomtoolbarVisible(bool visible);

    // 获得工具栏按钮
    DIconButton *getBottomtoolbarButton(imageViewerSpace::ButtonType type);

    // 获得当前展示图片路径
    QString getCurrentPath();

    // 二次开发接口

    // 设置图片显示panel右键菜单的显示和隐藏，false为永久隐藏，true为跟随原有策略显示或隐藏
    void setViewPanelContextMenuItemVisible(imageViewerSpace::NormalMenuItemId id, bool visible);

    // 设置下方工具栏按钮的显示和隐藏，true为永久隐藏，false为跟随原有策略显示或隐藏
    void setBottomToolBarButtonAlawysNotVisible(imageViewerSpace::ButtonType id, bool notVisible);

    // 相册-给右键菜单传输自定义相册名称
    void setCustomAlbumName(const QMap<QString, bool> map, bool isFav);

    // 相册-给右键菜单传输自定义相册名称的UID方案接口，map的参数依次是UID，相册名，是否在此相册
    void setCustomAlbumNameAndUID(const QMap<int, std::pair<QString, bool> > &map, bool isFav);

    // 设置全屏/退出全屏
    void switchFullScreen();

    // 设置启动幻灯片
    void startSlideShow(const QStringList &paths, const QString &firstPath);

    // 设置panel拖拽使能
    void setDropEnabled(bool enable);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    QScopedPointer<ImageViewerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ImageViewer)
};

#endif  // IMAGEVIEWER_H
