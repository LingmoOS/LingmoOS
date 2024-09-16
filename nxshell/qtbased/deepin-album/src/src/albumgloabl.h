// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ALBUMGLOBAL_H
#define ALBUMGLOBAL_H

#include <QPixmap>
#include <QList>
#include <QPixmap>
#include <QIcon>
#include <QFileInfo>
#include <QSize>
#include <QBuffer>
#include <QPointer>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QElapsedTimer>

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
    //QString albumUID;      // 图片所属相册UID，以","分隔，用于恢复，有需要再放开
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
            //<< " UID:" << info.albumUID
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

Q_DECLARE_METATYPE(DBImgInfo)
#endif // ALBUMGLOBAL_H
