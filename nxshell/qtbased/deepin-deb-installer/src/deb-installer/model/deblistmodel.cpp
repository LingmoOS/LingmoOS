// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deblistmodel.h"
#include "manager/packagesmanager.h"
#include "manager/PackageDependsStatus.h"
#include "packageanalyzer.h"
#include "view/pages/AptConfigMessage.h"
#include "view/pages/settingdialog.h"
#include "utils/utils.h"
#include "utils/hierarchicalverify.h"
#include "singleInstallerApplication.h"

#include <DDialog>
#include <DSysInfo>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QSize>
#include <QtConcurrent>

#include <QApt/Backend>
#include <QApt/Package>

using namespace QApt;

/**
 * @brief isDpkgRunning 判断当前dpkg 是否在运行
 * @return
 */

DebListModel::DebListModel(QObject *parent)
    : AbstractPackageListModel(parent)
    , m_packagesManager(new PackagesManager(this))
{
    // 配置包安装的进程
    m_procInstallConfig = new Konsole::Pty;
    configWindow = new AptConfigMessage;

    // 链接信号与槽
    initConnections();
    // 检查系统版本与是否开启了开发者模式
    m_isDevelopMode = Utils::isDevelopMode();
}

bool DebListModel::isDpkgRunning()
{
    QProcess proc;

    // 获取当前的进程信息
    proc.start("ps",
               QStringList() << "-e"
                             << "-o"
                             << "comm");
    proc.waitForFinished();

    // 获取进程信息的数据
    const QString processOutput = proc.readAllStandardOutput();

    // 查看进程信息中是否存在dpkg 存在说明已经正在安装其他包
    if (processOutput.contains("dpkg"))
        return true;  // 更换判断的方式

    return false;
}

const QStringList DebListModel::netErrors()
{
    QStringList errorDetails;
    errorDetails << "Address family for hostname not supported";
    errorDetails << "Temporary failure resolving";
    errorDetails << "Network is unreachable";
    errorDetails << "Cannot initiate the connection to";
    return errorDetails;
}

const QString DebListModel::workerErrorString(const int errorCode, const QString &errorInfo)
{
    switch (errorCode) {
        case FetchError:
        case DownloadDisallowedError:
            return QApplication::translate("DebListModel",
                                           "Installation failed, please check your network connection");  // 网络错误
        case NotFoundError:
            return QApplication::translate("DebListModel", "Installation failed, please check for updates in Control Center");
        case DiskSpaceError:
            return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");  // 存储空间不足
        // lockError 也会报空间不足的错误
        case LockError:
            if (errorInfo.contains("No space left on device")) {
                return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
            }
            break;
        // fix bug:39834 网络断开时，偶现安装deb包失败时提示语不显示
        case CommitError:
            // commit Error 中会报网络错误
            for (auto error : netErrors()) {
                if (errorInfo.contains(error) && errorInfo.contains("http"))
                    return QApplication::translate("DebListModel", "Installation failed, please check your network connection");
            }
            // commitError 有时会出现空间不足的错误
            if (errorInfo.contains("No space left on device")) {
                return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
            }
            break;
        // 无数字签名的错误
        case DebListModel::NoDigitalSignature:
            return QApplication::translate("DebListModel", "No digital signature");

        // 无有效的数字签名
        case DebListModel::DigitalSignatureError:
            return QApplication::translate("DebListModel", "Invalid digital signature");

        // 安装配置包时，没有得到授权
        case DebListModel::ConfigAuthCancel:
            return QApplication::translate("DebListModel", "Authentication failed");
        case DebListModel::ErrorCode::ApplocationProhibit:
            return QApplication::translate("DebListModel",
                                           "The administrator has set policies to prevent installation of this package");
        default:
            break;
    }
    // 其余错误，暂不提示具体的错误原因
    return QApplication::translate("DebListModel", "Installation Failed");
}
void DebListModel::initAppendConnection()
{
    connect(m_packagesManager, &PackagesManager::signalAppendFailMessage, this, &DebListModel::signalAppendFailMessage);

    // 告诉前端当前处在添加过程中
    connect(m_packagesManager, &PackagesManager::signalAppendStart, this, &DebListModel::signalAppendStart);

    // 提示前端当前已经添加完成
    connect(m_packagesManager, &PackagesManager::signalAppendFinished, this, &DebListModel::getPackageMd5);

    // 当前由于文件路径被修改删除md5
    connect(m_packagesManager, &PackagesManager::signalPackageMd5Changed, this, &DebListModel::getPackageMd5);
}

/**
 * @brief DebListModel::initInstallConnecions 链接安装过程的信号与槽
 */
void DebListModel::initInstallConnections()
{
    // 安装成功后，根据安装结果排序
    connect(this, &DebListModel::signalWorkerFinished, this, &DebListModel::slotUpWrongStatusRow);

    // 配置安装结束
    connect(m_procInstallConfig,
            static_cast<void (QProcess::*)(int)>(&QProcess::finished),
            this,
            &DebListModel::slotConfigInstallFinish);

    // 配置安装的过程数据
    connect(m_procInstallConfig, &Konsole::Pty::receivedData, this, &DebListModel::slotConfigReadOutput);

    // 向安装进程中写入配置信息（一般是配置的序号）
    connect(configWindow, &AptConfigMessage::AptConfigInputStr, this, &DebListModel::slotConfigInputWrite);

    // 处理wine依赖安装的过程
    connect(m_packagesManager, &PackagesManager::signalDependResult, this, &DebListModel::slotDealDependResult);

    // 安装wine依赖的时候不允许程序退出
    connect(m_packagesManager, &PackagesManager::signalEnableCloseButton, this, &DebListModel::signalEnableCloseButton);
}

/**
 * @brief DebListModel::initRefreshPageConnecions 链接刷新界面的信号与槽
 */
void DebListModel::initRefreshPageConnecions()
{
    // Refresh the current install page based on the packge count, single / multiple or choose file page.
    connect(m_packagesManager, &PackagesManager::signalPackageCountChanged, this, &DebListModel::signalPackageCountChanged);
}

/**
 * @brief DebListModel::initConnections 链接所有的信号与槽
 */
void DebListModel::initConnections()
{
    // 链接添加时的信号与槽
    initAppendConnection();

    // 链接页面刷新的信号与槽
    initRefreshPageConnecions();

    // 链接安装过程的信号与槽
    initInstallConnections();
}

void DebListModel::slotDealDependResult(int authType, int dependIndex, const QString &dependName)
{
    m_brokenDepend = dependName;
    switch (authType) {
        case DebListModel::CancelAuth:
            m_packageOperateStatus[m_packagesManager->getPackageMd5(dependIndex)] =
                Prepare;  // 取消授权后，缺失wine依赖的包的操作状态修改为prepare
            break;
        case DebListModel::AuthConfirm:  // 确认授权后，状态的修改由debinstaller进行处理
            break;
        case DebListModel::AuthDependsSuccess:  // 安装成功后，状态的修改由debinstaller进行处理
            m_packageOperateStatus[m_packagesManager->getPackageMd5(dependIndex)] = Prepare;
            m_workerStatus = WorkerPrepare;
            break;
        case DebListModel::AuthDependsErr:  // 安装失败后，状态的修改由debinstaller进行处理
            break;
        case DebListModel::VerifyDependsErr:  // 依赖包分级管控验证签名失败，弹出分级设置提示框
            showHierarchicalVerifyWindow();
            break;
        default:
            break;
    }
    emit signalDependResult(authType, dependName);  // 发送信号，由debinstaller处理界面状态。
}

