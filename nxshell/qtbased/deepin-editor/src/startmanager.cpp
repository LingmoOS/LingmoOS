// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "startmanager.h"
//#include <settings.h>

#include <DApplication>
#include <QDesktopWidget>
#include <DWidgetUtil>
#include <QDebug>
#include <QScreen>
#include <QPropertyAnimation>
#include <DSettingsOption>
#include <DAboutDialog>
//#include <DSettings>

DWIDGET_USE_NAMESPACE

// 备份定时器间隔
enum BackupInterval {
    EAutoBackupInterval = 5 * 60 * 1000,        ///< 周期自动备份定时 5分钟
    EDelayBackupInterval = 20,                  ///< 延迟备份间隔 20ms
};

// 用于配置文件书签的标识
static const QString s_bookMarkKey = "advance.editor.bookmark";

StartManager *StartManager::m_instance = nullptr;

StartManager *StartManager::instance()
{
    if (m_instance == nullptr) {
        m_instance = new StartManager;
    }

    return m_instance;
}

StartManager::StartManager(QObject *parent)
    : QObject(parent)
{
    //m_bIsDragEnter = false;
    //Create blank directory if it not exist.

    initBlockShutdown();

    m_blankFileDir = QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("blank-files");
    m_backupDir = QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("backup-files");
    m_autoBackupDir = QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("autoBackup-files");

    if (!QFileInfo(m_blankFileDir).exists()) {
        QDir().mkpath(m_blankFileDir);
    }

    if (!QFileInfo(m_backupDir).exists()) {
        QDir().mkpath(m_backupDir);
    }

    m_qlistTemFile = Settings::instance()->settings->option("advance.editor.browsing_history_temfile")->value().toStringList();
    // 初始化书签信息记录表
    initBookmark();

    //按时间自动备份（5分钟）
    m_pTimer = new QTimer;
    connect(m_pTimer, &QTimer::timeout, this, &StartManager::autoBackupFile);
    m_pTimer->start(EAutoBackupInterval);
}

bool StartManager::checkPath(const QString &file)
{
    for (int i = 0; i < m_windows.count(); i++) {
        EditWrapper *wrapper = m_windows.value(i)->wrapper(file);

        if (wrapper != nullptr) {
            FileTabInfo info = getFileTabInfo(wrapper->textEditor()->getFilePath());
            // Open exist tab if file has opened.
            popupExistTabs(info);
            return false;
        }
    }

    return true;
}

bool StartManager::ifKlu()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else {
        return false;
    }
}

bool StartManager::isMultiWindow()
{
    if (m_windows.count() > 1) {
        return true;
    }

    return false;
}

bool StartManager::isTemFilesEmpty()
{
    bool bIsEmpty = false;

    for (auto temFile : m_qlistTemFile) {
        if (temFile.isEmpty()) {
            bIsEmpty = true;
        }
    }

    return bIsEmpty;
}

