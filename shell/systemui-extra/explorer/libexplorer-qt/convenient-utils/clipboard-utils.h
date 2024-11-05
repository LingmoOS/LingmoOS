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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef CLIPBOARDUTILS_H
#define CLIPBOARDUTILS_H

#include <QObject>
#include <QThread>
#include "explorer-core_global.h"

namespace Peony {

class FileOperation;

/*!
 * \brief The ClipboardUtils class, a convinet class to access with QClipboard instance.
 * \details
 * This class provide some nessersary method which other libexplorer-qt's class need.
 * For example, isClipboardFilesBeCut is used in Peony::DirectoryView::IconView.
 * IconViewDelegate paint the cut files with different opacity. The paint event
 * is triggered by clipboardChanged() signal.
 * \todo
 * Automatically detect the duplicated copy/paste and handle in backend.
 */
class PEONYCORESHARED_EXPORT ClipboardUtils : public QObject
{
    Q_OBJECT
public:
    static ClipboardUtils *getInstance();
    void release();

    /*!
     * \brief ClipboardUtils::setClipboardFiles
     * \param uris
     * \param isCut
     * \details
     * QClipboard doesn't have a concept of 'cut'. This concept would be used
     * in explorer-qt for judge how we deal with the files in clipboard when we do
     * a 'paste' (copy/move).
     * \see ClipboardUtils::isClipboardFilesBeCut
     */
    static void setClipboardFiles(const QStringList &uris, bool isCut);
    static void setClipboardFiles(const QStringList &uris, bool isCut, bool isSearch);
    static bool isClipboardHasFiles();
    /*!
     * \brief isClipboardFilesBeCut
     * \return
     * \retval true if you use setClipboardFiles and pass the isCut argument as 'true',
     * otherwise false.
     */
    static bool isClipboardFilesBeCut();
    static bool isDesktopFilesBeCut();
    static bool isPeonyFilesBeCut();
    static bool isSearchTab();/* 是否在搜索tab中执行剪切或者移动操作 */
    static QStringList getClipboardFilesUris();
    static FileOperation *pasteClipboardFiles(const QString &targetDirUri);
    static void clearClipboard();
    static void popLastTargetDirectoryUri(QString& uri);
    static const QString getClipedFilesParentUri();

    QStringList getCutFileUris();

    const QString getLastTargetDirectoryUri();

    /*!
     * \brief updateClipboardManually
     * \param force
     * \deprecated
     * this method is depcreated and won't effect
     */
    void updateClipboardManually(bool force = false);

public Q_SLOTS:
    void onClipboardDataChanged ();

Q_SIGNALS:
    void clipboardChanged();

private:
    explicit ClipboardUtils(QObject *parent = nullptr);
    ~ClipboardUtils();
};

// 针对云桌面进行定制
// 临时解决方案
class ClipboardThread : public QThread
{
    Q_OBJECT
public:
    void run () override;
    void setTargetUri (const QString& u);

    QString getTargetUri ();
    static ClipboardThread* getInstance ();

Q_SIGNALS:
    void startOp (QStringList& str, bool isMove);

private:
    explicit ClipboardThread ();
    ~ClipboardThread ();
    QStringList getUrlsByX11 ();
    explicit ClipboardThread (ClipboardThread&) = delete;

private:
    bool mIsMove = false; // false 表示Copy true 表示 Move

    QString mTargetUri;
    QStringList mUris;

    static ClipboardThread* gInstance;
};

}

#endif // CLIPBOARDUTILS_H
