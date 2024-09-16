// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debinstaller.h"
#include "model/deblistmodel.h"
#include "model/packageanalyzer.h"
#include "view/widgets/filechoosewidget.h"
#include "view/pages/multipleinstallpage.h"
#include "view/pages/singleinstallpage.h"
#include "view/pages/uninstallconfirmpage.h"
#include "view/pages/AptConfigMessage.h"
#include "view/pages/packageselectview.h"
#include "view/pages/ddimerrorpage.h"
#include "singleInstallerApplication.h"
#include "model/packageselectmodel.h"
#include "settingdialog.h"
#include "utils/utils.h"

#include <DInputDialog>
#include <DRecentManager>
#include <DMessageManager>
#include <DTitlebar>
#include <DGuiApplicationHelper>

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMimeData>
#include <QStatusBar>
#include <QLayout>
#include <QProcess>
#include <QScreen>
#include <QStyleFactory>
#include <QApplication>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>

using QApt::DebFile;

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

// Avoid magic number
enum PackageCountType {
    NoPackage = 0,
    OnePackage = 1,
    TwoPackages = 2,
};

DebInstaller::DebInstaller(QWidget *parent)
    : DMainWindow(parent)
    , m_fileListModel(new DebListModel(this))
    , m_fileChooseWidget(new FileChooseWidget(this))
    , m_centralLayout(new QStackedLayout())
    , m_settingDialog(new SettingDialog(this))
    , m_ddimModel(new PackageSelectModel(this))
{
    initUI();
    initConnections();

    QtConcurrent::run([]() { PackageAnalyzer::instance().initBackend(); });
}

DebInstaller::~DebInstaller() { }

void DebInstaller::initUI()
{
    // Hide the shadow under the title bar
    setTitlebarShadowEnabled(false);
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);

    this->setObjectName("DebInstaller");
    this->setAccessibleName("DebInstaller");

    // file choose widget settings
    m_fileChooseWidget->setObjectName("FileChooseWidget");
    m_fileChooseWidget->setAccessibleName("FileChooseWidget");

    // 初始化 加载文件选择widget
    m_centralLayout->addWidget(m_fileChooseWidget);
    m_lastPage = m_fileChooseWidget;
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);

    // 设置当前主窗口
    QWidget *wrapWidget = new QWidget(this);
    wrapWidget->setLayout(m_centralLayout);

// #define SHOWBORDER
#ifdef SHOWBORDER
    wrapWidget->setStyleSheet("QWidget{border:1px solid black;}");
#endif

    initTitleBar();
    setCentralWidget(wrapWidget);  // 将给定的小部件设置为主窗口的中心小部件。
    setAcceptDrops(true);          // 启用了drop事件
    setFixedSize(480, 380);
    setWindowTitle(tr("Package Installer"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));  // 仅仅适用于windows系统
    move(qApp->primaryScreen()->geometry().center() - geometry().center());
}

void DebInstaller::initTitleBar()
{
    // title bar settings
    QAction *settingAction(new QAction(tr("Settings"), this));
    DMenu *menu = new DMenu;
    menu->addAction(settingAction);
    DTitlebar *tb = titlebar();
    if (tb != nullptr) {
        tb->setMenu(menu);
        tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
        tb->setTitle("");
        tb->setAutoFillBackground(true);
        tb->setDisableFlags(Qt::CustomizeWindowHint);
    }
    connect(settingAction, &QAction::triggered, this, &DebInstaller::slotSettingDialogVisiable);
}