bool DebListModel::isReady() const
{
    return m_packagesManager->isBackendReady();
}

const QList<QString> DebListModel::preparedPackages() const
{
    return m_packagesManager->m_preparedPackages;
}

QModelIndex DebListModel::first() const
{
    return index(0);
}

int DebListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_packagesManager->m_preparedPackages.size();
}

QVariant DebListModel::data(const QModelIndex &index, int role) const
{
    const int currentRow = index.row();
    // 判断当前下标是否越界
    if (currentRow < 0 || currentRow >= m_packagesManager->m_preparedPackages.size()) {
        return QVariant();
    }
    // 当前给出的路径文件已不可访问.直接删除该文件
    if (!recheckPackagePath(m_packagesManager->package(currentRow))) {
        m_packagesManager->removePackage(currentRow);
        return QVariant();
    }
    const DebFile deb(m_packagesManager->package(currentRow));
    if (!deb.isValid())
        return QVariant();
    QString packageName = deb.packageName();            // 包名
    QString filePath = deb.filePath();                  // 包的路径
    QString version = deb.version();                    // 包的版本
    QString architecture = deb.architecture();          // 包可用的架构
    QString shortDescription = deb.shortDescription();  // 包的短描述
    QString longDescription = deb.longDescription();    // 包的长描述 //删除该指针，以免内存泄露
    switch (role) {
        case WorkerIsPrepareRole:
            return isWorkerPrepare();  // 获取当前工作状态是否准备九局
        case ItemIsCurrentRole:
            return m_currentIdx == index;  // 获取当前的index
        case PackageNameRole:
            return packageName;  // 获取当前index包的包名
        case PackagePathRole:
            return filePath;  // 获取当前index包的路径
        case PackageVersionRole:
            return version;  // 获取当前index包的版本
        case PackageVersionStatusRole:
            return m_packagesManager->packageInstallStatus(currentRow);  // 获取当前index包的安装状态
        case PackageDependsStatusRole:
            return m_packagesManager->getPackageDependsStatus(currentRow).status;  // 获取当前index包的依赖状态
        case PackageInstalledVersionRole:
            return m_packagesManager->packageInstalledVersion(currentRow);  // 获取当前index包在系统中安装的版本
        case PackageAvailableDependsListRole:
            return m_packagesManager->packageAvailableDepends(currentRow);  // 获取当前index包可用的依赖
        case PackageReverseDependsListRole:
            return m_packagesManager->packageReverseDependsList(packageName, architecture);  // 获取依赖于当前index包的应用
        case PackageShortDescriptionRole:
            return Utils::fromSpecialEncoding(shortDescription);  // 获取当前index包的短描述
        case PackageLongDescriptionRole:
            return Utils::fromSpecialEncoding(longDescription);  // 获取当前index包的长描述
        case PackageFailReasonRole:
            return packageFailedReason(currentRow);  // 获取当前index包的安装失败的原因
        case PackageOperateStatusRole: {
            auto md5 = m_packagesManager->getPackageMd5(currentRow);
            if (m_packageOperateStatus.contains(md5))  // 获取当前包的操作状态
                return m_packageOperateStatus[md5];
            else
                return Prepare;
        }
        case PackageDependsDetailRole:
            return QVariant::fromValue(m_packagesManager->getPackageDependsDetail(currentRow));
        case Qt::SizeHintRole:  // 设置当前index的大小
            return QSize(0, 48);

        default:
            break;
    }

    return QVariant();
}

bool DebListModel::isDevelopMode()
{
    return m_isDevelopMode;
}

void DebListModel::slotInstallPackages()
{
    if (m_workerStatus != WorkerPrepare)
        return;

    m_workerStatus = WorkerProcessing;  // 刷新包安装器的工作状态
    m_operatingIndex = 0;               // 初始化当前操作的index
    m_operatingStatusIndex = 0;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];

    // start first
    initRowStatus();  // 初始化包的操作状态

    // 检查当前应用是否在黑名单中
    // 非开发者模式且数字签名验证失败
    if (checkBlackListApplication() || !checkDigitalSignature())
        return;
    installNextDeb();  // 开始安装
}

void DebListModel::slotUninstallPackage(const int index)
{
    m_workerStatus = WorkerProcessing;  // 刷新当前包安装器的工作状态
    m_operatingIndex = index;           // 获取卸载的包的indx
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    // fix bug : 卸载失败时不提示卸载失败。
    m_operatingStatusIndex = index;  // 刷新操作状态的index

    DebFile debFile(m_packagesManager->package(m_operatingIndex));  // 获取到包
    if (!debFile.isValid())
        return;
    const QStringList rdepends =
        m_packagesManager->packageReverseDependsList(debFile.packageName(), debFile.architecture());  // 检查是否有应用依赖到该包
    qInfo() << QString("Will remove reverse depends before remove %1 , Lists:").arg(debFile.packageName()) << rdepends;

    Backend *backend = PackageAnalyzer::instance().backendPtr();
    for (const auto &r : rdepends) {  // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (backend->package(r)) {
            // 更换卸载包的方式，remove卸载不卸载完全会在影响下次安装的依赖判断。
            backend->package(r)->setPurge();
        } else
            qWarning() << "DebListModel:"
                       << "reverse depend" << r << "error ,please check it!";
    }
    // 卸载当前包 更换卸载包的方式，remove卸载不卸载完全会在影响下次安装的依赖判断。
    QApt::Package *uninstalledPackage = backend->package(debFile.packageName() + ':' + debFile.architecture());

    // 未通过当前包的包名以及架构名称获取package对象，刷新操作状态为卸载失败
    if (!uninstalledPackage) {
        refreshOperatingPackageStatus(Failed);
        return;
    }
    uninstalledPackage->setPurge();

    refreshOperatingPackageStatus(Operating);  // 刷新当前index的操作状态
    Transaction *transsaction = backend->commitChanges();

    // trans 进度change 链接
    connect(transsaction, &Transaction::progressChanged, this, &DebListModel::signalCurrentPacakgeProgressChanged);

    // 详细状态信息（安装情况）展示链接
    connect(transsaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);

    // trans 运行情况（授权是否成功）
    connect(transsaction, &Transaction::statusChanged, this, &DebListModel::slotTransactionStatusChanged);

    // trans运行中出现错误
    connect(transsaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    // 卸载结束，处理卸载成功与失败的情况并发送结束信号
    connect(transsaction, &Transaction::finished, this, &DebListModel::slotUninstallFinished);

    // 卸载结束之后 删除指针
    connect(transsaction, &Transaction::finished, transsaction, &Transaction::deleteLater);

    m_currentTransaction = transsaction;  // 保存trans指针

    transsaction->run();  // 开始卸载
}

void DebListModel::removePackage(const int idx)
{
    if (WorkerPrepare != m_workerStatus) {
        qWarning() << "installer status error";
    }
    // 去除操作状态 中的index
    int packageOperateStatusCount = m_packageOperateStatus.size() - 1;
    m_packageOperateStatus.clear();
    for (int i = 0; i < packageOperateStatusCount; i++) {
        m_packageOperateStatus[m_packagesManager->getPackageMd5(i)] = DebListModel::Prepare;
    }
    m_packagesManager->removePackage(idx);  // 在packageManager中删除标记的下标
}

