// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UISTRUCT_H
#define UISTRUCT_H

#include "dcompressornamespace.h"
#include <DStandardPaths>
#include <QString>
#include <QMap>
#include <QDir>

#define TEMPPATH DStandardPaths::writableLocation(QStandardPaths::TempLocation)
#define MAINWINDOW_WIDTH_NAME "MainWindowWidthName"
#define MAINWINDOW_HEIGHT_NAME "MainWindowHeightName"
#define HISTORY_DIR_NAME "dir"
#define MAINWINDOW_DEFAULTW 620
#define MAINWINDOW_DEFAULTH 465
#define MAXCOMMENTLEN 10000

DCOMPRESSOR_BEGIN_NAMESPACE

enum Progress_Type {
    PT_None = 0,
    PT_Compress,
    PT_UnCompress,
    PT_Delete,
    PT_Rename,
    PT_CompressAdd,
    PT_Convert,
    PT_Comment,
};

enum Archive_OperationType {
    Operation_NULL,
    Operation_Load,
    Operation_Create,
    Operation_Extract,
    Operation_SingleExtract,
    Operation_TempExtract_Open,
    Operation_DRAG,
    Operation_CONVERT,
    Operation_Add,
    Operation_DELETE,
    Operation_Update_Comment,
    Operation_Add_Comment,
    Operation_UpdateData
};

enum DataView_Column {
    DC_Name,
    DC_Time,
    DC_Type,
    DC_Size,
};

enum Page_ID {
    PI_Home = 0,
    PI_Compress,
    PI_CompressSetting,
    PI_UnCompress,
    PI_CompressProgress,
    PI_AddCompressProgress,
    PI_UnCompressProgress,
    PI_DeleteProgress,
    PI_RenameProgress,
    PI_ConvertProgress,
    PI_CommentProgress,
    PI_Success,
    PI_Failure,
    PI_Loading
};

struct CompressParameter
{
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

    QString strMimeType;
    QString strArchiveName;
    QString strTargetPath;
    QString strPassword;
    QString strEncryptionMethod;
    QString strCompressionMethod;
    bool bEncryption = false;
    bool bHeaderEncryption;
    bool bSplit;
    bool bTar_7z;
    int iCPUTheadNum;
    int iVolumeSize;
    int iCompressionLevel;
    qint64 qSize;
    QStringList listCompressFiles;
};

struct UnCompressParameter
{

    enum SplitType {
        ST_No,
        ST_Zip,
        ST_Other
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

    QString strFullPath;
    QString strExtractPath;
    SplitType eSplitVolume = UnCompressParameter::SplitType::ST_No;
    bool bMultiplePassword;
    bool bModifiable;
    bool bCommentModifiable;
    qint64 qSize;
    QList<QString> listExractFiles;

    QStringList listBatchFiles;
    bool bBatch;
};

enum SuccessInfo {
    SI_Compress,
    SI_UnCompress,
    SI_Convert
};

enum FailureInfo {
    FI_Compress = 0,
    FI_Load,
    FI_Uncompress,
    FI_Add,
    FI_Delete,
    FI_Rename,
    FI_Convert
};

enum ErrorInfo {
    EI_NoPlugin = 0,
    EI_ArchiveDamaged,
    EI_ArchiveMissingVolume,
    EI_WrongPassword,
    EI_LongFileName,
    EI_CreatFileFailed,
    EI_CreatArchiveFailed,
    EI_InsufficientDiskSpace,
    EI_ArchiveNoData,
    EI_ExistVolume
};

enum StartupType {
    ST_Normal,
    ST_Compress,
    ST_Compresstozip7z,
    ST_Extract,
    ST_ExtractHere,
    ST_Extractto,
    ST_DragDropAdd
};
DCOMPRESSOR_END_NAMESPACE
Q_DECLARE_METATYPE(DCOMPRESSOR_NAMESPACE::CompressParameter)
Q_DECLARE_METATYPE(DCOMPRESSOR_NAMESPACE::UnCompressParameter)

#endif   // CUSTOMDATAINFO_H
