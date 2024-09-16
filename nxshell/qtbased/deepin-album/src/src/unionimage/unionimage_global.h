// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEVIEWER_GLOBAL_H
#define IMAGEVIEWER_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QDateTime>
#include <QDebug>
#include <QDir>

#if defined(IMAGEVIEWER_LIBRARY)
#  define IMAGEVIEWERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define IMAGEVIEWERSHARED_EXPORT Q_DECL_IMPORT
#endif

const QString DATETIME_FORMAT_DATABASE = "yyyy.MM.dd hh:mm";

//相册的define
#define COMMON_STR_ALLPHOTOS "所有照片"
#define COMMON_STR_TIMELINE "时间线"
#define COMMON_STR_RECENT_IMPORTED "已导入"
#define COMMON_STR_TRASH "最近删除"
#define COMMON_STR_FAVORITES "我的收藏"
#define COMMON_STR_SEARCH "搜索"
#define ALBUM_PATHNAME_BY_PHONE "DCIM"
#define ALBUM_PATHTYPE_BY_PHONE "External Devices"
#define ALBUM_PATHTYPE_BY_U "U Devices"
#define COMMON_STR_SLIDESHOW "幻灯片放映"
#define COMMON_STR_CREATEALBUM "新建相册"
#define COMMON_STR_RENAMEALBUM "重命名相册"
#define COMMON_STR_EXPORT "导出相册"
#define COMMON_STR_DELETEALBUM "删除相册"
#define COMMON_STR_VIEW_TIMELINE "timelineview"
#define COMMON_STR_CUSTOM "自定义"

enum DelegateType {
    NullType = 0,
    AllPicViewType,//所有照片

    TimeLineViewType,//时间线

    SearchViewType,//搜索

    AlbumViewImportTimeLineViewType,//相册-最近导入
    AlbumViewTrashType,//相册-已删除
    AlbumViewFavoriteType,//相册-收藏
    AlbumViewCustomType,//相册-自定义
    AlbumViewPhoneType//相册-设备
};

namespace albumGlobal {

//图片缓存文件夹
const QString CACHE_PATH = QDir::homePath() + "/.local/share/deepin/deepin-album";

//图片删除文件夹
const QString DELETE_PATH = QDir::homePath() + "/.local/share/deepin/deepin-album-delete";

}
//数据库存储的文件类型
enum DbFileType {
    DbFileTypeNull = 0,
    DbFileTypePic,      //图片
    DbFileTypeVideo     //视频
};

struct DBAlbumInfo {
    QString name;
//    int count;
    QDateTime beginTime;
    QDateTime endTime;
};

enum ItemType {
    ItemTypeNull = 1,
    ItemTypeBlank,         //空白项，列表上方，悬浮控件下方高度
    ItemTypePic,
    ItemTypeVideo,
    ItemTypeTimeLineTitle, //时间线标题
    ItemTypeImportTimeLineTitle, //已导入时间线标题
    ItemTypeMountImg //设备图片
};

//注意内部数据的摆放顺序，可以依靠64位程序的8字节对齐以节省空间
struct DBImgInfo {
    //数据库
    QString filePath;
    QDateTime time;        // 图片创建时间
    QDateTime changeTime;  // 文件修改时间
    QDateTime importTime;  // 导入时间 Or 删除时间
    QString albumUID = "-1";      // 图片所属相册UID，以","分隔，用于恢复
    QString pathHash;      // 用于应付频繁的hash，但不一定每个DBImgInfo都装载了它

    ItemType itemType = ItemTypePic;//类型，空白，图片，视频

    //显示
    int imgWidth = 0;
    int imgHeight = 0;
    int remainDays = 30;
    bool bNotSupportedOrDamaged = false;
    bool bNeedDelete = false;//删除时间线与已导入标题时使用
    bool isSelected = false;
    QPixmap image;
    QString date;
    QString num;

    friend QDebug operator<<(QDebug &dbg, const DBImgInfo &info)
    {
        dbg << "(DBImgInfo)["
            << " Path:" << info.filePath
            << " Name:" << info.getFileNameFromFilePath()
            << " Time:" << info.time
            << " ChangeTime:" <<  info.changeTime
            << " ImportTime:" << info.importTime
            << " UID:" << info.albumUID
            << "]";
        return dbg;
    }

    bool operator==(const DBImgInfo &info)
    {
        return this->filePath == info.filePath;
    }

    //辅助函数，在需要的时候调用

    //反hex，用于将toHex后的32字节hash值反向提取成标准16字节hash值，用于从数据库读取数据用
    //后续版本注意不要在非界面展示上使用toHex后的数据以节省内存和拷贝时间
    static QByteArray deHex(const QString &hexString)
    {
        QByteArray result;
        if (hexString.size() != 32) {
            return result;
        }
        auto srcData = hexString.toStdString();
        for (size_t i = 0; i < 32; i += 2) {
            quint8 highHelf = static_cast<quint8>(srcData[i]);
            quint8 lowHelf = static_cast<quint8>(srcData[i + 1]);
            result.push_back(static_cast<char>(highHelf << 4 | lowHelf));
        }
        return result;
    }