int DebListModel::checkInstallStatus(const QString &package_path)
{
    return m_packagesManager->checkInstallStatus(package_path);
}

int DebListModel::checkDependsStatus(const QString &package_path)
{
    return m_packagesManager->checkDependsStatus(package_path).status;
}

int DebListModel::checkDigitalSignature(const QString &package_path)
{
    // 分级管控可用时，交由分级管控进行签名验证
    if (HierarchicalVerify::instance()->isValid()) {
        return Utils::VerifySuccess;
    }

    const auto stat = m_packagesManager->checkDependsStatus(package_path);  // 获取包的依赖状态
    if (stat.isBreak() || stat.isAuthCancel())
        return Utils::VerifySuccess;
    SettingDialog dialog;
    m_isDigitalVerify = dialog.isDigitalVerified();
    int digitalSigntual = Utils::Digital_Verify(package_path);  // 判断是否有数字签名
    if (m_isDevelopMode && !m_isDigitalVerify) {                // 开发者模式且未设置验签功能
        return Utils::VerifySuccess;
    } else if (m_isDevelopMode && m_isDigitalVerify) {  // 开发者模式且设置验签功能
        return digitalSigntual;
    } else {  // 非开发者模式
        return digitalSigntual;
    }
}

QStringList DebListModel::getPackageInfo(const QString &package_path)
{
    return m_packagesManager->getPackageInfo(package_path);
}

QString DebListModel::getInstallErrorMessage()
{
    if (m_currentTransaction)
        return m_currentTransaction->errorString();
    return "faild";
}

QString DebListModel::checkPackageValid(const QString &package_path)
{
    return m_packagesManager->checkPackageValid(QStringList(package_path));
}

void DebListModel::slotAppendPackage(const QStringList &package)
{
    if (WorkerPrepare != m_workerStatus) {
        qWarning() << "installer status error";
    }
    m_packagesManager->appendPackage(package);  // 添加包，并返回添加结果
}

void DebListModel::slotTransactionStatusChanged(TransactionStatus transactionStatus)
{
    switch (transactionStatus) {
        case TransactionStatus::AuthenticationStatus:  // 等待授权
            emit signalLockForAuth(true);              // 设置底层窗口按钮不可用
            break;
        case TransactionStatus::WaitingStatus:  // 当前操作在队列中等待操作
            emit signalLockForAuth(false);      // 设置底层窗口按钮可用
            break;
        default:
            break;
    }
}

void DebListModel::reset()
{
    m_workerStatus = WorkerPrepare;  // 工作状态重置为准备态
    m_operatingIndex = 0;            // 当前操作的index置为0
    m_operatingPackageMd5 = nullptr;
    m_operatingStatusIndex = 0;  // 当前操作状态的index置为0

    m_packageOperateStatus.clear();  // 清空操作状态列表
    m_packageFailCode.clear();       // 清空错误原因列表
    m_packageFailReason.clear();
    m_packagesManager->reset();  // 重置packageManager

    m_hierarchicalVerifyError = false;  // 复位分级管控安装状态
}

int DebListModel::getInstallFileSize()
{
    return m_packagesManager->m_preparedPackages.size();
}

void DebListModel::resetFileStatus()
{
    m_packageOperateStatus.clear();  // 重置包的操作状态
    m_packageFailReason.clear();     // 重置包的错误状态
    m_packageFailCode.clear();
}

void DebListModel::resetInstallStatus()
{
    m_packageOperateStatus.clear();  // 重置包的操作状态
    m_packageFailReason.clear();     // 重置包的错误状态
    m_packageFailCode.clear();

    initPrepareStatus();
}

void DebListModel::bumpInstallIndex()
{
    if (m_currentTransaction.isNull()) {
        qWarning() << "previous transaction not finished";
    }
    if (++m_operatingIndex >= m_packagesManager->m_preparedPackages.size()) {
        m_workerStatus = WorkerFinished;        // 设置包安装器的工作状态为Finish
        emit signalWorkerFinished();            // 发送安装完成信号
        emit signalWholeProgressChanged(100);  // 修改安装进度
        emit signalCurrentPacakgeProgressChanged(100);

        // 安装结束，弹出分级管控信息提示框
        if (m_hierarchicalVerifyError) {
            showHierarchicalVerifyWindow();
            m_hierarchicalVerifyError = false;
        }
        return;
    }
    ++m_operatingStatusIndex;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    emit signalCurrentProcessPackageIndex(m_operatingIndex);  // 修改当前操作的下标
    // install next
    qInfo() << "DebListModel:"
            << "install next deb package";

    // 检查当前应用是否在黑名单中
    // 非开发者模式且数字签名验证失败
    if (checkBlackListApplication() || !checkDigitalSignature())
        return;
    installNextDeb();  // 安装下一个包
}

void DebListModel::slotTransactionErrorOccurred()
{
    if (WorkerProcessing != m_workerStatus) {
        qWarning() << "installer status error" << m_workerStatus;
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction)
        return;
    // 失败时刷新操作状态为failed,并记录失败原因
    refreshOperatingPackageStatus(Failed);
    m_packageOperateStatus[m_operatingPackageMd5] = Failed;

    m_packageFailCode[m_operatingPackageMd5] = transaction->error();
    m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();

    if (!transaction->errorString().contains("proper authorization was not provided"))
        emit signalAppendOutputInfo(transaction->errorString());

    const QApt::ErrorCode errorCode = transaction->error();  // trans错误的代码

    qWarning() << "DebListModel:"
               << "Transaction Error:" << errorCode << workerErrorString(errorCode, transaction->errorString());
    qWarning() << "DebListModel:"
               << "Error Infomation:" << transaction->errorDetails() << transaction->errorString();

    if (transaction->isCancellable())
        transaction->cancel();

    // 特殊处理授权错误
    if (AuthError == errorCode) {
        transaction->deleteLater();  // 删除 trans指针
        QTimer::singleShot(
            100 * 1, this, &DebListModel::checkBoxStatus);  // 检查授权弹窗的状态 如果弹窗仍然在只是超时，则底层窗口按钮不可用
        qWarning() << "DebListModel:"
                   << "Authorization error";

        // 复位，注意先取消锁定状态再设置界面，否则焦点设置丢失
        emit signalLockForAuth(false);  // 取消授权锁定，设置按钮可用
        emit signalAuthCancel();        // 发送授权被取消的信号
        emit signalEnableCloseButton(true);
        m_workerStatus = WorkerPrepare;  // 重置工作状态为准备态
        return;
    }

    // DO NOT install next, this action will finished and will be install next automatic.
    transaction->setProperty("exitStatus", QApt::ExitFailed);  // 设置trans的退出状态为 失败
}

void DebListModel::refreshOperatingPackageStatus(PackageOperationStatus operationStatus)
{
    m_packageOperateStatus[m_operatingPackageMd5] = operationStatus;  // 将失败包的索引和状态修改保存,用于更新

    const QModelIndex modelIndex = index(m_operatingStatusIndex);

    emit dataChanged(modelIndex, modelIndex);  // 发送状态已经修改的信号
}