void DebInstaller::initConnections()
{
    connect(m_fileListModel, &DebListModel::signalAppendFailMessage, this, &DebInstaller::slotReceiveAppendFailed);
    connect(m_fileListModel, &DebListModel::signalPackageCountChanged, this, [this](int count) {
        switch (count) {
            case NoPackage:
                slotReset();
                break;
            case OnePackage:
                refreshSingle();
                break;
            default:
                // If current not multi page, change.
                if (MultiPage != m_Filterflag) {
                    single2Multi();
                } else {
                    refreshMulti();
                }
                break;
        }
    });

    // 正在添加的信号
    connect(m_fileListModel, &DebListModel::signalAppendStart, this, &DebInstaller::appendPackageStart);

    // 此次添加完成的信号
    connect(m_fileListModel, &DebListModel::signalAppendFinished, this, &DebInstaller::appendFinished);

    // Append packages via file-choose-widget's file-choose-button
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::slotPackagesSelected);

    // Select the focus of the page

    // TODO: remove to instance later.
    // Determine the status of the current application based on the status of the authorization box.
    connect(m_fileListModel, &DebListModel::signalLockForAuth, this, &DebInstaller::slotSetAuthingStatus);
    connect(m_fileListModel, &DebListModel::signalAuthCancel, this, &DebInstaller::slotShowHiddenButton);
    connect(m_fileListModel, &DebListModel::signalEnableReCancelBtn, this, &DebInstaller::slotSetEnableButton);
    connect(m_fileListModel, &DebListModel::signalDependResult, this, &DebInstaller::slotDealDependResult);
    connect(m_fileListModel, &DebListModel::signalEnableCloseButton, this, &DebInstaller::slotEnableCloseButton);

    // During installing/uninstalling, drag is not allowed
    connect(m_fileListModel, &DebListModel::signalStartInstall, this, &DebInstaller::disableCloseAndExit);
    connect(m_fileListModel, &DebListModel::signalWorkerFinished, this, &DebInstaller::slotChangeDragFlag);
    connect(m_fileListModel, &DebListModel::signalPackageCannotFind, this, &DebInstaller::slotShowPkgRemovedMessage);

    // 选择安装页面
    connect(m_ddimModel, &PackageSelectModel::selectInfosChanged, this, &DebInstaller::slotShowSelectPage);
    connect(m_ddimModel, &PackageSelectModel::selectInfosDoNotHaveChange, this, &DebInstaller::slotShowPkgExistMessage);

    // 阻塞界面
    connect(&PackageAnalyzer::instance(), &PackageAnalyzer::runBackend, this, [this](bool inProcess) {
        if (inProcess) {
            slotShowPkgProcessBlockPage(BackendProcessPage::APT_INIT, 0, 0);
        } else {
            slotShowPkgProcessBlockPage(BackendProcessPage::PROCESS_FIN, 0, 0);
        }
    });

    connect(&PackageAnalyzer::instance(),
            &PackageAnalyzer::runAnalyzeDeb,
            this,
            [this](bool inProcess, int currentRote, int pkgCount) {
                if (inProcess) {
                    slotShowPkgProcessBlockPage(BackendProcessPage::READ_PKG, currentRote, pkgCount);
                } else {
                    slotShowPkgProcessBlockPage(BackendProcessPage::PROCESS_FIN, 0, 0);
                }
            });

    connect(this, &DebInstaller::runOldProcess, this, &DebInstaller::slotPackagesSelected, Qt::ConnectionType::QueuedConnection);
}

void DebInstaller::slotEnableCloseButton(bool enable)
{
    if (enable) {
        enableCloseAndExit();  // 启用关闭按钮
    } else {
        disableCloseAndExit();  // 禁用关闭按钮
    }
}

void DebInstaller::slotSettingDialogVisiable()
{
    m_settingDialog->setModal(true);
    m_settingDialog->show();
}

#if 0
void DebInstaller::PackagesSelected(const QStringList &debPathList)
{
    // 如果此时 软件包安装器不是处于准备状态且还未初始化完成或此时正处于正在安装或者卸载状态，则不添加
    // 依赖配置过程中，不添加其他包
    if ((!m_lastPage.isNull() && m_fileListModel->getWorkerStatus() != DebListModel::WorkerPrepare) ||
        DebListModel::WorkerProcessing == m_fileListModel->getWorkerStatus() ||
        DebListModel::WorkerUnInstall == m_fileListModel->getWorkerStatus() || DebListModel::AuthPop == m_wineAuthStatus ||
        DebListModel::AuthConfirm == m_wineAuthStatus || DebListModel::AuthDependsErr == m_wineAuthStatus) {
    } else {
        // 开始添加包，将要添加的包传递到后端，添加包由后端处理
        m_fileListModel->slotAppendPackage(debPathList);
    }
}

QString DebInstaller::startInstallPackge(const QString &debPath)
{
    QString message;
    // 判断包有效性
    message = m_fileListModel->checkPackageValid(debPath);
    if (!message.isEmpty())
        return message;
    // 添加包
    PackagesSelected(QStringList(debPath));
    // 安装包
    m_fileListModel->slotInstallPackages();
    // 获取安装结果
    message = m_fileListModel->getInstallErrorMessage();
    if (message.isEmpty())
        message = "install succeeded";
    return message;
}