void StartManager::autoBackupFile()
{
    // 标签页在拖拽状态时不执行备份
    if (m_bIsTagDragging) {
        return;
    }

    //如果自动备份文件夹不存在，创建自动备份文件夹
    if (!QFileInfo(m_autoBackupDir).exists()) {
        QDir().mkpath(m_autoBackupDir);
    } else {
        //有用户备份时删除用户备份
        if (!QDir(m_backupDir).isEmpty()) {
            QDir(m_backupDir).removeRecursively();
        }
    }

    QMap<QString, EditWrapper *> wrappers;
    QStringList listBackupInfo;
    QString filePath, localPath, curPos;
    QFileInfo fileInfo;
    m_qlistTemFile.clear();
    listBackupInfo = Settings::instance()->settings->option("advance.editor.browsing_history_temfile")->value().toStringList();

    //记录所有的文件信息
    for (int var = 0; var < m_windows.count(); ++var) {
        wrappers = m_windows.value(var)->getWrappers();
        QStringList list = wrappers.keys();

        for (EditWrapper *wrapper : wrappers) {
            //大文件加载时不备份
            if (wrapper->getFileLoading()) continue;

            filePath = wrapper->textEditor()->getFilePath();
            localPath = wrapper->textEditor()->getTruePath();

            StartManager::FileTabInfo tabInfo = StartManager::instance()->getFileTabInfo(filePath);
            curPos = QString::number(wrapper->textEditor()->textCursor().position());
            fileInfo.setFile(localPath);

            //json格式记录文件信息
            QJsonObject jsonObject;
            QJsonDocument document;
            jsonObject.insert("localPath", localPath); //记录源文件路径
            jsonObject.insert("cursorPosition", curPos); //记录鼠标位置
            jsonObject.insert("modify", wrapper->isModified()); //记录修改状态
            jsonObject.insert("window", var); //记录窗口Index

            //记录书签
            QList<int> bookmarkList = wrapper->textEditor()->getBookmarkInfo();

            if (!bookmarkList.isEmpty()) {
                QString bookmarkInfo;

                for (int i = 0; i < bookmarkList.count(); i++) {
                    if (i == bookmarkList.count() - 1) {
                        bookmarkInfo.append(QString::number(bookmarkList.value(i)));
                    } else {
                        bookmarkInfo.append(QString::number(bookmarkList.value(i)) + ",");
                    }
                }

                jsonObject.insert("bookMark", bookmarkInfo);

                // 更新记录全局书签信息
                m_bookmarkTable.insert(localPath, bookmarkList);
            } else {
                // 无书签，移除
                m_bookmarkTable.remove(localPath);
            }

            //记录活动页
            if (m_windows.value(var)->isActiveWindow()) {
                if (wrapper == m_windows.value(var)->currentWrapper()) {
                    jsonObject.insert("focus", true);
                }
            }

            //保存备份文件
            if (Utils::isDraftFile(filePath)) {
                wrapper->saveTemFile(filePath);
            } else {
                if (wrapper->isModified()) {
                    QString name = fileInfo.absolutePath().replace("/", "_");
                    QString qstrFilePath = m_autoBackupDir + "/" + Utils::getStringMD5Hash(fileInfo.baseName()) + "." + name + "." + fileInfo.suffix();
                    jsonObject.insert("temFilePath", qstrFilePath);
                    wrapper->saveTemFile(qstrFilePath);
                }
            }

            //使用json串形式保存
            document.setObject(jsonObject);
            QByteArray byteArray = document.toJson(QJsonDocument::Compact);
            list.replace(tabInfo.tabIndex, byteArray);
        }

        m_qlistTemFile.append(list);
    }

    //将json串列表写入配置文件
    Settings::instance()->settings->option("advance.editor.browsing_history_temfile")->setValue(m_qlistTemFile);
    // 备份书签信息
    saveBookmark();
}

