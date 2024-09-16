// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESMANAGER_H
#define PACKAGESMANAGER_H

#include "utils/package_defines.h"
#include "utils/result.h"
#include "model/dependgraph.h"

#include <QApt/Backend>
#include <QApt/DebFile>

#include <QThread>
#include <QProcess>
#include <QFuture>
#include <QObject>

using namespace QApt;

#define BLACKFILE "/usr/share/udcp/appblacklist.txt"

typedef Result<QString> ConflictResult;

class PackageDependsStatus;
class DebListModel;
class DealDependThread;
class AddPackageThread;

/**
 * @brief init_backend 初始化后端
 * @return 初始化完成的后端指针
 */
Backend *init_backend();

class PackagesManager : public QObject
{
    Q_OBJECT

    friend class DebListModel;

public:
    explicit PackagesManager(QObject *parent = nullptr);
    ~PackagesManager();

    /**
     * @brief isArchMatches 判断包的架构是否符合系统要求
     * @param sysArch       系统架构
     * @param packageArch   包的架构
     * @param multiArchType 系统多架构类型
     * @return 是否符合多架构要求
     */
    static bool isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType);

    /**
     * @brief resolvMultiArchAnnotation 处理多架构问题
     * @param annotation  当前control文件中的附加信息
     * @param debArch   deb包的架构
     * @param multiArchType 多架构的类型
     * @return
     */
    static QString
    resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, const int multiArchType = InvalidMultiArchType);

    /**
     * @brief dependencyVersionMatch 判断当前依赖版本是否匹配
     * @param result 当前依赖版本
     * @param relation 依赖版本关系的类型
     * @return
     */
    static bool dependencyVersionMatch(const int result, const RelationType relation);

    /**
     * @brief searchPackageInstallInfo 查找指定包安装状态
     * @param package_path 路径
     */
    int checkInstallStatus(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包依赖
     * @param package_path 包名字
     */
    PackageDependsStatus checkDependsStatus(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包信息
     * @param package_path 包名字
     */
    QStringList getPackageInfo(const QString &package_path);
    /**
     * @brief checkPackageValid 检查包有效性
     * @param package_path 包名字
     */
    QString checkPackageValid(const QStringList &package_path);
public slots:
    /**
     * @brief DealDependResult 处理wine依赖下载结果的槽函数
     * @param iAuthRes  下载结果
     * @param iIndex    wine应用的下标
     * @param dependName    wine依赖错误的依赖名称
     */
    void slotDealDependResult(int iAuthRes, int iIndex, const QString &dependName);

    //// 依赖下载相关信号
signals:
    /**
     * @brief DependResult 处理wine依赖下载结果
     */
    void signalDependResult(int, int, const QString &);

    /**
     * @brief enableCloseButton 设置关闭按钮是否可用的信号
     */
    void signalEnableCloseButton(bool);

    ////添加包相关信号
signals:
    // Manange package insert failed reason.
    void signalAppendFailMessage(Pkg::AppendFailReason reason);

    /**
     * @brief appendStart 批量安装开始添加包的信号
     */
    void signalAppendStart();

    /**
     * @brief appendFinished 批量安装添加包结束的信号
     * @param packageMd5List 添加后的md5列表
     */
    void signalAppendFinished(const QList<QByteArray> &packageMd5List);

    /**
     * @brief packageMd5Changed 添加完成之后更新MD5的列表
     * @param packageMd5List 当前的MD5列表
     */
    void signalPackageMd5Changed(const QList<QByteArray> &packageMd5List);

    //// 界面刷新相关信号
signals:
    /**
       @brief signalPacakgeCountChanged notify pacakge append or remove.
     */
    void signalPackageCountChanged(int count);

    //// 后端状态相关函数
public:
    /**
     * @brief isBackendReady 判断安装程序后端是否加载完成
     * @return 安装程序后端加载的结果
     *
     * true: 加载完成
     * false: 未加载完成
     */
    bool isBackendReady();

    //// 包状态相关函数
public:
    /**
     * @brief package   获取指定下标的包的路径
     * @param index  下标
     * @return  包的路径
     */
    QString package(const int index) const;

    /**
     * @brief isArchError 判断指定下标的包是否符合架构要求
     * @param idx   指定的下标
     * @return  符合架构要求的结果
     *
     * false: 符合架构要求
     * true: 不符合当前系统架构的要求
     */
    bool isArchError(const int idx);
    bool isArchErrorQstring(const QString &package_name);

    /**
     * @brief packageInstallStatus 获取指定index的包的安装状态
     * @param index 指定的index
     * @return 包的安装状态
     */
    int packageInstallStatus(const int index);

    /**
     * @brief packageInstalledVersion 获取指定下标的包的安装版本
     * @param index 下标
     * @return 包的版本
     */
    const QString packageInstalledVersion(const int index);

    /**
     * @brief packageConflictStat 获取指定包的冲突状态
     * @param index 下标
     * @return 包的冲突状态
     */
    const ConflictResult packageConflictStat(const int index);

    /**
     * @brief packageAvailableDepends 获取指定包的可用的依赖
     * @param index 下标
     * @return  指定包所有的需要下载的依赖
     */
    const QStringList packageAvailableDepends(const int index);

    /**
     * @brief getPackageDependsStatus 获取指定包的依赖的状态
     * @param index 下标
     * @return 包的依赖状态
     */
    PackageDependsStatus getPackageDependsStatus(const int index);

    /**
     * @brief getPackageOrDepends 解析或依赖关系
     * @param package 包的路径或者包名
     * @param arch  架构
     * @param flag  标记是依赖还是安装包
     */
    void getPackageOrDepends(const QString &package, const QString &arch, bool flag);

    /**
     * @brief getPackageMd5 获取包的md5值
     * @param index 下标
     * @return 包的MD5值
     */
    QByteArray getPackageMd5(const int index);

    /**
     * @brief packageReverseDependsList 获取依赖于此包的所有应用名称
     * @param packageName 需要检查的包的包名
     * @param sysArch 包的架构
     * @return  依赖于此包的应用列表
     */
    const QStringList packageReverseDependsList(const QString &packageName, const QString &sysArch);

    //// 添加删除相关函数
public:
    /**
     * @brief appendPackage 添加包到程序中
     * @param debPackage    要添加的包的列表
     */
    void appendPackage(QStringList debPackage);

    /**
     * @brief removePackage 删除指定的包
     * @param index 要删除的包的下标
     * @param listDependInstallMark 因wine依赖被标记的下标
     */
    void removePackage(const int index);

    //// 重置状态相关函数
public:
    /**
     * @brief reset 重置 PackageManager的状态
     */
    void reset();

    /**
     * @brief resetPackageDependsStatus 重置指定安装包的状态
     * @param index 指定包的下标
     */
    void resetPackageDependsStatus(const int index);

    //// 依赖查找 获取等相关函数
private:
    /**
     * @brief checkDependsPackageStatus 检查依赖包的状态
     * @param choosed_set   被选择安装或卸载的包的集合
     * @param architecture  包的架构
     * @param depends       包的依赖列表
     * @return
     */
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                         const QString &architecture,
                                                         const QList<QApt::DependencyItem> &depends);
    const PackageDependsStatus
    checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture, const QApt::DependencyItem &candicate);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                         const QString &architecture,
                                                         const QApt::DependencyInfo &dependencyInfo,
                                                         const QString &providesName = QString::null);
    /**
     * @brief packageCandidateChoose   查找包的依赖候选
     * @param choosed_set   包的依赖候选的集合
     * @param debArch       包的架构
     * @param dependsList   依赖列表
       @param levelInfo     提供用于打印的软件包查找层级依赖信息
     */
    void packageCandidateChoose(QSet<QString> &choosed_set,
                                const QString &debArch,
                                const QList<QApt::DependencyItem> &dependsList,
                                const QString &levelInfo);
    void packageCandidateChoose(QSet<QString> &choosed_set,
                                const QString &debArch,
                                const QApt::DependencyItem &candidateItem,
                                const QString &levelInfo);

    /**
     * @brief isInstalledConflict 是否存在下载冲突
     * @param packageName         包名
     * @param packageVersion      包的版本
     * @param packageArch         包的架构
     * @return
     */
    const ConflictResult
    isInstalledConflict(const QString &packageName, const QString &packageVersion, const QString &packageArch);
    /**
     * @brief isConflictSatisfy 是否冲突满足
     * @param arch              架构
     * @param package           包名
     * @return     冲突的结果
     */
    const ConflictResult isConflictSatisfy(const QString &arch, QApt::Package *package);
    const ConflictResult isConflictSatisfy(const QString &arch, const QList<QApt::DependencyItem> &conflicts);

    // 带replaces的检查，如果判定待安装包可以替换冲突包，则认为不构成冲突
    const ConflictResult
    isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts, const QList<DependencyItem> &replaces);

    //// 依赖查找 获取查找包是否为消极的反向依赖