QString DebInstaller::startUnInstallPackge(const QString &debPath)
{
    // 查询的当前包是否安装,避免重复卸载
    if (DebListModel::PackageInstallStatus::NotInstalled == checkInstallStatus(debPath))
        return "currentdeb not install, uninstall package faild";
    // 添加包
    PackagesSelected(QStringList(debPath));
    // 开始卸载
    m_fileListModel->slotUninstallPackage(0);
    // 获取卸载结果
    QString message = m_fileListModel->getInstallErrorMessage();
    if (message.isEmpty())
        message = "Uninstall succeeded";
    return message;
}

int DebInstaller::checkInstallStatus(const QString &debPath)
{
    if (debPath.isEmpty())
        return -1;
    return m_fileListModel->checkInstallStatus(debPath);
}

int DebInstaller::checkDependsStatus(const QString &debPath)
{
    if (debPath.isEmpty())
        return -1;
    return m_fileListModel->checkDependsStatus(debPath);
}

int DebInstaller::checkDigitalSignature(const QString &debPath)
{
    if (debPath.isEmpty())
        return -1;
    return m_fileListModel->checkDigitalSignature(debPath);
}

QString DebInstaller::getPackageInfo(const QString &debPath)
{
    if (debPath.isEmpty())
        return "";
    return m_fileListModel->getPackageInfo(debPath).join(";");
}
#endif

void DebInstaller::slotShowSelectInstallPage(const QList<int> &selectIndexes)
{
    QtConcurrent::run([this, selectIndexes]() {
        // 1.交给model分析需要装哪些包
        auto needInstallIrs = m_ddimModel->analyzePackageInstallNeeded(selectIndexes);

        // 2.汇集安装路径
        QStringList paths;
        for (auto &ir : needInstallIrs) {
            paths.push_back(ir.filePath);
        }

        // 3.按老流程进行安装
        emit runOldProcess(paths);
    });
}

void DebInstaller::slotShowSelectPage(const QList<DebIr> &selectedInfos)
{
    if (selectedInfos.isEmpty() && m_ddimView == nullptr) {
        // 不应该能够跳转至此
        qWarning() << "ddim error process";
        return;
    }

    if (selectedInfos.isEmpty() &&
        m_centralLayout->currentWidget() == m_ddimView) {  // 此时已经判定m_ddimView界面是存在的，属于界面刷新操作
        slotShowSelectInstallPage({});
        return;
    }

    bool haveMustInstallDeb = !m_ddimModel->mustInstallData().isEmpty();

    if (m_ddimView != nullptr) {
        if (m_centralLayout->currentWidget() == m_ddimView) {  // 正处于此页面时直接刷新
            m_ddimView->flushDebList(selectedInfos);
            m_ddimView->setHaveMustInstallDeb(haveMustInstallDeb);
        } else {
            return;
        }
    } else {  // 初次进入
        titlebar()->setTitle(tr("Bulk Install"));
        m_ddimView = new PackageSelectView;
        m_ddimView->flushDebList(selectedInfos);
        m_ddimView->setHaveMustInstallDeb(haveMustInstallDeb);
        m_lastPage = m_ddimView;
        m_dragflag = 0;
        m_Filterflag = NonePage;
        m_centralLayout->addWidget(m_ddimView);
        // m_centralLayout->setCurrentIndex(1);
        m_centralLayout->setCurrentWidget(m_ddimView);
        connect(m_ddimView,
                &PackageSelectView::packageInstallConfim,
                this,
                &DebInstaller::slotShowSelectInstallPage);  // 选择完毕信号
    }
}

void DebInstaller::slotShowPkgProcessBlockPage(BackendProcessPage::DisplayMode mode, int currentRate, int pkgCount)
{
    if (m_backendProcessPage == nullptr) {
        m_backendProcessPage = new BackendProcessPage;
        m_centralLayout->addWidget(m_backendProcessPage);
    }

    m_backendProcessPage->setDisplayPage(mode);

    if (mode == BackendProcessPage::PROCESS_FIN) {
        if (m_lastPage != nullptr) {
            if (m_centralLayout->currentWidget() == m_backendProcessPage) {
                m_centralLayout->setCurrentWidget(m_lastPage);
            }
        }
    } else {
        if (mode == BackendProcessPage::READ_PKG) {
            m_backendProcessPage->setPkgProcessRate(currentRate, pkgCount);
        }
        if (m_centralLayout->currentWidget() != m_backendProcessPage) {
            m_centralLayout->setCurrentWidget(m_backendProcessPage);
        }
    }
}