int StartManager::recoverFile(Window *window)
{
    Window *pFocusWindow = nullptr;
    QString focusPath;
    bool bIsTemFile = false;
    QStringList blankFiles = QDir(m_blankFileDir).entryList(QStringList(), QDir::Files);
    int recFilesSum = 0;
    QStringList files = blankFiles;
    QFileInfo fileInfo;
    QString lastmodifiedtime;
    //去除非新建文件
    for (auto file : blankFiles) {
        if (!file.contains("blank_file")) {
            files.removeOne(file);
        }
    }

    int windowIndex = -1;

    //根据备份信息恢复文件
    for (int i = 0; i < m_qlistTemFile.count(); i++) {
        QJsonParseError jsonError;
        // 转化为 JSON 文档
        QJsonDocument doucment = QJsonDocument::fromJson(m_qlistTemFile.value(i).toUtf8(), &jsonError);
        // 解析未发生错误
        if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            if (doucment.isObject()) {
                QString temFilePath;
                QString localPath;
                // JSON 文档为对象
                QJsonObject object = doucment.object();  // 转化为对象

                //得到恢复文件对应的窗口
                if (object.contains("window")) {  // 包含指定的 key
                    QJsonValue value = object.value("window");  // 获取指定 key 对应的 value

                    if (value.isDouble()) {
                        if (windowIndex == -1) {
                            windowIndex = static_cast<int>(value.toDouble());
                        } else {
                            if (windowIndex != static_cast<int>(value.toDouble())) {
                                windowIndex = static_cast<int>(value.toDouble());
                                window = createWindow(true);
                                window->showCenterWindow(false);
                            }
                        }
                    }
                }

                //得到备份文件路径
                if (object.contains("temFilePath")) {  // 包含指定的 key
                    QJsonValue value = object.value("temFilePath");  // 获取指定 key 对应的 value

                    if (value.isString()) {  // 判断 value 是否为字符串
                        temFilePath = value.toString();  // 将 value 转化为字符串
                    }
                }

                //得到文件修改状态
                if (object.contains("modify")) {  // 包含指定的 key
                    QJsonValue value = object.value("modify");  // 获取指定 key 对应的 value

                    if (value.isBool()) {  // 判断 value 是否为字符串
                        bIsTemFile = value.toBool();
                    }
                }
                if (object.contains("lastModifiedTime")) {
                    auto v = object.value("lastModifiedTime");
                    if (v.isString()) {
                        lastmodifiedtime = v.toString();
                    }
                }

                //得到真实文件路径
                if (object.contains("localPath")) {  // 包含指定的 key
                    QJsonValue value = object.value("localPath");  // 获取指定 key 对应的 value

                    if (value.isString()) {  // 判断 value 是否为字符串
                        localPath = value.toString();  // 将 value 转化为字符串
                        fileInfo.setFile(localPath);
                        if (!fileInfo.exists()) {
                            continue;
                        }
                    }
                }

                //打开文件
                if (!temFilePath.isEmpty()) {
                    if (Utils::fileExists(temFilePath)) {
                        window->addTemFileTab(temFilePath, fileInfo.fileName(), localPath, lastmodifiedtime, bIsTemFile);

                        //打开文件后设置书签
                        if (object.contains("bookMark")) {  // 包含指定的 key
                            QJsonValue value = object.value("bookMark");  // 获取指定 key 对应的 value

                            if (value.isString()) {
                                QList<int> bookmarkList;
                                bookmarkList = analyzeBookmakeInfo(value.toString());
                                window->wrapper(temFilePath)->textEditor()->setBookMarkList(bookmarkList);
                            }
                        } else if (m_bookmarkTable.contains(temFilePath)) {
                            // 若文件已有配置，则以文件为准，否则以全局配置为准
                            window->wrapper(localPath)->textEditor()->setBookMarkList(m_bookmarkTable.value(temFilePath));
                        }

                        if (object.contains("focus")) {  // 包含指定的 key
                            QJsonValue value = object.value("focus");  // 获取指定 key 对应的 value

                            if (value.isBool() && value.toBool()) {
                                pFocusWindow = window;
                                focusPath = temFilePath;
                            }
                        }

                        recFilesSum++;
                    }
                } else {
                    if (!localPath.isEmpty() && Utils::fileExists(localPath)) {
                        // 若为草稿文件或不支持的MIMETYPE文件，显示默认名称标签
                        if (Utils::isDraftFile(localPath) || !Utils::isMimeTypeSupport(localPath)) {
                            //得到新建文件名称
                            int index = files.indexOf(QFileInfo(localPath).fileName());

                            if (index >= 0) {
                                QString fileName = tr("Untitled %1").arg(index + 1);
                                window->addTemFileTab(localPath, fileName, localPath, lastmodifiedtime, bIsTemFile);

                            }
                        } else {
                            window->addTemFileTab(localPath, fileInfo.fileName(), localPath, lastmodifiedtime, bIsTemFile);
                        }

                        //打开文件后设置书签
                        if (object.contains("bookMark")) {  // 包含指定的 key
                            QJsonValue value = object.value("bookMark");  // 获取指定 key 对应的 value

                            if (value.isString()) {
                                QList<int> bookmarkList;
                                bookmarkList = analyzeBookmakeInfo(value.toString());
                                window->wrapper(localPath)->textEditor()->setBookMarkList(bookmarkList);
                            }
                        } else if (m_bookmarkTable.contains(localPath)) {
                            // 若文件已有配置，则以文件为准，否则以全局配置为准
                            window->wrapper(localPath)->textEditor()->setBookMarkList(m_bookmarkTable.value(localPath));
                        }

                        if (object.contains("focus")) {  // 包含指定的 key
                            QJsonValue value = object.value("focus");  // 获取指定 key 对应的 value

                            if (value.isBool() && value.toBool()) {
                                pFocusWindow = window;
                                focusPath = localPath;
                            }
                        }

                        recFilesSum++;
                    }
                }
            }
        }
    }

    //当前活动页
    if (pFocusWindow != nullptr && !focusPath.isNull()) {
        FileTabInfo info;
        info.windowIndex = m_windows.indexOf(pFocusWindow);
        info.tabIndex = pFocusWindow->getTabIndex(focusPath);
        popupExistTabs(info);
    }

    return recFilesSum;
}