QString DebListModel::packageFailedReason(const int idx) const
{
    const auto dependStatus = m_packagesManager->getPackageDependsStatus(idx);  // 获取包的依赖状态
    const auto md5 = m_packagesManager->getPackageMd5(idx);                     // 获取包的md5值
    if (m_packagesManager->isArchError(idx))
        return tr("Unmatched package architecture");  // 判断是否架构冲突
    if (dependStatus.isProhibit())
        return tr("The administrator has set policies to prevent installation of this package");
    if (dependStatus.isBreak() || dependStatus.isAuthCancel()) {  // 依赖状态错误
        if (!dependStatus.package.isEmpty() || !m_brokenDepend.isEmpty()) {
            if (m_packagesManager->m_errorIndex.contains(md5)) {  // 修改wine依赖的标记方式
                auto ret = static_cast<DebListModel::DependsAuthStatus>(m_packagesManager->m_errorIndex.value(md5));
                switch (ret) {
                    case DebListModel::VerifyDependsErr:
                        return m_brokenDepend + tr("Invalid digital signature");
                    default:
                        return tr("Failed to install %1").arg(m_brokenDepend);  // wine依赖安装失败
                }
            }
            return tr("Broken dependencies: %1").arg(dependStatus.package);  // 依赖不满足
        }

        const auto conflictStatus = m_packagesManager->packageConflictStat(idx);  // 获取冲突情况
        if (!conflictStatus.is_ok())
            return tr("Broken dependencies: %1").arg(conflictStatus.unwrap());  // 依赖冲突
    }

    // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
    return workerErrorString(m_packageFailCode[md5], m_packageFailReason[md5]);  // 根据错误代码和错误原因返回具体的错误原因
}

void DebListModel::slotTransactionFinished()
{
    if (m_workerStatus == WorkerProcessing) {
        qWarning() << "installer status still processing";
    }
    // 获取trans指针
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction)
        return;
    // prevent next signal
    disconnect(transaction, &Transaction::finished, this, &DebListModel::slotTransactionFinished);  // 不再接收trans结束的信号

    // report new progress
    // 更新安装进度（批量安装进度控制）
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());
    emit signalWholeProgressChanged(progressValue);

    qInfo() << "DebListModel:"
            << "transaciont finished with exit status:" << transaction->exitStatus();
    if (transaction->exitStatus()) {
        // 安装失败
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();
        // 检测错误信息是否包含分级管控错误码，若存在，则当前错误为分级管控验证签名不通过
        QString errorInfo = transaction->errorDetails();
        if (errorInfo.isEmpty()) {
            errorInfo = transaction->errorString();
        }
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        bool verifyError = HierarchicalVerify::instance()->checkTransactionError(sPackageName, errorInfo);

        // 检测安装失败时，弹出对话框提示
        if (verifyError) {
            // 安装结束后再弹出提示对话框
            m_hierarchicalVerifyError = true;
        }

        // 保存错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode[m_operatingPackageMd5] =
            verifyError ? static_cast<int>(DebListModel::DigitalSignatureError) : static_cast<int>(transaction->error());
        m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();

        // 刷新操作状态
        refreshOperatingPackageStatus(Failed);
        emit signalAppendOutputInfo(transaction->errorString());
    } else if (m_packageOperateStatus.contains(m_operatingPackageMd5) &&
               m_packageOperateStatus[m_operatingPackageMd5] != Failed) {
        // 安装成功
        refreshOperatingPackageStatus(Success);

        // 准备安装下一个包，修改下一个包的状态为正在安装状态
        if (m_operatingStatusIndex < m_packagesManager->m_preparedPackages.size() - 1) {
            auto md5 = m_packagesManager->getPackageMd5(m_operatingIndex + 1);
            m_packageOperateStatus[md5] = Waiting;
        }
    }
    //    delete trans;
    if (!m_currentTransaction.isNull()) {
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }
    transaction = nullptr;
    bumpInstallIndex();  // 进入安装进度控制
}

void DebListModel::slotDependsInstallTransactionFinished()  // 依赖安装关系满足
{
    if (m_workerStatus == WorkerProcessing) {
        qWarning() << "installer status still processing";
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction)
        return;

    const auto transExitStatus = transaction->exitStatus();

    if (transExitStatus) {
        // record error
        // 记录错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();  // 向终端打印错误
        m_packageFailCode[m_operatingPackageMd5] = transaction->error();
        m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();
        refreshOperatingPackageStatus(Failed);  // 刷新操作状态
        emit signalAppendOutputInfo(transaction->errorString());
    }

    //    delete trans;
    if (!m_currentTransaction.isNull()) {
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }
    transaction = nullptr;

    // check current operate exit status to install or install next
    if (transExitStatus) {
        bumpInstallIndex();  // 依赖安装失败，直接安装下一个包
    } else {
        // 安装依赖前已对此包进行黑名单及验签校验，无需二次验证
        installNextDeb();  // 依赖安装成功，开始安装这个包
    }
}

void DebListModel::setEndEnable()
{
    emit signalEnableReCancelBtn(true);
}

void DebListModel::checkBoxStatus()
{
    QTime startTime = QTime::currentTime();  // 获取弹出的时间
    Transaction *transation = nullptr;
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    transation = backend->commitChanges();

    QTime stopTime = QTime::currentTime();
    int elapsed = startTime.msecsTo(stopTime);  // 获取commit授权被取消的时间
    if (elapsed > 20000) {                      // 如果时间超过20ms则不断判断当前窗口是否超时
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);
        return;
    }

    if (transation) {
        if (transation->isCancellable()) {  // 当前transaction已经被取消
            transation->cancel();
            QTimer::singleShot(100 * 1, this, &DebListModel::setEndEnable);  // 设置按钮可用
        } else {
            QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);  // 当前Transaction还在运行中，继续等待并判断
        }
    } else {
        qWarning() << "DebListModel:"
                   << "Transaction is Nullptr";
    }
}