private:
    /**
     * @brief isNegativeReverseDepend 判断软件包 `reverseDepend` 是否是 `packageName` 的消极依赖包，
     *      例如 冲突(Conflict)/替换(Replace)/破坏(Breaks) 这些包在卸载时将被跳过，
     *      在 依赖(Depends) 字段设置包将不会被视为消极包。
     * @param packageName   当前处理的软件包
     * @param reverseDepend `packageName` 的反向依赖包
     * @return `reverseDepend` 是否为 `packageName` 的消极依赖
     */
    bool isNegativeReverseDepend(const QString &packageName, const QApt::Package *reverseDepend);

private:
    /**
     * @brief packageWithArch 从指定的架构上打包
     * @param packageName   包名
     * @param sysArch       系统架构
     * @param annotation    注解
     * @return  package指针
     */
    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation = QString());

    /**
     * @brief checkPackageArchValid 检测通过包名 \a packageName 获取的软件包 \a package 架构
     *      是否支持建议架构 \a suggestArch
     * @param package       软件包
     * @param packageName   软件包名称
     * @param resloveArch   解析建议的软件包名称
     * @return 软件包 \a package 支持 \a suggestArch 架构
     */
    bool checkPackageArchValid(const QApt::Package *package, const QString &packageName, const QString &suggestArch);