void StartManager::openFilesInWindow(QStringList files)
{
    // Open window with blank tab if no files need open.
    if (files.isEmpty()) {
        if (m_windows.count() >= 20) return;
        Window *window = createWindow();

        if (m_windows.count() > 0) {
            window->showCenterWindow(false);
        } else {
            window->showCenterWindow(true);
        }

        window->addBlankTab();
        window->activateWindow();
    } else {
        for (const QString &file : files) {
            FileTabInfo info = getFileTabInfo(file);

            // Open exist tab if file has opened.
            if (info.windowIndex != -1) {
                popupExistTabs(info);
            }
            // Add new tab in current window.
            else {
                Window *window = createWindow();
                window->showCenterWindow(true);
                window->addTab(file);
            }
        }
    }
}

void StartManager::openFilesInTab(QStringList files)
{
    if (files.isEmpty()) {
        QDir blankDirectory = QDir(QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("blank-files"));
        QStringList blankFiles = blankDirectory.entryList(QStringList(), QDir::Files);

        if (m_windows.isEmpty()) {
            Window *window = createWindow(true);
            window->showCenterWindow(true);

            if (!isTemFilesEmpty()) {
                if (recoverFile(window) == 0) {
                    window->addBlankTab();
                }
            } else {
                if (blankFiles.isEmpty()) {
                    window->addBlankTab();
                } else {
                    for (auto file : blankFiles) {
                        if (file.contains("blank_file")) {
                            blankDirectory.remove(file);
                        }
                    }

                    window->addBlankTab();
                }
            }
        }
        // Just active first window if no file is need opened.
        else {
            Window *window = createWindow();
            window->show();
            window->addBlankTab();
        }
    } else {
        for (const QString &file : files) {

            if (!checkPath(file)) {
                // 存在已打开文件时，进行下一文件判断
                continue;
            }

            FileTabInfo info = getFileTabInfo(file);

            // Open exist tab if file has opened.
            if (info.windowIndex != -1) {
                popupExistTabs(info);
            }
            // Create new window with file if haven't window exist.
            else if (m_windows.size() == 0) {
                Window *window = createWindow(true);
                window->showCenterWindow(true);
                QTimer::singleShot(50, [ = ] {
                    recoverFile(window);
                    window->addTab(file);
                });
            }
            // Open file tab in first window of window list.
            else {
                Window *window = m_windows[0];
                window->addTab(file);
                //window->setWindowState(Qt::WindowActive);
                //通过dbus接口从任务栏激活窗口
                if (!Q_LIKELY(Utils::activeWindowFromDock(window->winId()))) {
                    window->activateWindow();
                }
            }
        }
    }
}

