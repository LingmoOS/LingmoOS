// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBLISTMODEL_H
#define DEBLISTMODEL_H

#include "manager/packagesmanager.h"
#include "process/Pty.h"
#include "abstract_package_list_model.h"

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>

#include <DSysInfo>
#include <DPushButton>
#include <DDialog>

#include <QAbstractListModel>
#include <QFuture>
#include <QPointer>
#include <QDBusInterface>
#include <QDBusReply>
#include <QProcess>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

class AptConfigMessage;

class Dialog : public DDialog
{
    Q_OBJECT
public:
    explicit Dialog();
    void keyPressEvent(QKeyEvent *event);
signals:
    void signalClosed();
};

class DebListModel : public AbstractPackageListModel
{
    Q_OBJECT

public:
    explicit DebListModel(QObject *parent = nullptr);
    ~DebListModel() override;

    /**
     * @brief isDpkgRunning 判断当前dpkg 是否在运行
     * @return
     */
    static bool isDpkgRunning();
    /**
     * @brief netErrors
     * @return the List of The Error infomations.
     * 无网络安装依赖时，库返回错误为FetechError 偶尔为CommitError
     * 此函数处理库返回CommitError时，网络错误的各种情况，如果错误信息中包含此列表中的信息，则判断为网络原因。
     */
    static const QStringList netErrors();

    /**
     * @brief workerErrorString 根据安装失败的代码显示安装失败的原因。
     * @param errorCode 库或自定义的安装失败代码
     * @param errorInfo 库返回的或自定义的安装错误的原因
     * @return 要显示的安装失败的原因
     */
    static const QString workerErrorString(const int errorCode, const QString &errorInfo);

    /**
     * @brief reset
     * 重置包的工作状态、安装状态
     * 重置当前处理的下标装啊提
     * 清空安装错误的缓存
     * 重置packageManage的状态
     */
    void reset() override;

    /**
     * @brief resetFilestatus
     * 重置包的操作状态
     * 清空安装错误原因的缓存
     */
    void resetFileStatus();

    void resetInstallStatus() override;

    /**
     * @brief isReady 查看后端初始化的状态
     * @return 后端是否准备就绪
     */
    bool isReady() const;

    /**
     * @brief preparedPackages 获取当前已经添加的包的列表
     * @return 添加的包的列表
     */
    const QList<QString> preparedPackages() const;

    /**
     * @brief first 获取添加的第一个包
     * @return 第一个包的index
     */
    QModelIndex first() const;

    /**
     * @brief rowCount 当前行的数量
     * @param parent
     * @return 当前包的数量
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief data  获取某一项的数据
     * @param index 下表
     * @param role  数据的角色
     * @return
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief isDevelopMode 是否是开发者模式
     * @return
     */
    bool isDevelopMode();

public:
    /**
     * @brief initPrepareStatus  初始化所有包的状态为Prepare
     */
    void initPrepareStatus();

public:
    /**
     * @brief getInstallFileSize 获取要安装的包的数量
     * @return 包的数量
     */
    int getInstallFileSize();

public:
    /**
     * @brief removePackage 删除某一个包
     * @param idx 要删除的包的index
     */
    void removePackage(const int idx) override;