void DebListModel::installDebs()
{
    DebFile deb(m_packagesManager->package(m_operatingIndex));
    if (!deb.isValid())
        return;
    qInfo()
        << QString("Prepare to install %1, ver: %2, arch: %3").arg(deb.packageName()).arg(deb.version()).arg(deb.architecture());

    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");
    // 在判断dpkg启动之前就发送开始安装的信号，并在安装信息中输出 dpkg正在运行的信息。
    emit signalStartInstall();

    // fetch next deb
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    if (!backend)
        return;

    Transaction *transaction = nullptr;

    // reset package depends status
    m_packagesManager->resetPackageDependsStatus(m_operatingStatusIndex);

    // check available dependencies
    const auto dependsStat = m_packagesManager->getPackageDependsStatus(m_operatingStatusIndex);
    if (dependsStat.isBreak() || dependsStat.isAuthCancel() ||
        dependsStat.status == DebListModel::ArchBreak) {  // 依赖不满足或者下载wine依赖时授权被取消
        refreshOperatingPackageStatus(Failed);            // 刷新错误状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, -1);  // 保存错误原因
        // 记录详细错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, packageFailedReason(m_operatingStatusIndex));

        bumpInstallIndex();  // 开始下一步的安装流程
        return;
    } else if (dependsStat.isAvailable()) {
        if (isDpkgRunning()) {
            qInfo() << "DebListModel:"
                    << "dpkg running, waitting...";
            // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
            QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
            return;
        }
        // 依赖可用 但是需要下载
        Q_ASSERT_X(m_packageOperateStatus[m_operatingPackageMd5],
                   Q_FUNC_INFO,
                   "package operate status error when start install availble dependencies");

        // 获取到所有的依赖包 准备安装
        const QStringList availableDepends = m_packagesManager->packageAvailableDepends(m_operatingIndex);
        qInfo() << QString("Prepare install package: %1 , install depends: ").arg(deb.packageName()) << availableDepends;

        // 获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
        for (auto const &p : availableDepends) {
            if (p.contains(" not found")) {             // 依赖安装失败
                refreshOperatingPackageStatus(Failed);  // 刷新当前包的状态
                // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
                m_packageFailCode.insert(m_operatingPackageMd5, DownloadDisallowedError);  // 记录错误代码与错误原因
                m_packageFailReason.insert(m_operatingPackageMd5, p);
                emit signalAppendOutputInfo(m_packagesManager->package(m_operatingIndex) + "\'s depend " + " " +
                                            p);  // 输出错误原因
                bumpInstallIndex();              // 开始安装下一个包或结束安装

                qWarning() << QString("Packge %1 install failed, not found depend package: %2").arg(deb.packageName()).arg(p);
                return;
            }
            backend->markPackageForInstall(p);  // 开始安装依赖包
        }
        // 打印待安装的软件包信息
        printDependsChanges();

        transaction = backend->commitChanges();
        if (!transaction)
            return;
        // 依赖安装结果处理
        connect(transaction, &Transaction::finished, this, &DebListModel::slotDependsInstallTransactionFinished);
    } else {
        if (isDpkgRunning()) {
            qInfo() << "DebListModel:"
                    << "dpkg running, waitting...";
            // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
            QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
            return;
        }
        transaction = backend->installFile(deb);  // 触发Qapt授权框和安装线程
        if (!transaction)
            return;
        // 进度变化和结束过程处理
        connect(transaction, &Transaction::progressChanged, this, &DebListModel::signalCurrentPacakgeProgressChanged);
        connect(transaction, &Transaction::finished, this, &DebListModel::slotTransactionFinished);
    }

    // NOTE: DO NOT remove this.
    transaction->setLocale(".UTF-8");

    // 记录日志
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);

    // 刷新操作状态
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::slotTransactionOutput);

    // 授权处理
    connect(transaction, &Transaction::statusChanged, this, &DebListModel::slotTransactionStatusChanged);

    // 错误处理
    connect(transaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    m_currentTransaction = transaction;

    m_currentTransaction->run();
}

void DebListModel::digitalVerifyFailed(ErrorCode errorCode)
{
    if (preparedPackages().size() > 1) {        // 批量安装
        refreshOperatingPackageStatus(Failed);  // 刷新操作状态
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, errorCode);  // 记录错误代码与错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();  // 跳过当前包
    } else if (preparedPackages().size() == 1) {
        if (!m_isDevelopMode) {
            exit(0);
        } else {                                    // 开发者模式下，点击取消按钮，返回错误界面
            refreshOperatingPackageStatus(Failed);  // 刷新操作状态
            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, errorCode);  // 记录错误代码与错误原因
            m_packageFailReason.insert(m_operatingPackageMd5, "");
            emit signalWorkerFinished();
        }
    }
}

void DebListModel::showNoDigitalErrWindowInDdimProcess(void (DebListModel::*failedFunction)())
{
    DDialog *Ddialog = new DDialog();  // 弹出窗口
    Ddialog->setModal(true);
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);  // 窗口一直置顶
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);  // 添加确认按钮

    auto fullPath = m_packagesManager->package(m_operatingIndex);
    QFileInfo info(fullPath);
    Ddialog->setMessage(QString(tr("Failed to install %1: no valid digital signature").arg(info.fileName())) + QString("!"));

    // 消息框reject后的操作，包括点击取消按钮、关闭图标、按ESC退出
    std::function<void(void)> rejectOperate = [this, Ddialog, failedFunction]() {
        if (failedFunction) {
            (this->*failedFunction)();
        }
        Ddialog->deleteLater();
    };

    // 取消按钮
    QPushButton *btnOk = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(btnOk, &DPushButton::clicked, Ddialog, &DDialog::reject);

    // 关闭图标
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::reject);

    // ESC退出
    connect(Ddialog, &DDialog::rejected, rejectOperate);

    Ddialog->exec();  // 显示弹窗
}

void DebListModel::showNoDigitalErrWindow()
{
    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel) {
        showNoDigitalErrWindowInDdimProcess(&DebListModel::slotNoDigitalSignature);
        return;
    }

    // 批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(NoDigitalSignature);  // 刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();  // 弹出窗口
    Ddialog->setModal(true);
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);  // 窗口一直置顶
    Ddialog->setTitle(tr("Unable to install - no digital signature"));
    Ddialog->setMessage(QString(tr("Please go to Control Center to enable developer mode and try again. Proceed?")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));

    Ddialog->addButton(QString(tr("Cancel", "button")), true, DDialog::ButtonNormal);      // 添加取消按钮
    Ddialog->addButton(QString(tr("Proceed", "button")), true, DDialog::ButtonRecommend);  // 添加前往按钮
    Ddialog->show();                                                                       // 显示弹窗

    // 消息框reject后的操作，包括点击取消按钮、关闭图标、按ESC退出
    std::function<void(void)> rejectOperate = [this, Ddialog]() {
        this->slotNoDigitalSignature();
        Ddialog->deleteLater();
    };

    // 取消按钮
    QPushButton *btnCancel = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(btnCancel, &DPushButton::clicked, rejectOperate);

    // 关闭图标
    connect(Ddialog, &DDialog::aboutToClose, rejectOperate);

    // ESC退出
    connect(Ddialog, &DDialog::rejected, rejectOperate);

    // 前往按钮1
    QPushButton *btnProceedControlCenter = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(btnProceedControlCenter, &DPushButton::clicked, this, &DebListModel::slotShowDevelopModeWindow);
    connect(btnProceedControlCenter, &DPushButton::clicked, this, &QApplication::exit);
    connect(btnProceedControlCenter, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
}

void DebListModel::showDigitalErrWindow(bool recordError)
{
    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel) {
        // 不记录错误时仅提示，不涉及状态切换及更新记录
        showNoDigitalErrWindowInDdimProcess(recordError ? &DebListModel::slotDigitalSignatureError : nullptr);
        return;
    }

    // 批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        if (recordError) {
            digitalVerifyFailed(DigitalSignatureError);  // 刷新安装错误，并记录错误原因
        }
        return;
    }
    DDialog *Ddialog = new DDialog();
    // 设置窗口焦点
    Ddialog->setFocusPolicy(Qt::TabFocus);

    // 设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    // 设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature")) + QString("!"));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    Ddialog->show();
    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    btnOK->setFocusPolicy(Qt::TabFocus);
    btnOK->setFocus();

    // 窗口退出操作，包括所有可以退出此窗口的操作
    std::function<void(void)> exitOperate = [this, Ddialog, recordError]() {
        if (recordError) {
            this->slotDigitalSignatureError();
        }
        Ddialog->deleteLater();
    };

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, exitOperate);

    // 点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, exitOperate);

    // ESC退出
    connect(Ddialog, &DDialog::rejected, exitOperate);
}

