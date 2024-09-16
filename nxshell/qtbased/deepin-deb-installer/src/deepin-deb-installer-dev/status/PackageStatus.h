// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGEDEPENDSSTATUS_H
#define PACKAGEDEPENDSSTATUS_H
#include "result.h"

#include <QObject>
#include <QFuture>

#include <QApt/Backend>
/**
 * @brief The PackageDependsStatus enum
 * 当前包的依赖状态
 */
enum DependsStatus {
    DependsUnknown,
    DependsOk,          // 依赖满足
    DependsAvailable,   // 依赖可用但是需要下载
    DependsBreak,       // 依赖不满足
    DependsAuthCancel,  // 依赖授权失败（wine依赖）
    ArchBreak,  // 架构不满足（此前架构不满足在前端验证，此后会优化到后端）//2020-11-19 暂时未优化
};

/**
 * @brief The PackageInstallStatus enum
 * 包的安装状态
 */
enum InstallStatus {
    InstallStatusUnknown,
    NotInstalled,             // 当前包没有被安装
    InstalledSameVersion,     // 当前已经安装过相同的版本
    InstalledEarlierVersion,  // 当前已经安装过较早的版本
    InstalledLaterVersion,    // 当前已经安装过更新的版本
};

QApt::Backend *init_backend();
typedef Result<QString> ConflictResult;

class PackageStatus
{
public:
    friend class PackagesManager;

    PackageStatus();
    PackageStatus(DependsStatus status, const QString &package);

    ~PackageStatus();

    /**
     * @brief operator = 重写=操作符
     * @param other 要赋值的依赖的状态
     * @return 赋值后的依赖的状态
     */
    PackageStatus &operator=(const PackageStatus &other);

    /**
     * @brief max   状态的比较
     * @param other
     * @return
     */
    PackageStatus max(const PackageStatus &other);
    PackageStatus maxEq(const PackageStatus &other);
    PackageStatus min(const PackageStatus &other);
    PackageStatus minEq(const PackageStatus &other);

    bool isBreak() const;
    bool isAuthCancel() const;
    bool isAvailable() const;

    DependsStatus getPackageDependsStatus(const QString &packagePath);

    /**
     * @brief packageInstallStatus 获取指定index的包的安装状态
     * @param index 指定的index
     * @return 包的安装状态s
     */
    InstallStatus getPackageInstallStatus(const QString &packagePath);

    /**
     * @brief packageAvailableDepends 获取指定包的可用的依赖
     * @param index 下标
     * @return  指定包所有的需要下载的依赖
     */
    const QStringList getPackageAvailableDepends(const QString &packagePath);

    /**
     * @brief packageReverseDependsList 获取依赖于此包的所有应用名称
     * @param packageName 需要检查的包的包名
     * @param sysArch 包的架构
     * @return  依赖于此包的应用列表
     */
    const QStringList getPackageReverseDependsList(const QString &packageName, const QString &sysArch);

private:
    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation = QString());

    QString resolvMultiArchAnnotation(const QString &annotation,
                                      const QString &debArch,
                                      const int multiArchType = QApt::InvalidMultiArchType);

    /**
     * @brief isArchMatches 判断包的架构是否符合系统要求
     * @param sysArch       系统架构
     * @param packageArch   包的架构
     * @param multiArchType 系统多架构类型
     * @return 是否符合多架构要求
     */
    bool isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType);

    /**
     * @brief isArchError 判断指定的包是否符合架构要求
     * @param packagePath   指定的包
     * @return  符合架构要求的结果
     *
     * false: 符合架构要求
     * true: 不符合当前系统架构的要求
     */
    bool isArchError(const QString &packagePath);

    /**
     * @brief dependencyVersionMatch
     * @param result
     * @param relation
     * @return
     */
    bool dependencyVersionMatch(const int result, const QApt::RelationType relation);

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
    const ConflictResult isConflictSatisfy(const QString &arch,
                                           const QList<QApt::DependencyItem> &conflicts,
                                           const QList<QApt::DependencyItem> &replaces = {});

    /**
     * @brief checkDependsPackageStatus 检查依赖包的状态
     * @param choosed_set   被选择安装或卸载的包的集合
     * @param architecture  包的架构
     * @param depends       包的依赖列表
     * @return
     */
    DependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,
                                            const QString &architecture,
                                            const QList<QApt::DependencyItem> &depends);
    DependsStatus
    checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture, const QApt::DependencyItem &candicate);
    DependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set,
                                            const QString &architecture,
                                            const QApt::DependencyInfo &dependencyInfo);

private:
    /**
     * @brief packageCandidateChoose   查找包的依赖候选
     * @param choosed_set   包的依赖候选的集合
     * @param debArch       包的架构
     * @param dependsList   依赖列表
     */
    void
    packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch, const QList<QApt::DependencyItem> &dependsList);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch, const QApt::DependencyItem &candidateItem);

private:
    // fix bug:https://pms.uniontech.com/zentao/bug-view-37220.html
    // 卸载deepin-wine-plugin-virture 时无法卸载deepin-wine-helper. Temporary solution：Special treatment for these package
    QMap<QString, QString> specialPackage();

public:
    DependsStatus m_status = DependsUnknown;
    QString m_package = "";

private:
    QFuture<QApt::Backend *> m_backendFuture;
};
#endif  // PACKAGEDEPENDSSTATUS_H