void DebInstaller::disableCloseAndExit()
{
    titlebar()->setDisableFlags(Qt::WindowCloseButtonHint);  // 设置标题栏中的关闭按钮不可用
    DTitlebar *tbar = this->titlebar();
    if (tbar) {
        tbar->setQuitMenuDisabled(true);
    }
}

void DebInstaller::enableCloseAndExit()
{
    titlebar()->setDisableFlags(titlebar()->windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowCloseButtonHint);

    DTitlebar *tbar = this->titlebar();
    if (tbar) {
        tbar->setQuitMenuDisabled(false);
    }
}

void DebInstaller::dragEnterEvent(QDragEnterEvent *dragEnterEvent)
{
    this->activateWindow();                                                      // 拖入时，激活窗口
    if (m_fileListModel->getWorkerStatus() == DebListModel::WorkerProcessing) {  // 如果当前正在安装，不允许拖入包
        this->setAcceptDrops(false);                                             // 不允许拖入
    } else {
        m_fileChooseWidget->setAcceptDrops(true);  // 允许包被拖入
        if (m_dragflag == 0)                       // 如果当前不允许拖入，则直接返回
            return;

        auto *const mime = dragEnterEvent->mimeData();
        if (!mime->hasUrls())
            return dragEnterEvent->ignore();  // 如果当前的数据不是一个路径，直接忽略

        for (const auto &item : mime->urls()) {  // 循环 获取拖入的路径数据
            const QFileInfo info(item.path());
            if (info.isDir())
                return dragEnterEvent->accept();
            if (checkSuffix(item.path()))
                return dragEnterEvent->accept();  // 检查拖入包的后缀
        }
        dragEnterEvent->ignore();
    }
}

void DebInstaller::dropEvent(QDropEvent *dropEvent)
{
    auto *const mime = dropEvent->mimeData();
    if (!mime->hasUrls())
        return dropEvent->ignore();  // 如果数据不是一个路径，忽略

    dropEvent->accept();

    // find .deb files
    QStringList file_list;  // 存放文件列表
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile())
            continue;  // 如果不是本地的文件 忽略
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);

        if (checkSuffix(local_path))  // 检查拖入包的后缀
            file_list << local_path;
        else if (info.isDir()) {
            for (auto deb : QDir(local_path).entryInfoList(QStringList() << "*.deb", QDir::Files))
                file_list << deb.absoluteFilePath();  // 获取文件的绝对路径
        }
    }
    this->activateWindow();               // 激活窗口
    if (!file_list.isEmpty()) {           // 处理拖入文件夹的情况
        slotPackagesSelected(file_list);  // 放到安装列表中
    }
}

void DebInstaller::dragMoveEvent(QDragMoveEvent *dragMoveEvent)
{
    dragMoveEvent->accept();
}

void DebInstaller::slotPackagesSelected(const QStringList &packagesPathList)
{
    this->showNormal();      // 非特效模式下激活窗口
    this->activateWindow();  // 特效模式下激活窗口
    // 如果此时 软件包安装器不是处于准备状态且还未初始化完成或此时正处于正在安装或者卸载状态，则不添加
    // 依赖配置过程中，不添加其他包
    if ((!m_lastPage.isNull() && m_fileListModel->getWorkerStatus() != DebListModel::WorkerPrepare) ||
        DebListModel::WorkerProcessing == m_fileListModel->getWorkerStatus() ||
        DebListModel::WorkerUnInstall == m_fileListModel->getWorkerStatus() || DebListModel::AuthPop == m_wineAuthStatus ||
        DebListModel::AuthConfirm == m_wineAuthStatus || DebListModel::AuthDependsErr == m_wineAuthStatus) {
    } else {
        // 下一指令第1个包大小较大时，解析操作会阻塞当前线程，导致界面设置的逻辑顺序出现混乱，优先处理界面交互，然后再执行加载
        qApp->processEvents();

        // 开始添加包，将要添加的包传递到后端，添加包由后端处理
        m_fileListModel->slotAppendPackage(packagesPathList);
    }
}