void DebListModel::showDevelopDigitalErrWindow(ErrorCode code)
{
    Dialog *Ddialog = new Dialog();
    // 设置窗口焦点
    // fix bug:https://pms.uniontech.com/zentao/bug-view-44837.html
    Ddialog->setFocusPolicy(Qt::TabFocus);

    // 设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    // 设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature. Continue with the installation?")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("Cancel")), true, DDialog::ButtonNormal);
    Ddialog->addButton(QString(tr("Continue", "button")), true, DDialog::ButtonRecommend);  // 添加前往按钮

    Ddialog->show();
    QPushButton *cancelBtn = qobject_cast<QPushButton *>(Ddialog->getButton(0));

    cancelBtn->setFocusPolicy(Qt::TabFocus);
    cancelBtn->setFocus();

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, this, [=] {
        // 刷新当前包的操作状态，失败原因为数字签名校验失败
        digitalVerifyFailed(code);
    });
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::deleteLater);

    // 点击弹出窗口的确定按钮
    connect(cancelBtn, &DPushButton::clicked, this, [=] { digitalVerifyFailed(code); });
    connect(cancelBtn, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);

    QPushButton *continueBtn = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(continueBtn, &DPushButton::clicked, this, [&] { installNextDeb(); });  // 点击继续，进入安装流程
    connect(continueBtn, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
    connect(Ddialog, &Dialog::signalClosed, this, [=] { digitalVerifyFailed(code); });
    connect(Ddialog, &Dialog::signalClosed, Ddialog, &DDialog::deleteLater);
}

void DebListModel::slotDigitalSignatureError()
{
    digitalVerifyFailed(DigitalSignatureError);
}

void DebListModel::slotNoDigitalSignature()
{
    digitalVerifyFailed(NoDigitalSignature);
}

void DebListModel::slotShowDevelopModeWindow()
{
    // 弹出设置 通用窗口

    // 1.读取系统版本号
    QProcess *unlock = new QProcess(this);
    unlock->start("lsb_release", { "-r" });
    unlock->waitForFinished();
    auto output = unlock->readAllStandardOutput();
    auto str = QString::fromUtf8(output);
    QRegExp re("\t.+\n");
    QString osVerStr;
    if (re.indexIn(str) > -1) {
        auto result = re.cap(0);
        osVerStr = result.remove(0, 1).remove(result.size() - 1, 1);
        qInfo() << "lsb_release -r:" << output;
        qInfo() << "OS version:" << osVerStr;
    }

    // 2.打开控制中心
    if (osVerStr == "20") {  // V20模式
        QDBusInterface interface("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter");
        if (interface.isValid()) {
            interface.call("ShowPage", "commoninfo", "Developer Mode");
        }

        QDBusError error = interface.lastError();
        if (error.isValid()) {
            qWarning() << QString("DBus ControlCenter.ShowPage failed, Type: %1 MSG: %2").arg(error.type()).arg(error.message());
        }

    } else if (osVerStr == "23") {  // V23模式
        if (unlock->exitCode() != QProcess::NormalExit) {
            QDBusInterface interface(
                "org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1", "org.deepin.dde.ControlCenter1");
            if (interface.isValid()) {
                interface.call("ShowPage", "commoninfo", "Developer Mode");
            }

            QDBusError error = interface.lastError();
            if (error.isValid()) {
                qWarning()
                    << QString("DBus ControlCenter.ShowPage failed, Type: %1 MSG: %2").arg(error.type()).arg(error.message());
            }
        }

    } else {
        qWarning() << qPrintable("Unknown OS version, connot open dde-control-center");
    }

    unlock->deleteLater();
}

bool DebListModel::checkDigitalSignature()
{
    // 分级管控可用时，交由分级管控进行签名验证
    if (HierarchicalVerify::instance()->isValid()) {
        return true;
    }

    const auto stat = m_packagesManager->getPackageDependsStatus(m_operatingIndex);  // 获取包的依赖状态
    if (stat.isBreak() || stat.isAuthCancel())
        return true;
    SettingDialog dialog;
    m_isDigitalVerify = dialog.isDigitalVerified();
    int digitalSigntual = Utils::Digital_Verify(m_packagesManager->package(m_operatingIndex));  // 判断是否有数字签名
    qInfo() << "m_isDevelopMode:" << m_isDevelopMode << " /m_isDigitalVerify:" << m_isDigitalVerify
            << " /digitalSigntual:" << digitalSigntual;
    if (m_isDevelopMode && !m_isDigitalVerify) {  // 开发者模式且未设置验签功能
        return true;
    } else if (m_isDevelopMode && m_isDigitalVerify) {  // 开发者模式且设置验签功能
        if (digitalSigntual == Utils::VerifySuccess) {
            return true;
        } else {
            ErrorCode code;
            if (digitalSigntual == Utils::DebfileInexistence)
                code = NoDigitalSignature;
            else
                code = DigitalSignatureError;
            showDevelopDigitalErrWindow(code);  // 弹出提示框
            return false;
        }
    } else {  // 非开发者模式
        bool verifiedResult = false;
        switch (digitalSigntual) {
            case Utils::VerifySuccess:  // 签名验证成功
                verifiedResult = true;
                break;
            case Utils::DebfileInexistence:  // 无签名文件
                showNoDigitalErrWindow();
                verifiedResult = false;
                break;
            case Utils::ExtractDebFail:  // 无有效的数字签名
                showDigitalErrWindow();
                verifiedResult = false;
                break;
            case Utils::DebVerifyFail:
            case Utils::OtherError:  // 其他原因造成的签名校验失败
                showDigitalErrWindow();
                verifiedResult = false;
                break;
            default:  // 其他未知错误
                qInfo() << "unknown mistake";
                verifiedResult = false;
                break;
        }
        return verifiedResult;
    }
}

void DebListModel::installNextDeb()
{
    m_packagesManager->resetPackageDependsStatus(m_operatingStatusIndex);  // 刷新软件包依赖状态
    auto dependStatus = m_packagesManager->getPackageDependsStatus(m_operatingStatusIndex);
    if (dependStatus.isAvailable()) {  // 存在没有安装的依赖包，则进入普通安装流程执行依赖安装
        installDebs();
    } else if (dependStatus.status >= DependsBreak) {  // 安装前置条件不满足，无法处理
        refreshOperatingPackageStatus(Failed);
        bumpInstallIndex();
        return;
    } else {  // 如果当前包的依赖全部安装完毕，则进入配置判断流程
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        if (checkTemplate(sPackageName)) {  // 检查当前包是否需要配置
            rmdir();                        // 删除临时路径
            m_procInstallConfig->start("pkexec",
                                       QStringList() << "pkexec"
                                                     << "deepin-deb-installer-dependsInstall"
                                                     << "InstallConfig" << sPackageName,
                                       {},
                                       0,
                                       false);  // 配置安装流程
        } else {
            installDebs();  // 普通安装流程
        }
    }
}

void DebListModel::rmdir()
{
    QDir filePath(tempPath);
    if (filePath.exists()) {
        if (!filePath.removeRecursively()) {
            qWarning() << "DebListModel:"
                       << "remove temporary path failed";
        }
    }
}

bool DebListModel::checkTemplate(const QString &debPath)
{
    rmdir();
    getDebian(debPath);
    QFile templates(tempPath + "/templates");
    if (templates.exists()) {
        qInfo() << "DebListModel:"
                << "Check that the template file exists";
        return true;
    }
    return false;
}