void StartManager::createWindowFromWrapper(const QString &tabName, const QString &filePath, const QString &qstrTruePath, EditWrapper *buffer, bool isModifyed)
{
    Window *pWindow = createWindow();
    //window->showCenterWindow();
    QRect rect = pWindow->rect();
    QPoint pos = QCursor::pos() ;/*- window->topLevelWidget()->pos();*/
    QRect desktopRect = QApplication::desktop()->rect();
    QPoint startPos = pos;

    if ((pos.x() + rect.width()) > desktopRect.width()) {
        startPos.setX(desktopRect.width() - rect.width());
    } else if (pos.x() < 0) {
        startPos.setX(0);
    }

    if ((pos.y() + rect.height()) > desktopRect.height()) {
        startPos.setY(desktopRect.height() - rect.height());
    } else if (pos.y() < 0) {
        startPos.setY(0);
    }

    QRect startRect(startPos, Tabbar::sm_pDragPixmap->rect().size());
    //QRect startRect(startPos, QSize(0,0));
    QRect endRect(startPos, pWindow->rect().size());
    pWindow->move(startPos);
#if 0
    // window->setFixedSize(Tabbar::sm_pDragPixmap->rect().size());
    QLabel *pLab = new QLabel();
    //pLab->resize(Tabbar::sm_pDragPixmap->rect().size());
    pLab->move(pos);
    pLab->setPixmap(*Tabbar::sm_pDragPixmap);
    pLab->show();
#endif
    //添加编辑窗口drop动态显示效果　梁卫东　２０２０－０８－２５　０９：５４：５７
    QPropertyAnimation *geometry = new QPropertyAnimation(pWindow, "geometry");
    geometry->setDuration(200);
    geometry->setStartValue(startRect);
    geometry->setEndValue(endRect);
    geometry->setEasingCurve(QEasingCurve::InCubic);

    //OutCubic InCubic
#if 0
    QPropertyAnimation *Opacity = new QPropertyAnimation(this, "windowOpacity");
    connect(Opacity, &QPropertyAnimation::finished, Opacity, &QPropertyAnimation::deleteLater);
    Opacity->setDuration(200);
    Opacity->setStartValue(1.0);
    Opacity->setEndValue(0);
    Opacity->setEasingCurve(QEasingCurve::InCirc);
#endif

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    connect(group, &QParallelAnimationGroup::finished, this, [/*window,geometry,Opacity,group,*/ = ]() {
        pWindow->show();
        pWindow->showCenterWindow(false);
        geometry->deleteLater();
        group->deleteLater();

        pWindow->addTabWithWrapper(buffer, filePath, qstrTruePath, tabName);
        pWindow->currentWrapper()->updateModifyStatus(isModifyed);
        pWindow->currentWrapper()->OnUpdateHighlighter();
        pWindow->setFocus();
        pWindow->setFontSizeWithConfig(pWindow->currentWrapper());
    });

    group->addAnimation(geometry);
    // group->addAnimation(Opacity);
    group->start();
}

void StartManager::loadTheme(const QString &themeName)
{
    for (Window *window : m_windows) {
        window->loadTheme(themeName);
    }
}