void DebInstaller::slotDdimSelected(const QStringList &ddimFiles)
{
    this->activateWindow();

    if (ddimFiles.isEmpty()) {
        return;
    }

    QList<DdimSt> ddimResults;
    bool jsonError = false;
    bool versionError = false;
    bool haveDeb = false;
    for (auto eachFile : ddimFiles) {
        // 0.打开文件
        if (eachFile.endsWith(".deb")) {  // 直接排除掉deb包
            haveDeb = true;
            continue;
        }
        QFile ddimFile(eachFile);
        ddimFile.open(QIODevice::ReadOnly);
        auto jsonData = ddimFile.readAll();
        auto jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isNull()) {
            jsonError = true;
            continue;
        }
        auto jsonObj = jsonDoc.object();

        // 1.校验版本号
        auto version = jsonObj["version"].toString();
        if (version.isNull()) {
            versionError = true;
            continue;
        }

        // 2.根据版本号信息读取JSON文件内容
        // 后续版本号多了以后，需要建立跳转表以进行速度优化，版本号少的时候使用跳转表不划算
        QFileInfo info(ddimFile);
        DdimSt ddimResult;
        auto dirPath = info.absoluteDir().path();
        if (version == "1.0") {
            for (auto &st : ddimResults) {
                if (st.version == "1.0" && st.dirPath == dirPath) {  // ddim文件去重
                    continue;
                }
            }
            ddimResult = analyzeV10(jsonObj, info.absoluteDir().path());
        } else {
            versionError = true;
            continue;  // 无法处理的版本号
        }

        if (!ddimResult.isAvailable) {
            continue;
        }

        ddimResult.dirPath = dirPath;
        ddimResult.version = version;

        // 3.建立列表
        ddimResults.push_back(ddimResult);
    }

    // 4.转入包数据分析模块处理
    if (!ddimResults.isEmpty()) {
        QtConcurrent::run([this, ddimResults]() { m_ddimModel->appendDdimPackages(ddimResults); });
    } else {
        if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel && haveDeb) {  // 处于流程中的报错
            slotShowDdimFloatingMessage(tr("Installing other packages... Please open it later."));
        } else {  // 初次进入时的报错，未定义二次进入时的报错提示
            QString errorString;
            if (jsonError) {
                errorString = tr("Parsing failed: An illegal file structure was found in the manifest file!");
            } else if (versionError) {
                errorString = tr("Parsing failed: An illegal version number was found in the manifest file!");
            } else {  // ddim error
                errorString = tr("No deb packages found. Please check the folder.");
            }
            QMetaObject::invokeMethod(this, "slotShowDdimErrorMessage", Qt::QueuedConnection, Q_ARG(QString, errorString));
        }
    }
}

// 获取文件夹下的文件
void getAllDebFileInDir(const QDir &dir, QFileInfoList &result)
{
    QDir root(dir);
    auto list = root.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (auto &eachInfo : list) {
        if (eachInfo.isDir()) {
            getAllDebFileInDir(eachInfo.absoluteFilePath(), result);
        } else {
            if (eachInfo.absoluteFilePath().endsWith(".deb")) {  // 只抓deb包
                result.push_back(eachInfo);
            }
        }
    }
}

DdimSt DebInstaller::analyzeV10(const QJsonObject &ddimobj, const QString &ddimDir)
{
    // 1.0版ddim解析策略
    Q_UNUSED(ddimobj)
    DdimSt result;

    // 0.搜索三个主要路径
    QDir selectDir(ddimDir + "/Softwares");
    QDir dependDir(ddimDir + "/Depends");
    QDir mustInstallDir(ddimDir + "/Updates");

    // 1.抓取软件包路径
    if (selectDir.exists()) {
        getAllDebFileInDir(selectDir, result.selectList);
    }

    if (dependDir.exists()) {
        getAllDebFileInDir(dependDir, result.dependList);
    }

    if (mustInstallDir.exists()) {
        getAllDebFileInDir(mustInstallDir, result.mustInstallList);
    }

    // 1.1.抓取可选包的应用名
    for (auto &info : result.selectList) {
        auto fileName = info.fileName();
        if (fileName.size() > 4) {
            result.selectAppNameList.push_back(fileName.left(fileName.size() - 4));
        }
    }

    if (!result.selectList.isEmpty() || !result.mustInstallList.isEmpty()) {
        result.token = ddimDir;
        result.isAvailable = true;
    }

    // 2.返回数据
    return result;
}

