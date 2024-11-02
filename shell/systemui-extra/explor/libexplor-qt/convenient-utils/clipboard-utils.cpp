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

#include "clipboard-utils.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QStandardPaths>

#include "file-operation-manager.h"
#include "file-move-operation.h"
#include "file-copy-operation.h"
#include "sound-effect.h"

#include <QDir>
#include <QTimer>
#include <glib.h>
#include <glib/gi18n.h>
#include <X11/X.h>
#include <X11/Xlib.h>

using namespace Peony;

static ClipboardUtils *global_instance = nullptr;

static QStringList gClipboardFileUrls;
static QMutex gClipboardFileUrlsMutex;
static QList<quint64> gClipbordFileinode;
static QAtomicInt gRemoteCurrentCount = 0;
static bool gIsRemoteUri = false;
static QStringList gCutFileUris;

/*!
 * \brief m_clipboard_parent_uri
 * \note
 * Once we close the current application, the qt will take over the clipboard data.
 * If we open the application, the program will query the clipboard cross the process.
 * This will cost much more time than the first time we open the application.
 * This variable is used to make sure that we always get the clipboard data from current
 * progress when we get the files from clipboard utils.
 */
static QString m_clipboard_parent_uri = nullptr;

static bool m_is_desktop_cut = false;
static bool m_is_explor_cut = false;

static QList<QString> m_target_directory_uri;


ClipboardUtils *ClipboardUtils::getInstance()
{
    if (!global_instance) {
        global_instance = new ClipboardUtils;
    }
    return global_instance;
}

ClipboardUtils::ClipboardUtils(QObject *parent) : QObject(parent)
{
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &ClipboardUtils::clipboardChanged);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, [=]() {
        auto data = QApplication::clipboard()->mimeData();
        if (!data->hasFormat("explor-qt/is-cut")) {
            m_clipboard_parent_uri = nullptr;
            gCutFileUris.clear();
        }

        if (!QApplication::clipboard()->ownsClipboard()) {
            gCutFileUris.clear();
        }
    });

    connect (ClipboardThread::getInstance (), &ClipboardThread::startOp, this, [=] (QStringList& str, bool mIsMove) {
        qDebug() << "start 云桌面 uri 获取成功!!!" << str;
        if (str.isEmpty ()) return ;

        QString target = ClipboardThread::getInstance ()->getTargetUri ();
        qDebug() << "start 云桌面 uri 开始粘贴" << str << "  ==>  " << target;

        auto fileOpMgr = FileOperationManager::getInstance();
        if (mIsMove) {
            auto moveOp = new FileMoveOperation(str, target);
            moveOp->setAction(Qt::TargetMoveAction);
            fileOpMgr->startOperation(moveOp, true);
            QApplication::clipboard()->clear();
        } else {
            auto copyOp = new FileCopyOperation(str, target);
            fileOpMgr->startOperation(copyOp, true);
        }
        }, Qt::BlockingQueuedConnection);

    onClipboardDataChanged ();
}

ClipboardUtils::~ClipboardUtils()
{
    m_target_directory_uri.clear();
}

void ClipboardUtils::release()
{
    delete global_instance;
}

const QString ClipboardUtils::getClipedFilesParentUri()
{
    return m_clipboard_parent_uri;
}

QStringList ClipboardUtils::getCutFileUris()
{
    return gCutFileUris;
}

const QString ClipboardUtils::getLastTargetDirectoryUri()
{
    return m_target_directory_uri.size() > 0 ? m_target_directory_uri.back() : "";
}

void ClipboardUtils::updateClipboardManually(bool force)
{
    Q_UNUSED (force)
}

void ClipboardUtils::onClipboardDataChanged()
{
    {
        QMutexLocker lk(&gClipboardFileUrlsMutex);
        gClipboardFileUrls.clear();
    }

    gClipboardFileUrls.clear();

    const QMimeData *mimeData = qApp->clipboard()->mimeData();
    if (!mimeData || mimeData->formats().isEmpty()) {
        qWarning() << "get null mimeData from QClipBoard or remote formats is null!";
        return;
    }

    qDebug() << "onClipboardDataChanged:" << mimeData->formats ();

    if (mimeData->hasFormat("uos/remote-copy")) {
        qInfo() << "clipboard use other !";
        gIsRemoteUri = true;
        ++gRemoteCurrentCount;
        return;
    } else {
        qDebug() << "has no uos/remote-copy";
    }
}

