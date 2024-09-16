// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "drawshape/cdrawparamsigleton.h"
#include "ddialog.h"
#include "frame/ccentralwidget.h"
#include "bzItems/cgraphicstextitem.h"
#include "widgets/ctextedit.h"
#include "application.h"

#include <DApplication>
#include <QGuiApplication>
#include <QDebug>
#include <QString>
#include <QFileInfo>
#include <DDialog>

#include <sys/inotify.h>

DWIDGET_USE_NAMESPACE

CManageViewSigleton *CManageViewSigleton::m_pInstance = nullptr;

CManageViewSigleton::CManageViewSigleton()
    : QObject()
{
    Q_UNUSED(getFileSrcData(""));
}

CManageViewSigleton *CManageViewSigleton::GetInstance()
{
    if (m_pInstance == nullptr) {
        m_pInstance  = new CManageViewSigleton();
    }
    return m_pInstance;
}

bool CManageViewSigleton::isEmpty()
{
    return m_allViews.isEmpty();
}

PageView *CManageViewSigleton::getCurView()
{
    PageView *curView = nullptr;

    //获取当前窗口
    if (!m_allViews.isEmpty() && m_curIndex >= 0 && m_curIndex < m_allViews.size()) {
        curView = m_allViews[m_curIndex];
    }

    return curView;
}

PageScene *CManageViewSigleton::getCurScene()
{
    auto view = getCurView();
    if (view != nullptr)
        return view->drawScene();
    return nullptr;
}

void CManageViewSigleton::addView(PageView *view)
{
    if (nullptr == view) {
        return;
    }
    if (!m_allViews.contains(view)) {
        m_allViews.append(view);
        //emit viewAdded(view);
    }
    if (m_curIndex == -1 && !m_allViews.isEmpty()) {
        m_curIndex = 0;
    }
}

void CManageViewSigleton::setCurView(PageView *view)
{
    if (nullptr == view) {
        return;
    }
    auto currentView = getCurView();
    if (currentView == view)
        return;

    if (m_allViews.contains(view)) {
        m_curIndex = m_allViews.indexOf(view);
        //emit viewChanged(currentView, view);
    }
    if (m_curIndex == -1 && !m_allViews.isEmpty()) {
        m_curIndex = 0;
    }
}

void CManageViewSigleton::removeView(PageView *view)
{
    if (nullptr == view) {
        return;
    }
    if (m_allViews.contains(view)) {
        auto curIndex = m_allViews.indexOf(view);
        m_allViews.removeAt(curIndex);

        QString ddfPath = /*view->getDrawParam()->getDdfSavePath()*/"";
        //removeWacthedFile(ddfPath);

        if (curIndex == m_curIndex) {
            if (m_allViews.isEmpty()) {
                m_curIndex = -1;
                //emit viewChanged(view, nullptr);
            } else {
                m_curIndex = 0;
                //emit viewChanged(view, m_allViews[m_curIndex]);
            }
        }
        //emit viewRemoved(view);
    }
}

//void CManageViewSigleton::updateBlockSystem()
//{
//    initBlockShutdown();
//    bool shutdownFlag = false;
//    for (auto view : m_allViews) {
//        if (view->isModified()) {
//            shutdownFlag = true;
//            break;
//        }
//    }
//    if (shutdownFlag) {
//        m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);
//    } else {
//        QDBusReply<QDBusUnixFileDescriptor> tmp = m_reply;
//        m_reply = QDBusReply<QDBusUnixFileDescriptor>();
//        //m_pLoginManager->callWithArgumentList(QDBus::NoBlock, "Inhibit", m_arg);
//        qDebug() << "Nublock shutdown.";
//    }
//}

//CGraphicsView *CManageViewSigleton::getViewByViewName(QString name)
//{
//    for (int i = 0; i < m_allViews.count(); i++) {
//        if (m_allViews[i]->getDrawParam()->viewName() == name) {
//            return m_allViews[i];
//        }
//    }

//    // 返回空指针
//    return nullptr;
//}

//CGraphicsView *CManageViewSigleton::getViewByViewModifyStateName(QString name)
//{
//    for (int i = 0; i < m_allViews.count(); i++) {
//        if (m_allViews[i]->getDrawParam()->getShowViewNameByModifyState() == name) {
//            return m_allViews[i];
//        }
//    }

//    // 返回空指针
//    return nullptr;
//}

int CManageViewSigleton::viewCount()
{
    return m_allViews.size();
}