void DebInstaller::refreshMulti()
{
    // 部分场景下，由于获取后端指针 BackendPtr 等待，使用 Enter 打开多个软件包可能导致状态异常(标识为多个包状态)
    // 场景无法从单包安装正常切换到多包安装，因此在更新界面时判断是否切换当前安装界面。
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (!multiplePage) {
        qWarning() << "Refresh multi install page but not create!";
        single2Multi();
    }

    qInfo() << "[DebInstaller]"
            << "[refreshMulti]"
            << "add a package to multiple page";
    if (SingleInstallerApplication::mode ==
        SingleInstallerApplication::DdimChannel) {  // 之前有多个包，之后又添加了包，则直接刷新listview
        m_dragflag = 0;
    } else {
        m_dragflag = 1;
    }
    MulRefreshPage();
}

void DebInstaller::slotReceiveAppendFailed(Pkg::AppendFailReason reason)
{
    switch (reason) {
        case Pkg::PackageInvalid:
            slotShowInvalidePackageMessage();
            break;
        case Pkg::PackageNotDdim:
            slotShowDdimFloatingMessage(tr("Installing other packages... Please open it later."));
            break;
        case Pkg::PackageNotLocal:
            slotShowNotLocalPackageMessage();
            break;
        case Pkg::PackageNotInstallable:
            slotShowNotInstallablePackageMessage();
            break;
        case Pkg::PackageAlreadyExists:
            slotShowPkgExistMessage();
            break;
        default:
            break;
    }
}

void DebInstaller::slotShowInvalidePackageMessage()
{
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("The deb package may be broken"));
    floatingMsg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);  // 如果损坏，提示
}

void DebInstaller::slotShowNotLocalPackageMessage()
{
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("You can only install local deb packages"));
    floatingMsg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);  // 如果损坏，提示
}

void DebInstaller::slotShowNotInstallablePackageMessage()
{
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("No permission to access this folder"));
    floatingMsg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);  // 如果损坏，提示
}

void DebInstaller::slotShowPkgExistMessage()
{
    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel &&
        m_ddimView == nullptr) {  // 如果选择界面未创建，则表示是第一次进入且只有必装包和依赖包
        slotShowSelectInstallPage({});
        return;
    }

    qWarning() << "DebInstaller:"
               << "package is Exist! ";
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("Already Added"));
    floatingMsg->setIcon(QIcon::fromTheme("di_ok"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);  // 已经添加的包会提示
}

void DebInstaller::slotShowPkgRemovedMessage(QString packageName)
{
    qWarning() << "DebInstaller:" << packageName << "File is not accessible";
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("%1 does not exist, please reselect").arg(packageName));
    floatingMsg->setIcon(QIcon::fromTheme("di_ok"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);  // 已经添加的包会提示
}

void DebInstaller::slotShowDdimErrorMessage(const QString &message)
{
    titlebar()->setTitle(tr("Bulk Install"));
    m_ddimErrorPage = new DdimErrorPage;
    m_ddimErrorPage->setErrorMessage(message);
    m_dragflag = 0;
    m_Filterflag = NonePage;
    m_centralLayout->addWidget(m_ddimErrorPage);
    // m_centralLayout->setCurrentIndex(1);
    m_centralLayout->setCurrentWidget(m_ddimErrorPage);

    connect(m_ddimErrorPage, &DdimErrorPage::comfimPressed, this, &QMainWindow::close);
}

void DebInstaller::slotShowDdimFloatingMessage(const QString &message)
{
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(message);
    floatingMsg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);
}

void DebInstaller::slotShowUninstallConfirmPage()
{
    m_fileListModel->setWorkerStatus(DebListModel::WorkerUnInstall);  // 刷新当前安装器的工作状态

    this->setAcceptDrops(false);  // 卸载页面不允许添加/拖入包

    const QModelIndex index = m_fileListModel->index(0);  // 只有单包才有卸载界面

    m_uninstallPage = new UninstallConfirmPage(this);  // 初始化卸载页面
    m_uninstallPage->setRequiredList(
        index.data(DebListModel::PackageReverseDependsListRole).toStringList());  // 查看是否有包依赖于当前要卸载的包，病获取列表
    m_uninstallPage->setPackage(index.data().toString());                         // 添加卸载提示语

    m_Filterflag = UninstallPage;
    m_centralLayout->addWidget(m_uninstallPage);  // 添加卸载页面到主界面中
    // m_centralLayout->setCurrentIndex(2);                                                        //显示卸载页面
    m_centralLayout->setCurrentWidget(m_uninstallPage);
    m_uninstallPage->setAcceptDrops(false);  // 卸载页面不允许拖入包
    connect(m_uninstallPage,
            &UninstallConfirmPage::signalUninstallAccepted,
            this,
            &DebInstaller::slotUninstallAccepted);  // 卸载页面确认卸载
    connect(m_uninstallPage,
            &UninstallConfirmPage::signalUninstallCanceled,
            this,
            &DebInstaller::slotUninstallCancel);  // 卸载页面取消卸载
}