private:
    // 卸载deepin-wine-plugin-virture 时无法卸载deepin-wine-helper. Temporary solution：Special treatment for these package
    QMap<QString, QString> specialPackage();

private:
    /**
     * @brief SymbolicLink 为路径中存在空格的包创建临时文件夹以及软链接
     * @param previousName 存在空格的路径ruanji
     * @param packageName  当前包的包名
     * @return 创建成功后软链接的全路径
     */
    QString SymbolicLink(const QString &previousName, const QString &packageName);

    /**
     * @brief link          创建软链接
     * @param linkPath      原路径
     * @param packageName   包名
     * @return  创建软链接之后的路径
     */
    QString link(const QString &linkPath, const QString &packageName);

    /**
     * @brief mkTempDir 创建存放软链接的临时路径
     * @return 是否创建成功
     */
    bool mkTempDir();

    // 使用软连接方式解决文件路径中存在空格的问题。
    /**
     * @brief rmTempDir 删除存放软链接的临时目录
     * @return 删除临时目录的结果
     */
    bool rmTempDir();

private:
    /**
     * @brief addPackage   添加包的处理槽函数
     * @param validPkgCount 有效包的数量
     * @param packagePath   此次添加包的路径
     * @param packageMd5Sum 此次添加的包的md5值
     */
    void addPackage(int validPkgCount, const QString &packagePath, const QByteArray &packageMd5Sum);

    /**
     * @brief getAllDepends 获取安装包所有依赖
     * @param depends       依赖列表
     * @param architecture  安装包架构
     * @return
     */
    QList<QString> getAllDepends(const QList<DependencyItem> &depends, const QString &architecture);

    /**
     * @brief getAllDepends 获取依赖包所有依赖
     * @param packageName   依赖包包名
     * @param architecture  架构
     * @return
     */
    QList<QString> getAllDepends(const QString &packageName, const QString &architecture);

    /**
     * @brief refreshPage 刷新当前的页面
     * @param pkgCount  需要添加的包的数量
     */
    void refreshPage(int pkgCount);

    /**
     * @brief appendNoThread 不通过线程，直接添加包到应用中
     * @param packages 要添加的包
     * @param allPackageSize 要添加的包的数量
     */
    void appendNoThread(const QStringList &packages, int allPackageSize);

    /**
     * @brief checkInvalid 检查有效文件的数量
     */
    void checkInvalid(const QStringList &packages);

    /**
     * @brief dealInvalidPackage 处理无效的安装包
     * @param packagePath 包的路径
     * @return 包的有效性
     *   true   : 文件能打开
     *   fasle  : 文件不在本地或无权限
     */
    bool dealInvalidPackage(const QString &packagePath);

    /**
     * @brief dealPackagePath 处理包的路径
     * @param packagePath 包的路径
     * @return 经过处理后的包的路径
     * 处理两种情况
     *      1： 相对路径             --------> 转化为绝对路径
     *      2： 包的路径中存在空格     --------> 使用软链接，链接到/tmp下
     */
    QString dealPackagePath(const QString &packagePath);

    Pkg::DependsPair getPackageDependsDetail(const int index);

private slots:
    /**
     * @brief slotAppendPackageFinished 添加包结束后，如果此时需要下载wine依赖，则直接开始下载
     */
    void slotAppendPackageFinished();

