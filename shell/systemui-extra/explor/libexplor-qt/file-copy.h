/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */
#ifndef FILECOPY_H
#define FILECOPY_H

#include <QMutex>
#include <QObject>
#include <gio/gio.h>

#include <explor-core_global.h>

namespace Peony
{
/**
 * @brief Copy the file to the specified folder.
 * @note The source file must be a file, not a folder,
 */
class FileInfo;
class PEONYCORESHARED_EXPORT FileCopy : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        INVALID,
        PAUSE,
        RESTART,
        CANCEL,
        RUNNING,
        RESUME,
        FINISHED,
        ERROR
    };
    Q_ENUM(Status)
    explicit FileCopy (QString srcUri, QString destUri, GFileCopyFlags flags, GCancellable* cancel, GFileProgressCallback cb, gpointer pcd, GError** error, QObject* obj = nullptr);
    ~FileCopy();
    /**
     * @brief 整个复制流程在这里
     */
    void run();

    void setParentFlags(GFileCopyFlags parentFlags);

public Q_SLOTS:
    /**
     * @brief pause
     */
    void pause();
    /**
     * @brief resume
     */
    void resume();

    /**
     * @brief cancel
     */
    void cancel();

    Status getStatus();

private Q_SLOTS:

private:
    void updateProgress () const;
    void detailError (GError** error);
    void sync(const GFile* destFile);
    int doCopyBigFile(const char *srcPath, const char *destPath);
    bool isFileOnLocal(const GFile* destFile);
private:
    QMutex                          mPause;
    QString                         mSrcUri = nullptr;
    QString                         mDestUri = nullptr;

    GFileProgressCallback           mProgress;
    GFileCopyFlags                  mCopyFlags;
    GFileCopyFlags                  mParentFlags;
    GCancellable*                   mCancel = nullptr;          // temp param

    GError**                        mError = nullptr;           // temp param
    gpointer                        mProgressData;

    goffset                         mOffset = 0;                // 记录当前进度
    goffset                         mTotalSize = 0;             // 记录当前进度
    enum Status                     mStatus = INVALID;          // 记录运行状态
    bool                            mIsDestFileLocal = false;
};
};

#endif // FILECOPY_H