Window *StartManager::createWindow(bool alwaysCenter)
{
    // Create window.
    Window *window = new Window;
    connect(window, &Window::themeChanged, this, &StartManager::loadTheme, Qt::QueuedConnection);
    connect(window, &Window::sigJudgeBlockShutdown, this, &StartManager::slotCheckUnsaveTab, Qt::QueuedConnection);
    connect(window, &Window::tabChanged, this, &StartManager::slotDelayBackupFile, Qt::QueuedConnection);

    // Quit application if close last window.
    connect(window, &Window::closeWindow, this, &StartManager::slotCloseWindow);
//    connect(window, &Window::closeWindow, this, [ = ] {
//        int windowIndex = m_windows.indexOf(window);
//        //qDebug() << "Close window " << windowIndex;

//        Window *pWindow = static_cast<Window *>(sender());
//        if (windowIndex >= 0)
//        {
//            m_windows.takeAt(windowIndex);
//        }

//        if (m_windows.isEmpty())
//        {
//            QDir path = QDir::currentPath();
//            if (!path.exists()) {
//                return ;
//            }
//            path.setFilter(QDir::Files);
//            QStringList nameList = path.entryList();
//            foreach (auto name, nameList) {
//                if (name.contains("tabPaths.txt")) {
//                    QFile file(name);
//                    file.remove();
//                }
//            }
//            QApplication::quit();
//            PerformanceMonitor::closeAPPFinish();
//        }
//    });

    // Init window position.
    initWindowPosition(window, alwaysCenter);

    connect(window, &Window::newWindow, this, &StartManager::slotCreatNewwindow);

    // 标签页拖拽状态变更时触发，防止在拖拽过程中备份导致获取的标签状态异常
    connect(window->getTabbar(), &DTabBar::dragStarted, this, [this](){
        m_bIsTagDragging = true;
    });
    connect(window->getTabbar(), &DTabBar::dragEnd, this, [this](){
        m_bIsTagDragging = false;
        slotDelayBackupFile();
    });

    // Append window in window list.
    m_windows << window;

    return window;
}


void StartManager::slotCloseWindow()
{
    Window *pWindow = static_cast<Window *>(sender());
    int windowIndex = m_windows.indexOf(pWindow);

    if (windowIndex >= 0) {
        m_windows.takeAt(windowIndex);
    }

    if (m_windows.isEmpty()) {
        // 保存书签信息
        saveBookmark();

        QDir path = QDir::currentPath();
        if (!path.exists()) {
            return ;
        }
        path.setFilter(QDir::Files);
        QStringList nameList = path.entryList();
        foreach (auto name, nameList) {
            if (name.contains("tabPaths.txt")) {
                QFile file(name);
                file.remove();
            }
        }

        // 在程序退出前（最后一个窗口关闭后），手动注销DBus服务，防止重启文本编辑器时判断应用仍在占用总线
        // 导致被附加到上一文本编辑器进程
        QDBusConnection::sessionBus().unregisterService("com.deepin.Editor");

        QTimer::singleShot(1000, []() {
            QApplication::quit();
        });

        PerformanceMonitor::closeAPPFinish();
    }
}

void StartManager::slotDelayBackupFile()
{
    // 判断定时器是否已在触发状态，防止短时间内的多次触发，标签页拖拽状态不触发
    if (!m_DelayTimer.isActive() && !m_bIsTagDragging) {
        m_DelayTimer.start(EDelayBackupInterval, this);
    }
}

void StartManager::timerEvent(QTimerEvent *e)
{
    // 判断是否为延迟备份
    if (e->timerId() == m_DelayTimer.timerId()) {
        m_DelayTimer.stop();
        // 执行配置文件备份
        autoBackupFile();

        // 重启周期定时备份
        m_pTimer->start(EAutoBackupInterval);
    }
}

void StartManager::slotCreatNewwindow()
{
    openFilesInWindow(QStringList());
}

void StartManager::initWindowPosition(Window *window, bool alwaysCenter)
{
    if (m_windows.isEmpty() || alwaysCenter) {
        //Dtk::Widget::moveToCenter(window);
    } else {
        // Add window offset to avoid all editor window popup at same coordinate.
        int windowOffset = m_windows.size() * 50;
        QRect screenGeometry = qApp->desktop()->screenGeometry(QCursor::pos());
        window->move(screenGeometry.x() + windowOffset, screenGeometry.y() + windowOffset);
    }
}