    //根据filepath获取filename
    //由于目标只是filePath.split('/').last()，所以直接从后向前搜索'/'以节省时间
    static QString getFileNameFromFilePath(const QString &filePath)
    {
        auto rIter = std::find(filePath.crbegin(), filePath.crend(), '/');
        if (rIter != filePath.crend()) {
            auto iter = rIter.base();
            QString result;
            std::copy(iter, filePath.cend(), std::back_inserter(result));
            return result;
        } else {
            return QString();
        }
    }

    QString getFileNameFromFilePath() const
    {
        return getFileNameFromFilePath(filePath);
    }
};
Q_DECLARE_METATYPE(DBImgInfo)

typedef QList<DBImgInfo> DBImgInfoList;

enum OpenImgViewType {
    VIEW_MAINWINDOW_ALLPIC = 0,
    VIEW_MAINWINDOW_TIMELINE = 1,
    VIEW_MAINWINDOW_ALBUM = 2 //
};

enum OpenImgAdditionalOperation {
    Operation_NoOperation = 0,
    Operation_FullScreen,
    Operation_StartSliderShow
};


//image viewer plugin space
namespace imageViewerSpace {

enum NormalMenuItemId {
    IdFullScreen,
    IdExitFullScreen,
    IdStartSlideShow,
    IdRename,
    IdPrint,
    IdAddToAlbum,
    IdExport,
    IdCopy,
    IdMoveToTrash,
    IdRemoveFromTimeline,
    IdRemoveFromAlbum,
    IdAddToFavorites,
    IdRemoveFromFavorites,
    IdShowNavigationWindow,
    IdHideNavigationWindow,
    IdRotateClockwise,
    IdRotateCounterclockwise,
    IdSetAsWallpaper,
    IdDisplayInFileManager,
    IdImageInfo,
    IdSubMenu,
    IdDraw,
    IdOcr,
    MenuItemCount
};
enum ItemInfoType {
    ItemTypeBlank = 1,//空白项，最上面留空使用
    ItemTypePic,
    ItemTypeVideo,
    ItemTypeTimeLineTitle, //时间线标题
    ItemTypeImportTimeLineTitle, //已导入时间线标题
    ItemTypeMountImg //设备图片
};
//图片类型
enum ImageType {
    ImageTypeBlank = 0,     //空白项，最上面留空使用
    ImageTypeSvg,           //SVG图片
    ImageTypeDynamic,       //所有动态图
    ImageTypeStatic,        //所有静态图
    ImageTypeMulti,         //所有多页图
    ImageTypeDamaged,       //所有损坏图片
};
//路径类型
enum PathType {
    PathTypeBlank = 0,     //空白项，最上面留空使用
    PathTypeLOCAL,         //本地
    PathTypeSMB,           //
    PathTypeFTP,           //
    PathTypeMTP,           //
    PathTypePTP,           //
    PathTypeAPPLE,         //苹果手机
    PathTypeSAFEBOX,       //保险箱
    PathTypeRECYCLEBIN,    //回收站
};
//工具栏按钮枚举
enum ButtonType {
    ButtonTypeBack = 0,  //相册返回按钮
    ButtonTypeNext,     //下一页
    ButtonTypePre,         //上一页
    ButtonTypeAdaptImage,       //适应图片大小
    ButtonTypeAdaptScreen,      //适应窗口大小
    ButtonTypeCollection,         //收藏按钮
    ButtonTypeOcr,        //ocr按钮
    ButtonTypeRotateLeft,   //向左旋转
    ButtonTypeRotateRight,  //向右旋转
    ButtonTypeTrash,       //删除
    ButtonTypeCount         //工具栏按钮个数
};

struct ItemInfo {
    QString name = "";
    QString path = "";                   //路径信息
    QString md5Hash = "";                //文件md5 hash值
    ImageType imageType = ImageTypeBlank;//图片类型
    PathType pathType = PathTypeBlank;   //路径类型
    int imgOriginalWidth = 0;            //原图大小,导入图片子线程中读取
    int imgOriginalHeight = 0;           //原图大小,导入图片子线程中读取
    int imgWidth = 0;
    int imgHeight = 0;
    QString remainDays = QObject::tr("day");
    bool isSelected;
    ItemInfoType itemType = ItemTypePic;//类型，空白，图片，视频
    QImage image = QImage();
    QImage damagedPixmap = QImage();
    bool bNotSupportedOrDamaged = false;
    bool bNeedDelete = false;//删除时间线与已导入标题时使用

    QString date;
    QString num;

    friend bool operator== (const ItemInfo &left, const ItemInfo &right)
    {
        if (left.image == right.image)
            return true;
        return false;
    }
};

//图片展示方式
enum ImgViewerType {
    ImgViewerTypeNull = 0,//默认
    ImgViewerTypeLocal,   //本地图片浏览
    ImgViewerTypeAlbum    //相册浏览使用
};

}

Q_DECLARE_METATYPE(imageViewerSpace::ItemInfo)
#endif // IMAGEVIEWER_GLOBAL_H