void CManageViewSigleton::initBlockShutdown()
{
    if (!m_arg.isEmpty() || m_reply.value().isValid()) {
        qDebug() << "m_reply.value().isValid():" << m_reply.value().isValid();
        return;
    }

    m_pLoginManager = new QDBusInterface("org.freedesktop.login1",
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         QDBusConnection::systemBus());

    m_arg << QString("shutdown")             // what
          << qApp->productName()           // who
          << QObject::tr("File not saved")          // why
          << QString("block");                        // mode

    //int fd = -1;
    m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);
    if (m_reply.isValid()) {
        /*fd = */(void)m_reply.value().fileDescriptor();
    }
    //如果for结束则表示没有发现未保存的tab项，则放开阻塞关机
    if (m_reply.isValid()) {
        QDBusReply<QDBusUnixFileDescriptor> tmp = m_reply;
        m_reply = QDBusReply<QDBusUnixFileDescriptor>();
        //m_pLoginManager->callWithArgumentList(QDBus::NoBlock, "Inhibit", m_arg);
        qDebug() << "Nublock shutdown.";
    }
}

void CManageViewSigleton::quitIfEmpty()
{
    if (getCurView() == nullptr) {
        qApp->quit();
    }
}

QByteArray CManageViewSigleton::getFileSrcData(const QString &file)
{
    if (file.isEmpty())
        return QByteArray();
    QFile f(file);
    if (f.exists()) {
        return f.readAll();
    } else {
        qWarning() << QString("can not read, %1 is not exists!").arg(file);
    }
    return QByteArray();
}
CFileWatcher::CFileWatcher(QObject *parent)
    : QThread(parent)
{
    _handleId = inotify_init();
}

CFileWatcher::~CFileWatcher()
{
    clear();
}

bool CFileWatcher::isVaild()
{
    return (_handleId != -1);
}

void CFileWatcher::addWather(const QString &path)
{
    QMutexLocker loker(&_mutex);
    if (!isVaild())
        return;

    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        return;
    }

    if (watchedFiles.find(path) != watchedFiles.end()) {
        return;
    }

    std::string sfile = path.toStdString();
    int fileId = inotify_add_watch(_handleId, sfile.c_str(), IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);

    watchedFiles.insert(path, fileId);
    watchedFilesId.insert(fileId, path);

    if (!_running) {
        _running = true;
        start();
    }
}

void CFileWatcher::removePath(const QString &path)
{
    QMutexLocker loker(&_mutex);

    if (!isVaild())
        return;

    auto itf = watchedFiles.find(path);
    if (itf != watchedFiles.end()) {
        inotify_rm_watch(_handleId, itf.value());

        watchedFilesId.remove(itf.value());
        watchedFiles.erase(itf);
    }
}

void CFileWatcher::clear()
{
    QMutexLocker loker(&_mutex);

    for (auto it : watchedFiles) {
        inotify_rm_watch(_handleId, it);
    }
    watchedFilesId.clear();
    watchedFiles.clear();
}

void CFileWatcher::run()
{
    doRun();
}

void CFileWatcher::doRun()
{
    if (!isVaild())
        return;

    char name[1024];
    auto freadsome = [ = ](void *dest, size_t remain, FILE * file) {
        char *offset = reinterpret_cast<char *>(dest);
        while (remain) {
            size_t n = fread(offset, 1, remain, file);
            if (n == 0) {
                return -1;
            }

            remain -= n;
            offset += n;
        }
        return 0;
    };

    FILE *watcher_file = fdopen(_handleId, "r");

    while (true) {
        inotify_event event;
        if (-1 == freadsome(&event, sizeof(event), watcher_file)) {
            qWarning() << "------------- freadsome error !!!!!---------- ";
        }
        if (event.len) {
            freadsome(name, event.len, watcher_file);
        } else {
            QMutexLocker loker(&_mutex);
            auto itf = watchedFilesId.find(event.wd);
            if (itf != watchedFilesId.end()) {
                qDebug() << "file = " << itf.value() << " event.wd = " << event.wd << "event.mask = " << event.mask;

                if (event.mask & IN_MODIFY) {
                    emit fileChanged(itf.value(), EFileModified);
                } else if (event.mask & IN_MOVE_SELF) {
                    emit fileChanged(itf.value(), EFileMoved);
                } else if (event.mask & IN_DELETE_SELF) {
                    emit fileChanged(itf.value(), EFileMoved);
                }
            }
        }
    }
}

