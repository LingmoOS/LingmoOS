// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H
#include "dcompressornamespace.h"
#include <QObject>
#include <QMimeType>
#include <QFileInfo>
#include <QMutex>
#include <QMap>
class ReadOnlyArchiveInterface;

DCOMPRESSOR_BEGIN_NAMESPACE
class ArchiveJob;
class DArchiveManagerPrivate;
class DArchiveManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DArchiveManager)

public:
    enum ArchivePluginType {
        APT_Auto,
        APT_Cli7z,
        APT_Libarchive,
        APT_Libzip,
        APT_Libpigz
    };

    enum ArchiveJobType {
        JT_NoJob = 0,
        JT_Create,
        JT_Add,
        JT_Load,
        JT_Extract,
        JT_Delete,
        JT_Rename,
        JT_BatchExtract,
        JT_Open,
        JT_Update,
        JT_Comment,
        JT_Convert,
        JT_StepExtract
    };

    enum MgrPluginFinishType {
        PFT_Nomral = 0,
        PFT_Cancel,
        PFT_Error
    };

    enum MgrErrorType {
        ET_NoError = 0,
        ET_PluginError,
        ET_WrongPassword,
        ET_NeedPassword,
        ET_LongNameError,
        ET_ArchiveDamaged,
        ET_FileOpenError,
        ET_FileReadError,
        ET_FileWriteError,
        ET_DeleteError,
        ET_RenameError,
        ET_MissingVolume,
        ET_InsufficientDiskSpace,

        ET_UserCancelOpertion,
        ET_ExistVolume
    };

    struct MgrFileEntry
    {
        void reset();
        bool operator==(const MgrFileEntry &t) const;

        QString strFullPath;
        QString strFileName;
        QString strAlias;
        bool isDirectory;
        qlonglong qSize;
        uint uLastModifiedTime;
        int iIndex;
        MgrFileEntry();
    };

    struct MgrArchiveData
    {
        MgrArchiveData();
        void reset();

        qlonglong qSize;
        qlonglong qComressSize;
        QString strComment;
        QMap<QString, MgrFileEntry> mapFileEntry;
        QList<MgrFileEntry> listRootEntry;
        bool isListEncrypted;
        QString strPassword;
    };

    struct MgrCompressOptions
    {
        MgrCompressOptions();

        QString strPassword;
        QString strEncryptionMethod;
        QString strCompressionMethod;
        int iVolumeSize;
        int iCompressionLevel;
        qlonglong qTotalSize;
        QString strDestination;
        bool bEncryption;
        bool bHeaderEncryption;
        bool bSplit;
        bool bTar_7z;
        int iCPUTheadNum;
    };

    struct MgrExtractionOptions
    {
        MgrExtractionOptions();

        QString strTargetPath;
        QString strDestination;
        QString password;
        qint64 qSize;
        qint64 qComressSize;
        bool bExistList;
        bool bAllExtract;
        bool bBatchExtract;
        bool bTar_7z;
        bool bOpen;
    };

    struct MgrUpdateOptions
    {
        enum Type {
            Delete,
            Rename,
            Add
        };

        MgrUpdateOptions();
        void reset();

        Type eType;
        QString strParentPath;
        QList<MgrFileEntry> listEntry;
        qint64 qSize;
    };

    static DArchiveManager *get_instance(void);

    void destory_instance();

    bool createArchive(const QList<MgrFileEntry> &files, const QString &strDestination, const MgrCompressOptions &stOptions, ArchivePluginType eType);

    bool loadArchive(const QString &strArchiveFullPath, ArchivePluginType eType = ArchivePluginType::APT_Auto);

    MgrArchiveData *archiveData();

    bool addFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listAddEntry, const MgrCompressOptions &stOptions);

    bool extractFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &files, const MgrExtractionOptions &stOptions, ArchivePluginType eType = ArchivePluginType::APT_Auto);

    bool extractFiles2Path(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listSelEntry, const MgrExtractionOptions &stOptions);

    bool deleteFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listSelEntry);

    bool renameFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listSelEntry);

    bool batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath /*, bool bAutoCreatDir*/);

    bool openFile(const QString &strArchiveFullPath, const MgrFileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram);

    bool updateArchiveCacheData(const MgrUpdateOptions &stOptions);

    bool updateArchiveComment(const QString &strArchiveFullPath, const QString &strComment);

    bool convertArchive(const QString &strOriginalArchiveFullPath, const QString &strTargetFullPath, const QString &strNewArchiveFullPath);

    bool pauseOperation();

    bool continueOperation();

    bool cancelOperation();

    QString getCurFilePassword();

    bool currentStatus();

Q_SIGNALS:

    void signalError(MgrErrorType eErrorType);

    void signalJobFinished(ArchiveJobType eJobType, MgrPluginFinishType eFinishType, MgrErrorType eErrorType);

    void signalprogress(double dPercentage);

    void signalCurFileName(const QString &strName);

    void signalFileWriteErrorName(const QString &strName);

    void signalCurArchiveName(const QString &strArchiveName);

private:
    explicit DArchiveManager(QObject *parent = nullptr);
    ~DArchiveManager() override;

protected:
    QScopedPointer<DArchiveManagerPrivate> d_ptr;
};
DCOMPRESSOR_END_NAMESPACE
#endif   // ARCHIVEMANAGER_H
