// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STARTMANAGER_H
#define STARTMANAGER_H

#include "widgets/window.h"
#include "editor/editwrapper.h"

#include <com_deepin_dde_daemon_dock.h>
#include <com_deepin_dde_daemon_dock_entry.h>
#include <QObject>

using Dock          = com::deepin::dde::daemon::Dock;
using Entry         = com::deepin::dde::daemon::dock::Entry;


class StartManager : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.Editor")

public:
    struct FileTabInfo {
        int windowIndex;
        int tabIndex;
    };

    static StartManager *instance();
    explicit StartManager(QObject *parent = nullptr);
    bool checkPath(const QString &file);
    bool ifKlu();

    /**
     * @brief isMultiWindow 是否是多窗口
     * @return true or false
     */
    bool isMultiWindow();

    /**
     * @brief isTemFilesEmpty 是否需要备份
     * @return　true or false
     */
    bool isTemFilesEmpty();

    /**
     * @brief autoBackupFile 自动备份
     */
    void autoBackupFile();

    /**
     * @brief recoverFile 备份恢复
     * @param window　从哪个窗口进行恢复
     * @return 恢复完成的文件数量
     */
    int recoverFile(Window *window);

    /**
     * @brief analyzeBookmakeInfo 解析书签信息
     * @param bookmarkInfo 书签信息
     * @return 书签列表
     */
    QList<int> analyzeBookmakeInfo(QString bookmarkInfo);

    // 主动更新记录书签信息
    void recordBookmark(const QString &localPath, const QList<int> &bookmark);
    // 查找文件对应的书签记录
    QList<int> findBookmark(const QString &localPath);

public slots:
    Q_SCRIPTABLE void openFilesInTab(QStringList files);
    Q_SCRIPTABLE void openFilesInWindow(QStringList files);

    void createWindowFromWrapper(const QString &tabName, const QString &filePath, const QString &qstrTruePath, EditWrapper *buffer, bool isModifyed);
    void loadTheme(const QString &themeName);

    Window *createWindow(bool alwaysCenter = false);
    void initWindowPosition(Window *window, bool alwaysCenter = false);
    void popupExistTabs(FileTabInfo info);
    FileTabInfo getFileTabInfo(QString file);

    void slotCheckUnsaveTab();

    void closeAboutForWindow(Window *window);

    //信号封装 ut002764 2021.6.28
    void slotCreatNewwindow();
    void slotCloseWindow();

    // 延迟备份定时器，超时后备份配置文件
    void slotDelayBackupFile();

protected:
    // 接收延迟更新定时任务，执行备份配置文件
    virtual void timerEvent(QTimerEvent *e) override;

private:
    void initBlockShutdown();
    // 初始化书签信息
    void initBookmark();
    // 保存书签信息
    void saveBookmark();

private:
    static StartManager *m_instance;
    QList<Window *> m_windows;

    QDBusReply<QDBusUnixFileDescriptor> m_reply;
    QDBusInterface *m_pLoginManager = nullptr;
    QList<QVariant> m_arg;

    QDBusPendingReply<QDBusUnixFileDescriptor> m_inhibitReply;
    QScopedPointer<Dock> m_pDock;
    QScopedPointer<Entry> m_pEntry;
    QStringList m_qlistTemFile;                 ///< 备份信息列表
    QHash<QString, QList<int>> m_bookmarkTable; ///< 书签标记信息表
    QTimer *m_pTimer;
    QBasicTimer m_DelayTimer;   ///< 延迟备份定时器
    QString m_blankFileDir;     ///< 新建文件目录
    QString m_backupDir;        ///< 用户备份文件目录
    QString m_autoBackupDir;    ///< 自动备份文件目录
    Window *pFocusWindow;

    bool    m_bIsTagDragging = false;   ///< 当前Tab页处于拖拽状态时，部分处理被延后
};

#endif