bool DebListModel::mkdir()
{
    QDir filePath(tempPath);  // 获取配置包的临时路径

    if (!filePath.exists()) {             // 当前临时路径不存在
        return filePath.mkdir(tempPath);  // 删除临时路径，并返回删除结果
    }
    return true;
}

void DebListModel::getDebian(const QString &debPath)
{
    if (!mkdir()) {                                                 // 创建临时路径
        qWarning() << "check error mkdir" << tempPath << "failed";  // 创建失败
        return;
    }
    QProcess *m_pDpkg = new QProcess(this);
    m_pDpkg->start("dpkg", QStringList() << "-e" << debPath << tempPath);  // 获取DEBIAN文件，查看当前包是否需要配置
    m_pDpkg->waitForFinished();
    QString getDebianProcessErrInfo = m_pDpkg->readAllStandardError();
    if (!getDebianProcessErrInfo.isEmpty()) {
        qWarning() << "DebListModel:"
                   << "Failed to decompress the main control file" << getDebianProcessErrInfo;
    }
    delete m_pDpkg;
}

void DebListModel::slotTransactionOutput()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans)
        return;

    refreshOperatingPackageStatus(Operating);  // 刷新当前包的操作状态

    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::slotTransactionOutput);
}

void DebListModel::slotUninstallFinished()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }

    // 增加卸载失败的情况
    // 此前的做法是发出commitError的信号，现在全部在Finished中进行处理。不再特殊处理。
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans)
        return;

    if (trans->exitStatus()) {
        m_workerStatus = WorkerFinished;        // 刷新包安装器的工作状态
        refreshOperatingPackageStatus(Failed);  // 刷新当前包的操作状态
        m_packageOperateStatus[m_operatingPackageMd5] = Failed;
        qWarning() << "DebListModel:"
                   << "uninstall finished with finished code:" << trans->error() << "finished details:" << trans->errorString();
    } else {
        m_workerStatus = WorkerFinished;         // 刷新包安装器的工作状态
        refreshOperatingPackageStatus(Success);  // 刷新当前包的卸载状态
        m_packageOperateStatus[m_operatingPackageMd5] = Success;
    }
    emit signalWorkerFinished();  // 发送结束信号（只有单包卸载）卸载结束就是整个流程的结束
    trans->deleteLater();
}

void DebListModel::slotSetCurrentIndex(const QModelIndex &modelIndex)
{
    if (m_currentIdx == modelIndex)
        return;  // 要修改的index与当前index 一致

    const QModelIndex index = m_currentIdx;  // 保存当前的index
    m_currentIdx = modelIndex;               // 修改当前的index

    emit dataChanged(index, index);
    emit dataChanged(m_currentIdx, m_currentIdx);  // 发送index修改信号
}

void DebListModel::initPrepareStatus()
{
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        auto md5 = m_packagesManager->getPackageMd5(i);
        m_packageOperateStatus.insert(md5, Prepare);  // 刷新当前所有包的状态为Prepare
    }
}

void DebListModel::initRowStatus()
{
    // 更换状态存储方式后修改更新状态的方式
    for (auto md5 : m_packageMd5) {
        m_packageOperateStatus[md5] = Waiting;
    }
}

void DebListModel::slotUpWrongStatusRow()
{
    if (m_packagesManager->m_preparedPackages.size() == 1)
        return;

    QList<QByteArray> installErrorPackages;    // 安装错误的包的list
    QList<QByteArray> installSuccessPackages;  // 安装成功的包的list

    // 根据包的操作状态，分别找到所有安装成功的包与安装失败的包
    QMapIterator<QByteArray, int> iteratorpackageOperateStatus(m_packageOperateStatus);
    while (iteratorpackageOperateStatus.hasNext()) {
        iteratorpackageOperateStatus.next();
        // 保存安装成功的包
        if (iteratorpackageOperateStatus.value() == Failed ||
            iteratorpackageOperateStatus.value() == VerifyFailed) {           // 安装失败或签名验证失败
            installErrorPackages.append(iteratorpackageOperateStatus.key());  // 保存下标
        }
        // 保存安装失败的包
        if (iteratorpackageOperateStatus.value() == Success) {
            installSuccessPackages.append(iteratorpackageOperateStatus.key());
        }
    }
    if (installErrorPackages.size() == 0)  // 全部安装成功 直接退出
        return;

    // 先将包与md5 绑定
    // 后续要对根据MD5对包的路径进行排序，保证包名和md5的下标统一
    QMap<QByteArray, QString> md5Packages;
    for (int i = 0; i < m_packagesManager->m_packageMd5.size(); i++) {
        md5Packages.insert(m_packagesManager->m_packageMd5[i], m_packagesManager->m_preparedPackages[i]);
    }

    m_packagesManager->m_packageMd5.clear();
    m_packagesManager->m_packageMd5.append(installErrorPackages);
    m_packagesManager->m_packageMd5.append(installSuccessPackages);

    m_packagesManager->m_preparedPackages.clear();
    for (int i = 0; i < m_packagesManager->m_packageMd5.size(); i++) {
        m_packagesManager->m_preparedPackages.append(md5Packages[m_packagesManager->m_packageMd5[i]]);
    }

    // update view
    const QModelIndex idxStart = index(0);
    const QModelIndex idxEnd = index(m_packageOperateStatus.size() - 1);
    emit dataChanged(idxStart, idxEnd);

    // update scroll
    emit signalCurrentProcessPackageIndex(-1);
}

void DebListModel::slotConfigInstallFinish(int installResult)
{
    if (m_packagesManager->m_preparedPackages.size() == 0)
        return;
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) /
                                         m_packagesManager->m_preparedPackages.size());  // 批量安装时对进度进行处理
    emit signalWholeProgressChanged(progressValue);
    if (0 == installResult) {  // 安装成功
        if (m_packagesManager->m_packageMd5DependsStatus[m_packagesManager->m_packageMd5[m_operatingIndex]].status == DependsOk) {
            refreshOperatingPackageStatus(Success);  // 刷新安装状态
            m_procInstallConfig->terminate();        // 结束配置
            m_procInstallConfig->close();
        }
        bumpInstallIndex();  // 开始安装下一个
    } else {
        if (1 == m_packagesManager->m_preparedPackages.size()) {  // 单包安装
            refreshOperatingPackageStatus(Prepare);               // 刷新当前包的操作状态为准备态
            m_workerStatus = WorkerPrepare;
            emit signalAuthCancel();  // 授权取消
        } else {
            // 批量安装
            refreshOperatingPackageStatus(Failed);  // 刷新当前包的状态为失败

            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, installResult);  // 保存失败原因
            m_packageFailReason.insert(m_operatingPackageMd5, "Authentication failed");
            bumpInstallIndex();  // 开始安装下一个
        }
    }
    configWindow->hide();        // 隐藏配置窗口
    configWindow->clearTexts();  // 清楚配置信息
    //    m_procInstallConfig->terminate();                               //结束配置
    //    m_procInstallConfig->close();
}