void ClipboardUtils::setClipboardFiles(const QStringList &uris, bool isCut)
{
    setClipboardFiles(uris, isCut, false);
}

void ClipboardUtils::setClipboardFiles(const QStringList &uris, bool isCut, bool isSearch)
{
    if (!global_instance) {
        global_instance = new ClipboardUtils;
    }

    if (uris.isEmpty()) {
        clearClipboard();
        return;
    }

    if (isCut)
        gCutFileUris = uris;
    else
        gCutFileUris.clear();

    m_clipboard_parent_uri = FileUtils::getParentUri(uris.first());
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (FileUtils::isSamePath(m_clipboard_parent_uri, desktopPath) && isCut){
         m_is_desktop_cut = true;
         m_is_explor_cut = true;
    }
    else if (isCut)
    {
        m_is_desktop_cut = false;
        m_is_explor_cut = true;
    }else{
        m_is_desktop_cut = false;
        m_is_explor_cut = false;
    }

    // we should remain the encoded uri for file operation
    auto data = new QMimeData;
    QVariant isCutData = QVariant(isCut);
    data->setData("explor-qt/is-cut", isCutData.toByteArray());

    QList<QUrl> urls;
    QStringList encodedUris;
    for (auto uri : uris) {
        auto encodeUrl = Peony::FileUtils::urlEncode(uri);
        urls << QString(encodeUrl);
        encodedUris << QString(encodeUrl);
    }
    data->setUrls(urls);
//    QString string = encodedUris.join(" ");
//    data->setData("explor-qt/encoded-uris", string.toUtf8());
//    data->setText(string);

    QString text;
    QByteArray target = (isCut) ? "cut" : "copy";
    for (const QUrl &qurl : urls) {
        const QString &path = qurl.toLocalFile();
        if (!path.isEmpty()) {
            text += path + '\n';
        }
        target.append("\n");
        target.append(qurl.toString());
    }
    data->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    data->setData("x-special/gnome-copied-files", target);
    QVariant isSearchData = QVariant(isSearch);
    data->setData("explor-qt/is-search", isSearchData.toByteArray());

    QApplication::clipboard()->setMimeData(data);
}

bool ClipboardUtils::isClipboardHasFiles()
{
    return QApplication::clipboard()->mimeData()->hasUrls()|| qApp->clipboard()->mimeData()->hasFormat ("uos/remote-copy");
}

bool ClipboardUtils::isDesktopFilesBeCut()
{
    return m_is_desktop_cut;
}

bool ClipboardUtils::isPeonyFilesBeCut()
{
    return m_is_explor_cut;
}

bool ClipboardUtils::isSearchTab()
{
    bool isSearchTab = false;
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (mimeData && mimeData->hasFormat("explor-qt/is-search")) {
        isSearchTab = QVariant(mimeData->data("explor-qt/is-search")).toBool();
    }
    return isSearchTab;
}

bool ClipboardUtils::isClipboardFilesBeCut()
{
    if (isClipboardHasFiles()) {
        auto data = QApplication::clipboard()->mimeData();
        if (data->hasFormat("explor-qt/is-cut")) {
            QVariant var(data->data("explor-qt/is-cut"));
            return var.toBool();
        }
    }
    return false;
}

QStringList ClipboardUtils::getClipboardFilesUris()
{
    QStringList l;

    if (!isClipboardHasFiles()) {
        return l;
    }

    auto mimeData = QApplication::clipboard()->mimeData();
    //auto text = mimeData->text();
    auto explorText = mimeData->data("explor-qt/encoded-uris");

    if (!explorText.isEmpty()) {
        qDebug() << "explor text:" << explorText;
        auto byteArrays = explorText.split(' ');
        for (auto byteArray : byteArrays) {
            l<<byteArray;
        }
    } else {
        auto urls = mimeData->urls();
        for (auto url : urls) {
            // fix #144280, shenxinfu virual machine copy failed
            if (url.toString().count() < 5) {
                qWarning()<<url<<"is not standard uri, skip...";
                continue;
            }
            g_autofree gchar* uri = g_uri_unescape_string(url.toString().toUtf8().constData(), nullptr);
            if (uri) {
               l<<QString(uri);
            } else {
                qWarning()<<"can not unescape uri:"<<url.toString().toUtf8().constData();
                l<<url.toString().toUtf8().constData();
            }
        }
    }

    return l;
}

