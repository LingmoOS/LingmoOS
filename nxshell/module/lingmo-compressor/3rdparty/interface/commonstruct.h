// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONSTRUCT_H
#define COMMONSTRUCT_H

#include <QDateTime>
#include <QString>
#include <QMetaType>
#include <QVector>
#include <QMap>
#include <QMimeType>

// 插件结束类型
enum PluginFinishType {
    PFT_Nomral = 0,     // 正常退出
    PFT_Cancel,         // 取消
    PFT_Error,          // 错误
};
Q_DECLARE_METATYPE(PluginFinishType)

// 错误类型
enum ErrorType {
    ET_NoError = 0,     // 无错误
    ET_PluginError,        // 插件错误
    ET_WrongPassword,     // 输入的密码错误
    ET_NeedPassword,     // 需要密码
    ET_LongNameError,     // 文件名过长错误（对于zip和tar文件名过长解压成功也会使用这个枚举值）
    ET_ArchiveDamaged,      // 压缩包损坏
    ET_FileOpenError,     // 文件打开错误
    ET_FileReadError,     // 文件读取错误
    ET_FileWriteError,     // 文件写错误
    ET_DeleteError,         // 文件删除错误
    ET_RenameError,         // 文件重命名错误
    ET_MissingVolume,    // 分卷缺失
    ET_InsufficientDiskSpace,   // 磁盘空间不足

    ET_UserCancelOpertion,     // 用户取消操作
    ET_ExistVolume,   // 分卷已存在
};

//加密类型
enum EncryptionType {
    Unencrypted,
    Encrypted,
    HeaderEncrypted
};

// 文件路径格式
enum PathFormat {
    NoTrailingSlash,
    WithTrailingSlash
};

// 工作类型
enum WorkType {
    WT_List,
    WT_Extract,
    WT_Add,
    WT_Delete,
    WT_Rename,
    WT_Move,
    WT_Copy,
    WT_Comment,
    WT_Test,
    WT_Convert
};

// 文件数据
struct FileEntry {
    FileEntry()
    {
        reset();
    }

    void reset()
    {
        strFullPath = "";
        strFileName = "";
        strAlias = "";
        isDirectory = false;
        qSize = 0;
        uLastModifiedTime = 0;
        iIndex = -1;
    }


    bool operator==(const FileEntry &t) const       //==的重载
    {
        if (this->strFullPath == t.strFullPath && this->strFileName == t.strFileName
                && this->isDirectory == t.isDirectory && this->qSize == t.qSize
                && this->uLastModifiedTime == t.uLastModifiedTime && this->iIndex == t.iIndex) {
            return true;
        }
        return false;
    }

    QString strFullPath;    // 文件全路径
    QString strFileName;        // 文件名
    QString strAlias;       //别名，文件重命名
    bool isDirectory;        // 是否为文件夹
    qlonglong qSize;        // 文件真实大小（文件夹显示项）
    uint uLastModifiedTime;      // 文件最后修改时间
    int iIndex;         // 文件在压缩包中的索引位置（目前是只有zip格式会用到，通过索引查找）
};
Q_DECLARE_METATYPE(FileEntry)

// 压缩包数据
struct ArchiveData {
    ArchiveData()
    {
        reset();
    }

    void reset()
    {
        qSize = 0;
        qComressSize = false;
        strComment = "";
        mapFileEntry.clear();
        listRootEntry.clear();

        isListEncrypted = false;
        strPassword.clear();
    }

    qlonglong qSize;                            // 原始总大小
    qlonglong qComressSize;                     // 压缩包大小
    QString strComment;                         // 压缩包注释信息
    QMap<QString, FileEntry>  mapFileEntry;     // 压缩包内所有文件
    QList<FileEntry> listRootEntry;             // 第一层数据

    bool isListEncrypted;                       // 压缩包是否为列表加密
    QString strPassword;                        // 压缩包密码
};
Q_DECLARE_METATYPE(ArchiveData)