void DebListModel::slotConfigReadOutput(const char *buffer, int length, bool isCommandExec)
{
    QString tmp = QByteArray(buffer, length);  // 获取配置读取到的信息

    tmp.remove(QChar('"'), Qt::CaseInsensitive);
    tmp.remove(QChar('\n'), Qt::CaseInsensitive);

    // 取消授权弹窗，则不显示配置安装界面
    if (!tmp.contains("Error executing command as another user: Request dismissed")) {
        // 获取到当前正在安装配置
        emit signalStartInstall();
        refreshOperatingPackageStatus(Operating);  // 刷新当前的操作状态
        configWindow->show();                      // 显示配置窗口

        int iCutoutNum = tmp.size();
        if (iCutoutNum > 0) {
            emit signalAppendOutputInfo(tmp);  // 原本安装信息界面信息也要添加，以备安装完成后查看安装信息
            configWindow->appendTextEdit(tmp);  // 配置包安装信息界面显示配置信息
        }
    }
}

void DebListModel::slotConfigInputWrite(const QString &str)
{
    m_procInstallConfig->pty()->write(str.toUtf8());  // 将用户输入的配置项写入到配置安装进程中。
    m_procInstallConfig->pty()->write("\n");          // 写入换行，配置生效
}

void DebListModel::slotCheckInstallStatus(const QString &installInfo)
{
    // 判断当前的信息是否是错误提示信息
    if (installInfo.contains("Error executing command as another user: Request dismissed")) {
        emit signalAppendOutputInfo(installInfo);  // 输出安装错误的原因
        m_workerStatus = WorkerFinished;           // 刷新包安装器的工作状态

        refreshOperatingPackageStatus(Failed);  // 刷新当前包的操作状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageOperateStatus[m_operatingPackageMd5] = Failed;
        m_packageFailCode.insert(m_operatingPackageMd5, 0);  // 保存失败原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();
        return;
    }
}

bool DebListModel::recheckPackagePath(const QString &packagePath) const
{
    QFile packagePathFile(packagePath);
    do {
        if (packagePathFile.symLinkTarget().isEmpty()) {
            if (packagePathFile.exists()) {
                return true;
            }
        } else {
            QFile realPath(packagePathFile.symLinkTarget());
            if (realPath.exists() && packagePathFile.exists()) {
                return true;
            }
        }
    } while (false);
    QFileInfo fileInfo(packagePath);
    qWarning() << "check file path" << packagePath << "source file and link file not exist ";
    emit signalPackageCannotFind(fileInfo.fileName());
    return false;
}

void DebListModel::getPackageMd5(const QList<QByteArray> &packagesMD5)
{
    m_packageMd5.clear();
    m_packageMd5 = packagesMD5;
    emit signalAppendFinished();
}

void DebListModel::slotShowProhibitWindow()
{
    digitalVerifyFailed(ApplocationProhibit);
}
void DebListModel::showProhibitWindow()
{
    // 批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(ApplocationProhibit);  // 刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();
    // 设置窗口焦点
    Ddialog->setFocusPolicy(Qt::TabFocus);

    // 设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    // 设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("The administrator has set policies to prevent installation of this package")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    Ddialog->show();

    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    btnOK->setFocusPolicy(Qt::TabFocus);
    btnOK->setFocus();

    // 窗口退出操作，包括所有可以退出此窗口的操作
    std::function<void(void)> exitOperate = [this, Ddialog]() {
        this->slotShowProhibitWindow();
        Ddialog->deleteLater();
    };

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, exitOperate);

    // 点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, exitOperate);

    // ESC
    connect(Ddialog, &DDialog::rejected, exitOperate);
}

/**
   @brief 弹出分级管控安全等级设置引导提示窗口
 */
void DebListModel::showHierarchicalVerifyWindow()
{
    if (!HierarchicalVerify::instance()->isValid()) {
        return;
    }

    DDialog *dialog = new DDialog();
    // 限制显示宽度
    dialog->setFixedWidth(380);
    dialog->setFocusPolicy(Qt::TabFocus);
    dialog->setModal(true);
    dialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    dialog->setTitle(tr("Unable to install"));
    dialog->setMessage(tr("This package does not have a valid digital signature and has been blocked from installing/running. "
                          "Go to Security Center > Tools > App Security to change the settings."));
    dialog->setIcon(QIcon::fromTheme("di_popwarning"));
    dialog->addButton(QString(tr("Cancel", "button")), false, DDialog::ButtonNormal);
    dialog->addButton(QString(tr("Proceed", "button")), true, DDialog::ButtonRecommend);
    dialog->show();

    // 拷贝自 ddialog.cpp 用于设置默认对话框高度，避免在大字号/高缩放比下显示不全，在 show() 之后调用。
    QLabel *msgLabel = dialog->findChild<QLabel *>("MessageLabel");
    if (msgLabel) {
        auto dialogStyle = dialog->style();
        if (dialogStyle) {
            QSize sz =
                dialogStyle->itemTextRect(msgLabel->fontMetrics(), msgLabel->rect(), Qt::TextWordWrap, false, msgLabel->text())
                    .size();
            msgLabel->setMinimumHeight(qMax(sz.height(), msgLabel->sizeHint().height()));
        }
    }

    QPushButton *btnPorceed = qobject_cast<QPushButton *>(dialog->getButton(1));
    btnPorceed->setFocusPolicy(Qt::TabFocus);
    btnPorceed->setFocus();

    // 窗口退出操作，包括所有可以退出此窗口的操作
    std::function<void(void)> exitOperate = [dialog]() { dialog->deleteLater(); };

    connect(dialog, &DDialog::finished, [dialog](int result) {
        if (QDialog::Accepted == result) {
            HierarchicalVerify::instance()->proceedDefenderSafetyPage();
        }
        dialog->deleteLater();
    });
}

bool DebListModel::checkBlackListApplication()
{
    PackageDependsStatus dependsStat = m_packagesManager->getPackageDependsStatus(m_operatingIndex);
    if (dependsStat.isProhibit()) {
        showProhibitWindow();
        return true;
    }
    return false;
}

DebListModel::~DebListModel()
{
    delete m_packagesManager;
    delete configWindow;
    delete m_procInstallConfig;
}

/**
   @brief 打印待安装的软件包信息，将根据安装、升级、卸载等分类分别打印对应变更的软件包
 */
void DebListModel::printDependsChanges()
{
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        return;
    }

    auto changeList = backend->markedPackages();
    if (changeList.isEmpty()) {
        return;
    }

    static QMap<int, QString> tagTable = {
        { Package::IsManuallyHeld, "Package::IsManuallyHeld" }, { Package::NewInstall, "Package::NewInstall" },
        { Package::ToReInstall, "Package::ToReInstall" },       { Package::ToUpgrade, "Package::ToUpgrade" },
        { Package::ToDowngrade, "Package::ToDowngrade" },       { Package::ToRemove, "Package::ToRemove" }
    };
    QMap<int, QStringList> changeInfo;

    for (const Package *package : changeList) {
        int flags = package->state();
        int status = flags & (Package::IsManuallyHeld | Package::NewInstall | Package::ToReInstall | Package::ToUpgrade |
                              Package::ToDowngrade | Package::ToRemove);
        changeInfo[status] << QString("%1, %2, %3").arg(package->name()).arg(package->version()).arg(package->architecture());
    }

    qInfo() << "Install depends details:";
    for (auto info = changeInfo.begin(); info != changeInfo.end(); info++) {
        qInfo() << tagTable[info.key()] << info.value();
    }
}

Dialog::Dialog() { }

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        emit signalClosed();
}