FileOperation *ClipboardUtils::pasteClipboardFiles(const QString &targetDirUri)
{
    FileOperation *op = nullptr;
//    if (!isClipboardHasFiles()) {
//        return op;
//    }

    // 针对云桌面做定制
    QStringList uris;
    ClipboardUtils::getInstance ()->onClipboardDataChanged ();
    ClipboardThread* t = ClipboardThread::getInstance ();

    qDebug() << "gIsRemoteUri:" << gIsRemoteUri << "  t->isrunning: " << t->isRunning ();

    // 这里开始执行上传
    if (gIsRemoteUri && !t->isRunning ()) {
        qDebug() << "Sangfor- start 云桌面 uri 获取" << uris;
        t->setTargetUri (targetDirUri);
        t->start (QThread::NormalPriority);
        qDebug () << "Sangfor- 开始获取云桌面 uri, 返回";
        return op;
    }

    qDebug() << "pasteClipboardFiles" << uris;
    uris = getClipboardFilesUris ();
    if (uris.isEmpty()) {
        qDebug() << "return, op uris is null";
        return op;
    }

    auto parentPath = FileUtils::getParentUri(uris.first());
    //paste file in old path, return op
    if (FileUtils::isSamePath(parentPath, targetDirUri) && isClipboardFilesBeCut())
    {
        clearClipboard();
        return op;
    }

    auto fileOpMgr = FileOperationManager::getInstance();
    if (isClipboardFilesBeCut()) {
        //fix bug#162024, play sound when operation finished, comment this
        //SoundEffect::getInstance()->copyOrMoveSucceedMusic();
        qDebug()<<uris;
        auto moveOp = new FileMoveOperation(uris, targetDirUri);
        moveOp->setAction(Qt::TargetMoveAction);
        moveOp->setSearchOperation(isSearchTab());
        op = moveOp;
        fileOpMgr->startOperation(moveOp, true);
        QApplication::clipboard()->clear();
    } else {
        //fix bug#162024, play sound when operation finished, comment this
        //SoundEffect::getInstance()->copyOrMoveSucceedMusic();
        qDebug() << "clipboard:" << uris;
        auto copyOp = new FileCopyOperation(uris, targetDirUri);
        op = copyOp;
        fileOpMgr->startOperation(copyOp, true);
    }

    if (m_target_directory_uri.size() <= 0 || m_target_directory_uri.back() != targetDirUri) {
        m_target_directory_uri.append(targetDirUri);
    }


    if (m_target_directory_uri.size() > 2) {
        m_target_directory_uri.pop_front();
    }

    if (op) {
        gCutFileUris.clear();
    }
    return op;
}

void ClipboardUtils::clearClipboard()
{
    m_is_desktop_cut = false;
    m_is_explor_cut = false;
    QApplication::clipboard()->clear();
    gCutFileUris.clear();
}

void ClipboardUtils::popLastTargetDirectoryUri(QString &uri)
{
    if (m_target_directory_uri.size() > 0 && uri == m_target_directory_uri.back()) {
        m_target_directory_uri.pop_back();
    }
}

ClipboardThread* ClipboardThread::gInstance = nullptr;

ClipboardThread::ClipboardThread()
{
}

ClipboardThread::~ClipboardThread()
{
}