    /**
     * @brief checkPackageDigitalSignature 查找指定包安装状态
     * @param package_path 路径
     */
    int checkInstallStatus(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包依赖
     * @param package_path 路径
     */
    int checkDependsStatus(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包数字签名
     * @param package_path 路径
     */
    int checkDigitalSignature(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包信息
     * @param package_path 路径
     */
    QStringList getPackageInfo(const QString &package_path);
    /**
     * @brief getInstallErrorMessage 安装错误信息
     */
    QString getInstallErrorMessage();
    /**
     * @brief checkPackageValid 查找指定包信息
     * @param package_path 路径
     */
    QString checkPackageValid(const QString &package_path);

signals:
    /**
     * @brief signalPackageOperationChanged 包操作状态改变
     * @param index 包的index
     * @param status    修改的状态
     */
    void signalPackageOperationChanged(const QModelIndex &index, int status) const;

    /**
     * @brief signalPackageDependsChanged     包的依赖状态改变
     * @param index     包的index
     * @param status    修改后的状态
     */
    void signalPackageDependsChanged(const QModelIndex &index, int status) const;

public slots:

    /**
     * @brief slotSetCurrentIndex   设置当前操作的index
     * @param idx       修改后的index
     */
    void slotSetCurrentIndex(const QModelIndex &idx);

    /**
     * @brief slotInstallPackages 开始安装所有的包
     */
    void slotInstallPackages() override;

    /**
     * @brief slotUninstallPackage     卸载某一个包
     * @param index   包的index
     */
    void slotUninstallPackage(const int index);

    /**
     * @brief slotAppendPackage 添加包
     * @param package 添加的包的路径
     */
    void slotAppendPackage(const QStringList &packages);

    /**
     * @brief slotTransactionErrorOccurred 安装过程中出现错误
     */
    void slotTransactionErrorOccurred();

    /**
     * @brief slotTransactionStatusChanged 安装状态出现改变
     * @param TransactionStatus
     */
    void slotTransactionStatusChanged(QApt::TransactionStatus TransactionStatus);

    /**
     * @brief slotDealDependResult 处理依赖安装的过程
     * @param authType      授权类型
     * @param dependIndex   依赖包的index（因为这个包的依赖不被满足）
     * @param dependName    发生依赖安装失败的依赖名称
     */
    void slotDealDependResult(int authType, int dependIndex, const QString &dependName);

public slots:
    /**
     * @brief slotConfigReadOutput 处理配置包的输出并显示
     */
    void slotConfigReadOutput(const char *buffer, int length, bool isCommandExec);

    /**
     * @brief slotConfigInstallFinish 配置结束
     * @param flag 配置安装的结果
     */
    void slotConfigInstallFinish(int flag);

    /**
     * @brief slotConfigInputWrite 配置的输入数据处理
     * @param str 输入的数据（一般是输入的选项）
     */
    void slotConfigInputWrite(const QString &str);

    /**
     * @brief slotCheckInstallStatus 根据命令返回的消息判断安装状态
     * @param str  命令返回的安装信息
     * 如果命令返回的信息是Cannot run program deepin-deb-installer-dependsInstall: No such file or directory
     * 意味着当前/usr/bin下没有deepin-deb-installer-dependsInstall命令，此版本有问题，需要重新安装deepin-deb-installer-dependsInstall命令
     */
    void slotCheckInstallStatus(const QString &str);

private slots:

    /**
     * @brief slotUpWrongStatusRow 安装完成后对安装失败的包上滚 其中包括各种状态的变更
     */
    void slotUpWrongStatusRow();

    /**
     * @brief slotTransactionOutput
     * 修改当前包的工作状态
     */
    void slotTransactionOutput();

    /**
     * @brief slotTransactionFinished
     * 当前包安装结束
     */
    void slotTransactionFinished();

    /**
     * @brief  slotDependsInstallTransactionFinished();

     * 当前包依赖安装结束
     * 依赖安装成功，安装当前包
     * 依赖安装失败，记录失败原因，安装下一个包
     */
    void slotDependsInstallTransactionFinished();

    /**
     * @brief slotUninstallFinished
     * 卸载结束槽函数
     * 切换工作状态
     * 切换操作状态
     * 发送卸载结束信号
     */
    void slotUninstallFinished();

    /**
     * @brief slotNoDigitalSignature 无数字签名
     */
    void slotNoDigitalSignature();

    /**
     * @brief slotDigitalSignatureError 数字签名校验失败
     */
    void slotDigitalSignatureError();

    /**
     * @brief showDevelopModeWindow 打开控制中心通用界面
     */
    void slotShowDevelopModeWindow();

    /**
     * @brief slotShowProhibitWindow 应用在域管黑名单中，无法安装
     */
    void slotShowProhibitWindow();

private:
    /**
     * @brief setEndEnable  授权框取消，按钮启用
     */
    void setEndEnable();

    /**
     * @brief checkBoxStatus 检查授权框的弹出状态，判断当前按钮是否需要被禁用
     */
    void checkBoxStatus();

    /**
     * @brief bumpInstallIndex  安装包整体的流程控制
     * 安装完成上一个之后 index 增加
     * 全部安装完成之后 发送安装完成信号
     */
    void bumpInstallIndex();

    /**
     * @brief installNextDeb
     *  判断操作系统的状态，判断签名的状态
     *  检查当前包是否需要配置
     *  安装包
     */
    void installNextDeb();

    /**
     * @brief installDebs 安装单个包。
     */
    void installDebs();

    /**
     * @brief refreshOperatingPackageStatus 刷新当前操作的包的操作状态
     * @param oprationStatus  要修改的操作状态
     */
    void refreshOperatingPackageStatus(PackageOperationStatus oprationStatus);

    /**
     * @brief packageFailedReason  获取包安装失败的原因
     * @param idx       包的下标
     * @return  包失败原因的描述
     */
    QString packageFailedReason(const int idx) const;

    /**
     * @brief initRowStatus 初始化每一个项的 操作状态
     */
    void initRowStatus();

private:
    /**
     * @brief checkDigitalSignature  检查数字签名
     * @return  检查当前包是否有数字签名
     */
    bool checkDigitalSignature();

    /**
     * @brief showNoDigitalErrWindow 弹出无数字签名的错误弹窗
     */
    void showNoDigitalErrWindow();

    /**
     * @brief showNoDigitalErrWindow 弹出数字签名校验错误的错误弹窗
     * @param recordError 是否记录错误信息，引入分级验签后，允许只提示验签错误信息，
     *  不涉及状态切换及更新记录
     */
    void showDigitalErrWindow(bool recordError = true);

    /**
     * @brief showDevelopDigitalErrWindow 开发者模式下弹出数字签名无效的弹窗
     */
    void showDevelopDigitalErrWindow(ErrorCode code);

    /**
     * @brief showNoDigitalErrWindowInDdimProcess DDIM流程下的签名错误弹窗
     */
    void showNoDigitalErrWindowInDdimProcess(void (DebListModel::*failedFunction)());

    /**
     * @brief showProhibitWindow 弹出数字签名校验错误的错误弹窗
     */
    void showProhibitWindow();

    /**
     * @brief showHierarchicalVerifyWindow 弹出分级管控安全等级设置引导提示窗口
     */
    void showHierarchicalVerifyWindow();

    /**
     * @brief 检查当前将要安装的包是否在黑名单中。
     *
     * @return true 当前要安装的包在黑名单中
     * @return false 当前要安装的包不在黑名单中
     */
    bool checkBlackListApplication();

    /**
     * @brief 数字签名校验失败 弹窗处理的槽函数
     *
     * @param errorCode 错误原因代码
     */
    void digitalVerifyFailed(ErrorCode errorCode);

private:
    /**
     * @brief checkTemplate 检查是否需要配置
     * @param debPath   包的路径
     * @return  是否需要配置
     */
    bool checkTemplate(const QString &debPath);

    /**
     * @brief getDebian    获取包的DEBIAN文件
     * @param debPath      包的路径
     */
    void getDebian(const QString &debPath);

    /**
     * @brief mkdir     创建临时目录 ，存放DEBIAN文件
     * @return  是否创建成功
     */
    bool mkdir();

    /**
     * @brief rmdir     删除临时目录
     *
     */
    void rmdir();

    /**
     * @brief enableTitleBarFocus
     * 启用TitleBar焦点切换策略
     */
    void enableTitleBarFocus();

    /**
     * @brief Get the Package Md5 object
     *
     * @param packagesMD5 所有包的md5的列表
     */
    void getPackageMd5(const QList<QByteArray> &packagesMD5);

    //// 文件移动、删除、修改检查
private:
    /**
     * @brief recheckPackagePath 重新检查文件路径
     * @param packagePath 当前需要检查的文件路径
     * @return 检查的结果
     *         true : 文件存在
     *         false: 文件不存在
     */
    bool recheckPackagePath(const QString &packagePath) const;

private:
    /**
     * @brief initConnections 初始化信号与槽的链接
     */
    void initConnections();

    /**
     * @brief initInstallConnecions 链接安装过程中的信号与槽
     */
    void initInstallConnections();

    /**
     * @brief initAppendConnection 链接添加过程中的信号与槽
     */
    void initAppendConnection();

    /**
     * @brief initRefreshPageConnecions 链接页面刷新的信号与槽
     */
    void initRefreshPageConnecions();

    /**
       @brief printDependsChanges 打印安装的依赖包变更
     */
    void printDependsChanges();

private:
    // 当前正在操作的index
    int m_operatingIndex = 0;

    // 当前正在操作的状态的index
    int m_operatingStatusIndex = 0;

    // 当前正在处理的包的md5
    QByteArray m_operatingPackageMd5 = nullptr;

    // 当前的index
    QModelIndex m_currentIdx;

    // 后端类
    PackagesManager *m_packagesManager = nullptr;

    // 当前正在运行的Trans
    QPointer<QApt::Transaction> m_currentTransaction;

    // 所有包的操作状态Map
    QMap<QByteArray, int> m_packageOperateStatus = {};

    // FailCode 错误代码 ，trans返回的错误代码
    QMap<QByteArray, int> m_packageFailCode = {};

    // FailReason , trans返回的详细错误信息
    QMap<QByteArray, QString> m_packageFailReason = {};

    // 配置安装进程
    Konsole::Pty *m_procInstallConfig = {};

    // 配置的临时目录
    const QString tempPath = "/tmp/DEBIAN";

    QString m_brokenDepend = "";

    // 开发者模式的标志变量
    // 部分系统版本无需签名验证，默认开发者模式
    bool m_isDevelopMode = true;
    bool m_isDigitalVerify = false;

    QList<QByteArray> m_packageMd5 = {};

    AptConfigMessage *configWindow = nullptr;

    // 当前安装是否存在分级管控签名验证失败
    bool m_hierarchicalVerifyError = false;
};

#endif  // DEBLISTMODEL_H
