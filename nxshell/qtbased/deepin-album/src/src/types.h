// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPES_H
#define TYPES_H

#include "unionimage/unionimage_global.h"

#include <QObject>

class Types : public QObject
{
    Q_OBJECT
    Q_ENUMS(ItemType)
    Q_ENUMS(ModelType)
    Q_ENUMS(ThumbnailViewType)
    Q_ENUMS(ThumbnailType)
    Q_ENUMS(RectScrollDirType)
    Q_ENUMS(FileDeleteType)

    Q_ENUMS(ItemRole)
    Q_ENUMS(ImageType)
    Q_ENUMS(StackPage)

    Q_ENUMS(SwitchType)
public:
    explicit Types(QObject *parent = nullptr);
    ~Types() override;

    enum ItemType { All = 0, Picture, Video };
    // 模型枚举
    enum ModelType {
        Normal = 0,                // 常规视图model （大多缩略图使用该model）
        DayCollecttion,            // 合集日视图model
        AllCollection,             // 合集所有项目视图model
        HaveImported,              // 已导入视图数据model
        RecentlyDeleted,           // 最近删除视图model
        CustomAlbum,               // 自定义相册视图model
        SearchResult,              // 搜索结果视图model
        Device                     // 设备视图mode
    };

    // 视图类型枚举
    enum ThumbnailViewType {
        ViewImport = 0,            // 导入图片视图
        ViewNoPicture,             // 无图片视图
        ViewCollecttion,           // 合集视图
        ViewHaveImported,          // 已导入视图
        ViewFavorite,              // 我的收藏视图
        ViewRecentlyDeleted,       // 最近删除视图
        ViewCustomAlbum,           // 系统/自定义相册视图
        ViewSearchResult,          // 搜索结果视图
        ViewDevice                 // 设备视图
    };

    //缩略图列表类型枚举
    enum ThumbnailType {
        ThumbnailNormal,           // 普通模式
        ThumbnailTrash,            // 最近删除
        ThumbnailCustomAlbum,      // 自定义相册
        ThumbnailAutoImport,       // 自动导入路径
        ThumbnailAllCollection,    // 合集模式
        ThumbnailDate,             // 日期模式
        ThumbnailDevice            // 设备列表
    };

    // 框选超出边界朝向类型
    enum RectScrollDirType {
        NoType,                    // 框选没有朝向
        ToTop,                     // 框选超出边界朝上
        ToBottom                   // 框选超出边界朝下
    };

    // 文件删除类型
    enum FileDeleteType {
        TrashNormal,               // 普通删除
        TrashSel,                  // 最近删除-选中删除
        TrashAll                   // 最近删除-全部删除
    };

    enum StackViewType {
        StackAlbum,                // 相册主界面
        StackImageViewer,            // 看图主界面
        StackSlider                // 幻灯片主界面
    };

    // 看图相关枚举类型
    enum ItemRole {
        ImageUrlRole = Qt::UserRole + 1,  ///< 图片路径
        FrameIndexRole,                   ///< 图片帧索引
        ImageAngleRole,                   ///< 图片旋转角度
    };

    /**
       @brief 图片文件类型
     */
    enum ImageType {
        NullImage,      ///< 无图片信息，文件为空
        NormalImage,    ///< 静态图片
        DynamicImage,   ///< 动态图
        SvgImage,       ///< SVG图片
        MultiImage,     ///< 多页图
        DamagedImage,   ///< 损坏图片
        NonexistImage,  ///< 图片文件不存在，删除、移动等
        NoPermissionImage, ///< 图片文件不可读，提示没有权限查看
    };

    /**
       @brief 界面类型
     */
    enum StackPage {
        OpenImagePage,   ///< 默认打开窗口界面
        ImageViewPage,   ///< 图片展示界面(含缩略图栏)
        SliderShowPage,  ///< 图片动画展示界面
    };

    /**
       @brief 年月日动画切换类型
     */
    enum SwitchType {
        UnknownSwitchType = -1,
        HardCut,        ///< 硬切
        FlipScroll,     ///< 滚动翻页
        FadeInOut,      ///< 渐显渐隐
        ScaleInOUt,     ///< 缩放淡入淡出
    };
};

#endif  // TYPES_H