QStringList ClipboardThread::getUrlsByX11()
{
    QAtomicInt currentCount = gRemoteCurrentCount;
    const QMimeData *mimedata = qApp->clipboard()->mimeData();
    if (!mimedata) {
        qWarning() << "the clipboard mimedata is invalid!";
        return QStringList ();
    }

    bool isWayland = qApp->property("isWayland").toBool();
    if (!gIsRemoteUri || isWayland) {
        qWarning() << "不是云粘贴板或者是Wayland环境："<<gIsRemoteUri<<isWayland;
        return QStringList();
    }

    //使用x11创建一个窗口去阻塞获取URl
    Display *display = XOpenDisplay(nullptr);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);

    char *result = nullptr;
    unsigned long ressize = 0, restail = 0;
    int resbits;
    Atom bufid = XInternAtom(display, "CLIPBOARD", False),
        fmtid = XInternAtom(display, "text/uri-list", False),
        propid = XInternAtom(display, "XSEL_DATA", False),
        incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    QList<QUrl> urls;
    QString results;
    QAtomicInteger<bool> isCanceled = false;

    XSelectInput (display, window, PropertyChangeMask);
    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    do {
        XNextEvent(display, &event);
        qInfo () << "event-type:" << event.type << " -- " << event.xselection.selection;
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);
    if (event.xselection.property) {
        XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, True, AnyPropertyType,
                           &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);
        if (fmtid != incrid) {
            qInfo() << QString(result);
            urls += QUrl::fromStringList(QString(result).split("\n"));
        }

        XFree(result);

        if (fmtid == incrid) {
            do {
                do {
                    XNextEvent(display, &event);
                    if (event.type == SelectionNotify) {
                        isCanceled = true;
                        qDebug() << "选中改变";
                        break;
                    }
                    qInfo () << "11 event-type:" << event.type << " -- " << event.xselection.selection;
                } while (event.type != PropertyNotify || event.xproperty.atom != propid || event.xproperty.state != PropertyNewValue);
                if (isCanceled)
                    break;

                XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, True, AnyPropertyType, &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);
                if (QString(result) != "/")
                    results += QString(result);
                XFree(result);
            } while (ressize > 0);
        }
    }
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    if (isCanceled) {
        gIsRemoteUri = false;
        qWarning() << "user cancel remote download !";
        return QStringList ();
    }
    urls += QUrl::fromStringList(results.split("\n"));

    QStringList clipboardFileUrls;
    for (QUrl url : urls) {
        //链接文件的inode不加入clipbordFileinode，只用url判断clip，避免多个同源链接文件的逻辑误判
        if (url.toString() == "copy") {
            mIsMove = false;
            continue;
        }

        QString path = url.path();
        path = path.replace(QRegExp("/*/"),"/");
        if (path.isEmpty() || path == "/")
            continue;
        QUrl temp;
        temp.setScheme("file");
        temp.setPath(path);
        clipboardFileUrls.append (temp.toString ());
    }
    qInfo() << results << urls << clipboardFileUrls;

    if (gIsRemoteUri && !clipboardFileUrls.isEmpty () && currentCount == gRemoteCurrentCount) {
        QMutexLocker lk(&gClipboardFileUrlsMutex);
        gIsRemoteUri = false;
        gClipboardFileUrls.clear();
        gClipboardFileUrls = clipboardFileUrls;
        gRemoteCurrentCount = 0;
        qDebug() << "read OK!!";
    }

    return clipboardFileUrls;
}

void ClipboardThread::run ()
{
    mIsMove = true;
    mUris.clear ();

    bool isWayland = qApp->property("isWayland").toBool();
    if (isWayland)
        return;

    qDebug() << "ClipboardThread";
    do {
        mUris = getUrlsByX11 ();
    } while (gIsRemoteUri);

    qDebug() << "getUrlsByX11:" << "  ==  " << mUris;

    if (!mUris.isEmpty ()) {
        qDebug() << "gClipboardFileUrls ==>:" << gClipboardFileUrls << "  ==>:" << mUris;
        gClipboardFileUrls = mUris;
        Q_EMIT startOp (mUris, mIsMove);
    }
}

void ClipboardThread::setTargetUri(const QString &u)
{
    if (!u.isNull () && !u.isEmpty ()) {
        mTargetUri = u;
    }
}

QString ClipboardThread::getTargetUri()
{
    return mTargetUri;
}

ClipboardThread* ClipboardThread::getInstance()
{
    if (!gInstance) {
        gInstance = new ClipboardThread;
    }

    return gInstance;
}