void StartManager::popupExistTabs(FileTabInfo info)
{
    Window *window = m_windows[info.windowIndex];
    //window->showNormal();
    window->activeTab(info.tabIndex);
    //window->setWindowState(Qt::WindowActive);
    //通过dbus接口从任务栏激活窗口
    if (!Q_LIKELY(Utils::activeWindowFromDock(window->winId()))) {
        window->activateWindow();
    }

#if 0
    int indexid = 0;
    uint winid = 0;
    QDBusInterface dock("com.deepin.dde.daemon.Dock",
                        "/com/deepin/dde/daemon/Dock",
                        "com.deepin.dde.daemon.Dock",
                        QDBusConnection::sessionBus()
                       );
    QDBusReply<QStringList> rep = dock.call("GetEntryIDs");

    for (auto name : rep.value()) {
        if (name == "deepin-editor") {
            indexid = rep.value().indexOf(name);
        }
    }

    m_pDock.reset(new Dock("com.deepin.dde.daemon.Dock",
                           "/com/deepin/dde/daemon/Dock",
                           QDBusConnection::sessionBus(),
                           this
                          )
                 );
    QList<QDBusObjectPath> list = m_pDock->entries();

    m_pEntry.reset(new Entry("com.deepin.dde.daemon.Dock",
                             list[indexid].path(),
                             QDBusConnection::sessionBus(),
                             this));
    winid = m_pEntry->currentWindow() ;


    QDBusMessage active = QDBusMessage::createMethodCall("com.deepin.dde.daemon.Dock",
                                                         "/com/deepin/dde/daemon/Dock",
                                                         "com.deepin.dde.daemon.Dock",
                                                         "ActivateWindow");
    active << winid;
    QDBusConnection::sessionBus().call(active, QDBus::BlockWithGui);
#endif
}

StartManager::FileTabInfo StartManager::getFileTabInfo(QString file)
{
    FileTabInfo info = {-1, -1};

    //qDebug() << "Windows size: " << m_windows.size();

    for (Window *window : m_windows) {
        int tabIndex = window->getTabIndex(file);
        if (tabIndex >= 0) {
            info.windowIndex = m_windows.indexOf(window);
            info.tabIndex = tabIndex;
            break;
        }
    }

    return info;
}

QList<int> StartManager::analyzeBookmakeInfo(QString bookmarkInfo)
{
    QList<int> bookmarkList;
    int nLeftPosition = 0;
    int nRightPosition = bookmarkInfo.indexOf(",");
    bookmarkList << bookmarkInfo.mid(nLeftPosition, nRightPosition - nLeftPosition).toInt();
    nLeftPosition = nRightPosition;

    while (nRightPosition != -1) {
        nRightPosition = bookmarkInfo.indexOf(",", nRightPosition + 1);
        bookmarkList << bookmarkInfo.mid(nLeftPosition + 1, nRightPosition - nLeftPosition - 1).toInt();
        nLeftPosition = nRightPosition;
    }

    return bookmarkList;
}

/**
 * @brief 主动更新记录书签信息
 * @param localPath 文件路径
 * @param bookmark  书签信息
 */
void StartManager::recordBookmark(const QString &localPath, const QList<int> &bookmark)
{
    m_bookmarkTable.insert(localPath, bookmark);
}

/**
 * @return 返回文件 \a localPath 的书签记录
 */
QList<int> StartManager::findBookmark(const QString &localPath)
{
    return m_bookmarkTable.value(localPath);
}

void StartManager::initBlockShutdown()
{
    if (m_reply.value().isValid()) {
        //qDebug() << "m_reply.value().isValid():" << m_reply.value().isValid();
        return;
    }

    m_pLoginManager = new QDBusInterface("org.freedesktop.login1",
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         QDBusConnection::systemBus());

    m_arg << QString("shutdown")             // what
          << qApp->applicationDisplayName()           // who
          << QObject::tr("File not saved") // why
          << QString("delay");                        // mode

    m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);
    if (m_reply.isValid()) {
        m_reply.value().fileDescriptor();
    }
}

