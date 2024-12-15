// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMANAGEVIEWSIGLETON_H
#define CMANAGEVIEWSIGLETON_H

#include <QList>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QFileSystemWatcher>
#include <QAtomicInt>
#include <QDebug>
#include <DDialog>
#include <QThread>
#include <QMutex>

DWIDGET_USE_NAMESPACE

class PageView;
class CFileWatcher;

class PageScene;

#define CURRENTVIEW  (CManageViewSigleton::GetInstance()->getCurView())
#define CURRENTSCENE (CManageViewSigleton::GetInstance()->getCurScene())
#define appCurView  CURRENTVIEW
#define appCurScene CURRENTSCENE
class CManageViewSigleton: public QObject
{
    Q_OBJECT
private :
    static CManageViewSigleton *m_pInstance;
    CManageViewSigleton();

public :
    static CManageViewSigleton *GetInstance();

public:
    /**
     * @brief isEmpty 是否是空的(是否没有view)
     */
    bool isEmpty();
    /**
     * @brief getCurView 获取窗口
     */
    PageView *getCurView();

    /**
     * @brief getCurScene 获取当前的场景
     */
    PageScene  *getCurScene();
    /**
     * @brief addView 添加窗口
     */
    void addView(PageView *view);
    /**
     * @brief setCurView 设置当前窗口
     */
    void setCurView(PageView *view);
    /**
     * @brief setCurView 删除窗口
     */
    void removeView(PageView *view);
    /**
     * @brief CheckIsModify 检测是否修改
     */
    //void updateBlockSystem();

    int  viewCount();

    void quitIfEmpty();

    QByteArray getFileSrcData(const QString &file);

private:

    /**
     * @brief initBlockShutdown 柱塞关机
     */
    void initBlockShutdown();

private:
    //所有窗口
    QList<PageView *> m_allViews;
    //当前索引
    int m_curIndex = -1;

    QDBusReply<QDBusUnixFileDescriptor> m_reply;
    QDBusInterface *m_pLoginManager = nullptr;
    QList<QVariant> m_arg;

    QFileSystemWatcher m_ddfWatcher;
};

class CFileWatcher: public QThread
{
    Q_OBJECT
public:
    enum EFileChangedType {EFileModified, EFileMoved, EFileCount};

    explicit CFileWatcher(QObject *parent = nullptr);
    ~CFileWatcher();

    bool isVaild();

    void addWather(const QString &path);
    void removePath(const QString &path);

    void clear();

signals:
    void fileChanged(const QString &path, int tp);

protected:
    void run();

private:
    void doRun();

    int  _handleId = -1;
    bool _running = false;


    QMap<QString, int> watchedFiles;
    QMap<int, QString> watchedFilesId;

    QMutex _mutex;
};

#endif // CDRAWPARAMSIGLETON_H