private:
    /**
     * @brief 判断当前应用是否为黑名单应用
     *
     * @return true 是黑名单应用
     * @return false 不是黑名单应用
     */
    bool isBlackApplication(const QString &applicationName);

    /**
     * @brief 获取当前黑名单应用列表
     *
     */
    void getBlackApplications();

    /**
       @brief 过滤已安装或无需更新的Wine软件包
     */
    void filterNeedInstallWinePackage(QStringList &dependList,
                                      const DebFile &debFile,
                                      const QHash<QString, DependencyInfo> &dependInfoMap);

private:
    QMap<QByteArray, int> m_errorIndex;  // wine依赖错误的包的下标 QMap<MD5, DebListModel::DependsAuthStatus>

    // 存放包路径的列表
    QList<QString> m_preparedPackages = {};

    // 存放包MD5的集合
    QSet<QByteArray> m_appendedPackagesMd5 = {};

    QMap<QString, QByteArray> m_allPackages;  // 存放有效包路径及md5，避免二次获取消耗时间

    // 包MD5与下标绑定的list
    QList<QByteArray> m_packageMd5 = {};

    /**
     * @brief m_packageMd5DependsStatus 包的依赖状态的Map
     * QByteArray 包的下标
     * PackageDependsStatus 依赖的状态
     *
     * 此前将下标与安装状态绑定
     * 此时使用MD5与依赖状态绑定
     * 修改原因：
     * 1.修改添加方式从添加到插入到最前方
     * 2.使用之前的方式会导致所有包的依赖状态错乱
     */
    QMap<QByteArray, PackageDependsStatus> m_packageMd5DependsStatus;

    /**
     * @brief m_packageInstallStatus 包安装状态的Map
     * int: 包的下标
     * int: 包的安装状态
     * 此前的安装状态将下标与安装状态绑定
     * 此时使用MD5与安装状态绑定
     * 修改原因：
     * 1.修改添加方式从添加到插入到最前方
     * 2.使用之前的方式会导致所有包的安装状态都是第一个包的安装状态
     */
    QMap<QByteArray, int> m_packageInstallStatus = {};

    QByteArray m_currentPkgMd5;  // 当前包的md5

    /**
     * @brief m_dependsPackages  包依赖关系的map
     * QPair<QList<dependInfo>, QList<dependInfo>> 仓库可获取依赖与仓库不可获取依赖
     * 与md5进行绑定
     */
    QMap<QByteArray, Pkg::DependsPair> m_dependsPackages;
    Pkg::DependInfo m_dinfo;  // 依赖包的包名及版本

    /**
       @brief m_loopErrorDeepends 循环判断依赖时缓存非 Ok 的前置包状态
        用于对 OR 或依赖及 Provides 虚包依赖在循环中依赖中返回前置已检测的包状态，
        而不是直接返回 Ok .
     */
    QHash<QString, int> m_loopErrorDeepends;

    // wine应用处理的下标
    int m_DealDependIndex = -1;

    // 下载依赖的线程
    DealDependThread *m_installWineThread = nullptr;

    /**
     * @brief m_dependInstallMark wine依赖下标的标记
     * 将依赖下载的标记修改为md5sum  与包绑定 而非下标
     */
    QList<QByteArray> m_dependInstallMark = {};

    QPair<QList<Pkg::DependInfo>, QList<Pkg::DependInfo>> m_pair;  // 存储available及broken依赖

    QList<QString> m_allDependsList;  // 存储当前添加的包的所有依赖

private:
    const QString m_tempLinkDir = "/tmp/LinkTemp/";  // 软链接临时路径

private:
    AddPackageThread *m_pAddPackageThread = nullptr;  // 添加包的线程

    bool installWineDepends = false;

    bool isDependsExists = false;

    int m_validPackageCount = 0;

    qint64 dependsStatusTotalTime = 0;

    QString m_brokenDepend = "";

    QString m_currentPkgName = "";

    QStringList m_blackApplicationList = {};  // 域管黑名单

    QList<QVector<QString>> m_orDepends;  // 存储或依赖关系

    QList<QVector<QString>> m_unCheckedOrDepends;  // 存储还未检测的或依赖关系

    QMap<QString, PackageDependsStatus> m_checkedOrDependsStatus;  // 存储检测完成的或依赖包及其依赖状态

    QMap<QString, DependencyInfo> m_dependsInfo;  // 所有依赖的信息

    DependGraph m_dependGraph;  // 依赖关系图计算器
};

#endif  // PACKAGESMANAGER_H