void DebInstaller::slotUninstallAccepted()
{
    // uninstall begin
    SingleInstallPage *singlePage = backToSinglePage();  // 获取单包安装界面(卸载页面其实也是单包安装页面的一种)
    if (nullptr == singlePage)
        return;
    m_fileChooseWidget->setAcceptDrops(true);   // 设置文件选择界面可以拖入包
    singlePage->slotUninstallCurrentPackage();  // 显示正在卸载页面

    // set close button disabled while uninstalling
    disableCloseAndExit();  // 卸载时不允许关闭或退出

    m_Filterflag = static_cast<CurrentPage>(m_dragflag);
}

void DebInstaller::slotUninstallCancel()
{
    // Cancel uninstall
    this->setAcceptDrops(true);                                     // 取消卸载，允许包被拖入
    m_fileListModel->setWorkerStatus(DebListModel::WorkerPrepare);  // 重置工作状态为准备状态
    backToSinglePage();                                             // 返回单包安装页面

    m_Filterflag = static_cast<CurrentPage>(m_dragflag);
}

void DebInstaller::slotSetAuthingStatus(const bool authing)
{
    // The authorization box pops up, the setting button is not available
    setEnabled(!authing);  // 授权框弹出时，按钮不可用  授权框被关闭后，按钮可用
}

void DebInstaller::slotReset()
{
    // reset page status
    m_dragflag = -1;                  // 是否被允许拖入或添加
    m_Filterflag = ChoosePage;        // 当前显示的页面
    titlebar()->setTitle(QString());  // 重置标题栏
    m_fileListModel->reset();         // 重置model

    // 删除所有的页面
    if (!m_lastPage.isNull() && m_lastPage != m_fileChooseWidget) {
        m_lastPage->deleteLater();
    }
    m_centralLayout->setCurrentWidget(m_fileChooseWidget);

    this->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    m_wineAuthStatus = DebListModel::AuthBefore;
}

void DebInstaller::appendPackageStart()
{
    m_packageAppending = true;
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (multiplePage)
        multiplePage->setEnableButton(false);
}

void DebInstaller::appendFinished()
{
    qDebug() << "[DebInstaller]"
             << "[appendFinished]"
             << "append package finished";
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (multiplePage) {
        multiplePage->setEnableButton(true);
    }
    m_packageAppending = false;
}

void DebInstaller::MulRefreshPage()
{
    if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);  // 获取批量安装类的指针
        if (multiplePage)
            multiplePage->refreshModel();
    }
}

void DebInstaller::single2Multi()
{
    // 刷新文件的状态，初始化包的状态为准备状态
    m_fileListModel->resetInstallStatus();
    if (!m_lastPage.isNull() && m_lastPage != m_fileChooseWidget) {
        m_lastPage->deleteLater();  // 清除widgets缓存
    }

    // multiple packages install
    titlebar()->setTitle(tr("Bulk Install"));

    MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
    multiplePage->setObjectName("MultipleInstallPage");

    connect(multiplePage, &MultipleInstallPage::signalBackToFileChooseWidget, this, &DebInstaller::slotReset);

    multiplePage->refreshModel();
    m_lastPage = multiplePage;
    m_centralLayout->addWidget(multiplePage);

    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel) {
        m_dragflag = 0;
    } else {
        m_dragflag = 1;
    }

    m_Filterflag = MultiPage;

    // m_centralLayout->setCurrentIndex(1);
    m_centralLayout->setCurrentWidget(multiplePage);
}

void DebInstaller::refreshSingle()
{
    // 刷新文件的状态，初始化包的状态为准备状态
    m_fileListModel->resetInstallStatus();
    // clear widgets if needed
    if (!m_lastPage.isNull() && m_lastPage != m_fileChooseWidget) {
        m_lastPage->deleteLater();  // 清除widgets缓存
    }
    // 安装器中只有一个包，刷新单包安装页面
    // 刷新成单包安装界面时，删除标题
    titlebar()->setTitle(QString());

    SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);
    singlePage->setObjectName("SingleInstallPage");
    connect(singlePage, &SingleInstallPage::signalBacktoFileChooseWidget, this, &DebInstaller::slotReset);
    connect(singlePage, &SingleInstallPage::signalRequestUninstallConfirm, this, &DebInstaller::slotShowUninstallConfirmPage);

    m_lastPage = singlePage;
    m_centralLayout->addWidget(singlePage);

    // 重置安装器拖入的状态与工作的状态
    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel) {
        m_dragflag = 0;
        m_Filterflag = NonePage;
    } else {
        m_dragflag = 2;
        m_Filterflag = SinglePage;
    }
    // switch to new page.
    // m_centralLayout->setCurrentIndex(1);
    m_centralLayout->setCurrentWidget(singlePage);
}

