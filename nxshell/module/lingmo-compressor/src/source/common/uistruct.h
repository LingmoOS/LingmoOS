// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UISTRUCT_H
#define UISTRUCT_H

#include <DStandardPaths>

#include <QString>
#include <QMap>
#include <QDir>

#define TEMPPATH DStandardPaths::writableLocation(QStandardPaths::TempLocation)     // 临时路径（打开等操作）
#define MAINWINDOW_WIDTH_NAME "MainWindowWidthName"      // 主界宽
#define MAINWINDOW_HEIGHT_NAME "MainWindowHeightName"    // 主界面高
#define HISTORY_DIR_NAME "dir"      // 历史打开路径
#define MAINWINDOW_DEFAULTW 620     // 默认宽度
#define MAINWINDOW_DEFAULTH 465     // 默认高度
#define MAXCOMMENTLEN 10000         // 压缩包注释内容字数限制

// 进度类型
enum Progress_Type {
    PT_None = 0,            // 无进度
    PT_Compress,     // 压缩进度
    PT_UnCompress,   // 解压缩进度
    PT_Delete,       // 删除进度
    PT_Rename,       // 重命名进度
    PT_CompressAdd, // 追加压缩进度
    PT_Convert,         // 转换格式进度
    PT_Comment,  // 压缩后添加注释进度
};

/**
 * @brief The Archive_OperationType enum
 * 针对压缩包的操作，比如解压、提取、打开内容等
 * 不同的操作可能共用相同的job
 */
enum Archive_OperationType {
    Operation_NULL, // 无操作
    Operation_Load, // 加载
    Operation_Create, // 创建压缩包
    Operation_Extract, // 解压
    Operation_SingleExtract, // 提取
    Operation_TempExtract_Open, // 打开
    Operation_DRAG, // 拖拽
    Operation_CONVERT, // 格式转换
    Operation_Add, // 追加
    Operation_DELETE, // 删除
    Operation_Update_Comment, // 更新压缩包注释
    Operation_Add_Comment, // 添加zip压缩包注释
    Operation_UpdateData // 更新压缩包数据
};

// 压缩/解压列表的列号
enum DataView_Column {
    DC_Name,            // 名称
    DC_Time,            // 修改时间
    DC_Type,            // 类型
    DC_Size,            // 大小
};

// 界面标识
enum Page_ID {
    PI_Home = 0,                // 首页
    PI_Compress,            // 压缩列表
    PI_CompressSetting,     // 压缩设置
    PI_UnCompress,          // 解压列表
    PI_CompressProgress,    // 压缩进度
    PI_AddCompressProgress, // 追加进度
    PI_UnCompressProgress,  // 解压进度
    PI_DeleteProgress,      // 删除进度
    PI_RenameProgress,      // 重命名进度
    PI_ConvertProgress,     // 转换进度
    PI_CommentProgress,     // 添加zip注释进度
    PI_Success,             // 成功
    PI_Failure,             // 失败
    PI_Loading              // 加载
};

// 压缩参数
struct CompressParameter {
    CompressParameter()
        : qSize(0)
    {
        bEncryption = false;
        bHeaderEncryption = false;
        bSplit = false;
        iVolumeSize = 0;
        iCompressionLevel = -1;
        bTar_7z = false;
        iCPUTheadNum = 1;
    }

    QString strMimeType;    // 格式类型（application/x-tar）
    QString strArchiveName; // 压缩包名称(无路径)
    QString strTargetPath;  // 保存路径
    QString strPassword;        // 密码
    QString strEncryptionMethod;    // 加密算法
    QString strCompressionMethod; //压缩算法
    bool bEncryption = false;       // 是否加密
    bool bHeaderEncryption;     // 是否列表加密
    bool bSplit;        // 是否分卷
    bool bTar_7z;       // 是否是tar.7z格式（补充）
    int iCPUTheadNum;        // 线程数
    int iVolumeSize;    // 分卷大小
    int iCompressionLevel;      // 压缩等级
    qint64 qSize;       // 文件总大小
    QStringList listCompressFiles;  // 待压缩文件（快捷操作时使用）
};
Q_DECLARE_METATYPE(CompressParameter)

// 解压参数（选择压缩包之后分析的数据）
struct UnCompressParameter {
    // 分卷类型
    enum SplitType {
        ST_No, // 非分卷
        ST_Zip, // zip分卷
        ST_Other // 其他分卷
    };

    UnCompressParameter()
        : qSize(0)
    {
        eSplitVolume = ST_No;
        bMultiplePassword = false;
        bModifiable = false;
        bCommentModifiable = false;
        listExractFiles.clear();
        listBatchFiles.clear();
        bBatch = false;
    }

    QString strFullPath;        // 压缩包全路径
    QString strExtractPath;     // 解压路径
    SplitType eSplitVolume = UnCompressParameter::SplitType::ST_No;          // 分卷包类型
    bool bMultiplePassword;     // 是否支持多密码追加
    bool bModifiable;           // 是否更改压缩包数据
    bool bCommentModifiable;    // 是否支持注释更改
    qint64 qSize;               // 压缩包大小
    QList<QString> listExractFiles; // 存储提取文件,用来结束之后自动打开文件夹时选中

    QStringList listBatchFiles; // 批量解压压缩包全路径
    bool bBatch;       // 是否批量

};
Q_DECLARE_METATYPE(UnCompressParameter)

// 成功界面显示的信息
enum SuccessInfo {
    SI_Compress,    // 压缩成功
    SI_UnCompress,  // 解压成功
    SI_Convert      // 转换成功
};

// 失败界面信息
enum FailureInfo {
    FI_Compress = 0,     // 压缩失败
    FI_Load,         // 打开失败
    FI_Uncompress,   // 解压失败（包括解压、提取、打开文件）
    FI_Add,          // 追加失败
    FI_Delete,       // 删除失败
    FI_Rename,       // 重命名失败
    FI_Convert       // 转换失败
};

// 错误界面显示的信息
enum ErrorInfo {
    EI_NoPlugin = 0,                // 无可用插件
    EI_ArchiveDamaged,          // 压缩包损坏
    EI_ArchiveMissingVolume,    // 分卷包缺失
    EI_WrongPassword,           // 密码错误
    EI_LongFileName,            // 文件名过长
    EI_CreatFileFailed,         // 创建文件失败
    EI_CreatArchiveFailed,      // 创建压缩文件失败
    EI_InsufficientDiskSpace,      // 磁盘空间不足
    EI_ArchiveNoData,           // 压缩包无数据
    EI_ExistVolume,      // 分卷已存在
};

// 启动应用的方式
enum StartupType {
    ST_Normal,  // 右键打开、双击打开、打开单压缩包
    ST_Compress,    // 右键压缩
    ST_Compresstozip7z, // 右键压缩成7z、zip
    ST_Extract, // 右键解压
    ST_ExtractHere, // 右键解压到当前文件夹
//    ST_ExtractMulti, // 右键解压多压缩包
    ST_Extractto,   // 右键解压到指定目录（指定目录）
    ST_DragDropAdd, // 拖拽追加
};

#endif // CUSTOMDATAINFO_H