/**
 * @brief 从配置文件中获取全局的书签信息，包括已关闭的所有文件书签，
 *      会遍历每个书签记录并判断文件是否存在，若文件被删除或移动，则不再记录对应的书签信息。
 */
void StartManager::initBookmark()
{
    // 遍历书签信息列表
    QStringList bookmarkInfoList = Settings::instance()->settings->value(s_bookMarkKey).toStringList();
    for (const QString &bookmarkInfo : bookmarkInfoList) {
        QJsonParseError readError;
        QJsonDocument doc = QJsonDocument::fromJson(bookmarkInfo.toUtf8(), &readError);
        if (QJsonParseError::NoError == readError.error
                && !doc.isNull()) {
            QJsonObject obj = doc.object();
            QString filePath = obj.value("localPath").toString();

            // 判断文件是否仍存在，若不存在，则不保留书签信息
            if (!filePath.isEmpty()
                    && QFileInfo::exists(filePath)) {
                QString bookmarkStr = obj.value("bookmark").toString();
                if (!bookmarkStr.isEmpty()) {
                    QList<int> bookmarkList = analyzeBookmakeInfo(bookmarkStr);
                    if (!bookmarkList.isEmpty()) {
                        // 文件存在且书签标记非空，缓存书签信息
                        m_bookmarkTable.insert(filePath, bookmarkList);
                    }
                }
            }
        }
    }
}

/**
 * @brief 将当前记录的所有文件的书签信息转换为Json数据列表记录到配置信息中，
 *      被删除或移动文件的书签将被销毁。
 */
void StartManager::saveBookmark()
{
    QStringList recordInfo;
    // 遍历记录
    for (auto itr = m_bookmarkTable.begin(); itr != m_bookmarkTable.end();) {
        if (!QFileInfo::exists(itr.key())
                || itr.value().isEmpty()) {
            // 文件不存在则销毁记录
            itr = m_bookmarkTable.erase(itr);
        } else {
            QJsonObject obj;
            obj.insert("localPath", itr.key());

            // 遍历书签信息并组合
            QString bookmarkInfo;
            const auto &markList = itr.value();
            for (int i = 0; i < markList.count(); i++) {
                if (i == itr.value().count() - 1) {
                    bookmarkInfo.append(QString::number(markList.value(i)));
                } else {
                    bookmarkInfo.append(QString::number(markList.value(i)) + ",");
                }
            }
            obj.insert("bookmark", bookmarkInfo);

            QJsonDocument doc(obj);
            recordInfo.append(doc.toJson(QJsonDocument::Compact));

            ++itr;
        }
    }

    // 将书签信息保存至配置文件
    Settings::instance()->settings->option(s_bookMarkKey)->setValue(recordInfo);
}

void StartManager::slotCheckUnsaveTab()
{
    for (Window *pWindow : m_windows) {
        //如果返回true，则表示有未保存的tab项，则阻塞系统关机
        bool bRet = pWindow->checkBlockShutdown();
        if (bRet == true) {
            m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);
            if (m_reply.isValid()) {
            }

            return;
        }
    }

    //如果for结束则表示没有发现未保存的tab项，则放开阻塞关机
    if (m_reply.isValid()) {
        m_reply = QDBusReply<QDBusUnixFileDescriptor>();
    }
}

void StartManager::closeAboutForWindow(Window *window)
{
    if (qApp != nullptr) {
        DAboutDialog *pAboutDialog = qApp->aboutDialog();
        if (pAboutDialog != nullptr) {
            if (pAboutDialog->parent() != nullptr) {
                if (pAboutDialog->parent() == window) {
                    pAboutDialog->close();
                }
            }
        }
    }
}