// 压缩选项
struct CompressOptions {
    CompressOptions()
        : qTotalSize(0)
    {
        bEncryption = false;
        bHeaderEncryption = false;
        bSplit = false;
        iVolumeSize = 0;
        iCompressionLevel = -1;
        bTar_7z = false;
        iCPUTheadNum = 1;
    }

    QString strPassword;        // 密码
    QString strEncryptionMethod;    // 加密算法
    QString strCompressionMethod;    // 压缩算法
    int iVolumeSize;    // 分卷大小（kb）
    int iCompressionLevel;      // 压缩等级
    qlonglong qTotalSize; // 源文件总大小
    QString strDestination; // 压缩上级目录（若为空，代表在根目录压缩文件）
    bool bEncryption;       // 是否加密
    bool bHeaderEncryption;     // 是否列表加密
    bool bSplit;        // 是否分卷
    bool bTar_7z;       // 是否是tar.7z格式（补充）
    int iCPUTheadNum;        // CPU线程数
};
Q_DECLARE_METATYPE(CompressOptions)

// 解压选项
struct ExtractionOptions {
    ExtractionOptions()
        : qSize(0)
        , qComressSize(0)
    {
        bExistList = true;
        bAllExtract = false;
        bBatchExtract = false;
        bTar_7z = false;
        bOpen = false;
    }

    QString strTargetPath;      // 解压目标路径
    QString strDestination;     // 提取时的上级目录（若为空，代表提取的是根目录下的文件）
    QString password;           // 解压密码
    qint64 qSize;               // 解压:原始大小; 提取:待提取的总大小
    qint64 qComressSize;        // 压缩包大小
    bool bExistList;            // 是否有list过程
    bool bAllExtract;           // 是否全部解压（true：全部解压 false：提取）
    bool bBatchExtract;         // 是否批量解压
    bool bTar_7z;               // 是否是tar.7z格式（补充）
    bool bOpen;                 // 是否是用来打开
};
Q_DECLARE_METATYPE(ExtractionOptions)

// 更新选项
struct UpdateOptions {
    enum Type {
        Delete,     // 删除
        Rename,     // 重命名
        Add         // 追加
    };

    UpdateOptions()
    {
        reset();
    }

    void reset()
    {
        eType = Delete;
        strParentPath.clear();
        listEntry.clear();
        qSize = 0;
    }

    Type eType;                     // 更新类型
    QString strParentPath;          // 父目录（若为空，代表操作的是压缩包首层目录下的文件）
    QList<FileEntry> listEntry;     // 操作的文件（删除：存储选中的文件     追加：存储本地所有文件）
    qint64 qSize;                  // 操作的文件大小
};

/* 对一些未识别出来的类型进行区分
*  zip空压缩包：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/zip"
*  谷歌插件zip：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/x-chrome-extension"
*  谷歌插件crx：内容检测为"application/octet-stream"，后缀检测为"application/octet-stream"，file命令探测为"application/x-chrome-extension"
*  zip分卷包：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/octet-stream"
*/
class CustomMimeType
{
public:
    CustomMimeType() {}
    ~CustomMimeType() {}

    /**
     * @brief name  获取类型名
     * @return
     */
    QString name() const
    {
        if (m_bUnKnown) {
            return m_strTypeName;
        }

        return m_mimeType.name();
    }

    /**
     * @brief inherits      是否继承某个类型
     * @param strMimeType   父类型
     * @return
     */
    bool inherits(const QString &strMimeType) const
    {
        if (m_bUnKnown) {
            if (strMimeType == m_strTypeName)
                return true;

            return false;
        }

        return m_mimeType.inherits(strMimeType);
    }

public:
    bool m_bUnKnown = false;  // 是否自定义扩展类型（内容检测为"application/octet-stream"时使用strTypeName，其余情况使用mimeType）
    QMimeType m_mimeType;
    QString m_strTypeName;
};
Q_DECLARE_METATYPE(CustomMimeType)

#endif // COMMONSTRUCT_H