SingleInstallPage *DebInstaller::backToSinglePage()
{
    // 获取当前的页面并删除
    QWidget *confirmPage = m_centralLayout->widget(3);
    if (nullptr == confirmPage)
        return nullptr;
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *singleInstallPage = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(2));  // 获取单包安装widget
    if (!singleInstallPage) {
        return nullptr;
    }
    // 返回单包安装页面时，允许添加包
    singleInstallPage->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    this->setAcceptDrops(true);

    return singleInstallPage;
}

void DebInstaller::slotChangeDragFlag()
{
    repaint();
    m_dragflag = 0;  // 允许包被拖入且此时程序中没有包

    enableCloseAndExit();
}

void DebInstaller::slotSetEnableButton(bool bButtonEnabled)
{
    // 如果正在添加包，则启用按钮
    if (m_packageAppending)
        return;
    // Set button enabled after installation canceled
    if (2 == m_dragflag) {  // 单包安装按钮的启用与禁用
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        if (singlePage)
            singlePage->setEnableButton(bButtonEnabled);
    } else if (1 == m_dragflag) {  // 批量安装按钮的启用与禁用
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        if (multiplePage)
            multiplePage->setEnableButton(bButtonEnabled);
    }
}

void DebInstaller::slotShowHiddenButton()
{
    enableCloseAndExit();
    m_fileListModel->resetInstallStatus();  // 授权取消，重置所有的状态，包括安装状态，依赖状态等
    SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
    if (singlePage) {  // 单包安装显示按钮
        singlePage->afterGetAutherFalse();
    } else {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        if (multiplePage) {  // 批量安装显示按钮
            multiplePage->afterGetAutherFalse();
        }
    }
}

void DebInstaller::closeEvent(QCloseEvent *event)
{
    DTitlebar *tbar = this->titlebar();
    if (tbar && tbar->quitMenuIsDisabled()) {
        event->ignore();
        return;
    }

    PackageAnalyzer::instance().setUiExit();
    DMainWindow::closeEvent(event);
}

void DebInstaller::slotDealDependResult(int authDependsStatus, QString dependName)
{
    Q_UNUSED(dependName);
    // Set the display effect according to the status of deepin-wine installation authorization.
    // Before authorization, authorization confirmation, and when the authorization box pops up, it is not allowed to add
    // packages. 依赖下载时、授权时不允许拖入
    m_wineAuthStatus = authDependsStatus;
    if (authDependsStatus == DebListModel::AuthBefore || authDependsStatus == DebListModel::AuthConfirm ||
        authDependsStatus == DebListModel::AuthPop) {
        this->setAcceptDrops(false);
    } else {
        this->setAcceptDrops(true);
    }

    if (authDependsStatus == DebListModel::AuthDependsSuccess) {  // 依赖下载成功
        m_fileListModel->resetInstallStatus();  // 清除包的状态和包的错误原因 重置包的prepare状态
    }
    if (authDependsStatus == DebListModel::AuthBefore) {  // 授权框弹出时
        this->setEnabled(false);                          // 设置界面不可用
    } else {                                              // 授权成功或失败后
        this->setEnabled(true);                           // 根据授权的结果刷新单包或者批量安装界面
        if (m_fileListModel->rowCount() == OnePackage) {  // 刷新单包安装界面
            SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
            if (singlePage)
                singlePage->DealDependResult(authDependsStatus, dependName);
        } else if (m_fileListModel->rowCount() >= TwoPackages) {  // 刷新批量安装界面
            MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
            if (multiplePage) {
                multiplePage->DealDependResult(authDependsStatus, dependName);
                multiplePage->refreshModel();  // 滚动到最后一行。
            }
        }
    }
}

bool DebInstaller::checkSuffix(QString filePath)
{
    const QFileInfo info(filePath);
    if (info.isFile() && info.suffix().toLower() == "deb") {  // 大小写不敏感的判断是否为deb后缀
        return true;
    }
    return false;
}
