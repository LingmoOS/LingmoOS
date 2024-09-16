// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packagesmanager.h"
#include "DealDependThread.h"
#include "PackageDependsStatus.h"
#include "AddPackageThread.h"
#include "utils/utils.h"
#include "model/deblistmodel.h"
#include "model/dependgraph.h"
#include "model/packageanalyzer.h"
#include "singleInstallerApplication.h"

#include <DRecentManager>

#include <QPair>
#include <QSet>
#include <QDir>
#include <QtConcurrent>

#include <fstream>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace QApt;

// 特殊的 Wine 软件包标签
static const QString g_Tagi386 = "i386";
static const QString g_TagDeepinWine = "deepin-wine";
static const QString g_DeepinWineHelper = "deepin-wine-helper";

/**
 * @brief 检测传入软件包架构是否支持多架构
 */
bool archMultiSupport(const QString &arch)
{
    return "native" == arch || "all" == arch || "any" == arch;
}

/**
 * @brief isArchMatches 判断包的架构是否符合系统要求
 * @param sysArch       系统架构
 * @param packageArch   包的架构
 * @param multiArchType 系统多架构类型
 * @return 是否符合多架构要求
 */
bool PackagesManager::isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType)
{
    Q_UNUSED(multiArchType);

    if (sysArch.startsWith(':'))
        sysArch.remove(0, 1);
    if ("all" == sysArch || "any" == sysArch)
        return true;

    // bug119619  安装包时，仓库中的依赖fcitx是全架构，导致系统架构与包架构不匹配
    // 增加依赖包为全架构的判断
    if ("all" == packageArch || "any" == packageArch)
        return true;

    return sysArch == packageArch;
}

QString PackagesManager::resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, const int multiArchType)
{
    if ("native" == annotation || "any" == annotation)
        return QString();
    if ("all" == annotation)
        return QString();
    if (multiArchType == MultiArchForeign)
        return QString();

    QString arch;
    if (annotation.isEmpty())
        arch = debArch;
    else
        arch = annotation;

    if (!arch.startsWith(':') && !arch.isEmpty())
        return arch.prepend(':');
    else
        return arch;
}

bool PackagesManager::dependencyVersionMatch(const int result, const RelationType relation)
{
    switch (relation) {
        case LessOrEqual:
            return result <= 0;
        case GreaterOrEqual:
            return result >= 0;
        case LessThan:
            return result < 0;
        case GreaterThan:
            return result > 0;
        case Equals:
            return result == 0;
        case NotEqual:
            return result != 0;
        default:
            return true;
    }
}

int PackagesManager::checkInstallStatus(const QString &package_path)
{
    DebFile debFile(package_path);
    if (!debFile.isValid())
        return DebListModel::NotInstalled;
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return DebListModel::NotInstalled;
    }
    Package *package = packageWithArch(packageName, packageArch);

    if (!package)
        return DebListModel::NotInstalled;

    const QString installedVersion = package->installedVersion();
    package = nullptr;
    if (installedVersion.isEmpty())
        return DebListModel::NotInstalled;

    const QString packageVersion = debFile.version();
    const int result = Package::compareVersion(packageVersion, installedVersion);

    int ret;
    if (result == 0)
        ret = DebListModel::InstalledSameVersion;
    else if (result < 0)
        ret = DebListModel::InstalledLaterVersion;
    else
        ret = DebListModel::InstalledEarlierVersion;

    return ret;
}

PackageDependsStatus PackagesManager::checkDependsStatus(const QString &package_path)
{
    DebFile debFile(package_path);
    if (!debFile.isValid())
        return PackageDependsStatus::_break("");
    m_currentPkgName = debFile.packageName();
    m_orDepends.clear();
    m_checkedOrDependsStatus.clear();
    m_unCheckedOrDepends.clear();
    m_dependsInfo.clear();
    m_loopErrorDeepends.clear();
    // 用debFile.packageName()无法打开deb文件，故替换成debFile.filePath()
    // 更新m_dependsInfo
    getPackageOrDepends(debFile.filePath(), debFile.architecture(), true);

    const QString architecture = debFile.architecture();
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();

    if (isBlackApplication(debFile.packageName())) {
        dependsStatus.status = DebListModel::Prohibit;
        dependsStatus.package = debFile.packageName();
        qWarning() << debFile.packageName() << "In the blacklist";
        return dependsStatus;
    }

    if (isArchErrorQstring(package_path)) {
        dependsStatus.status = DebListModel::ArchBreak;  // 添加ArchBreak错误。
        dependsStatus.package = debFile.packageName();
        QString packageName = debFile.packageName();
        return PackageDependsStatus::_break(packageName);
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, debFile.conflicts(), debFile.replaces());

    if (!debConflitsResult.is_ok()) {
        qWarning() << "PackagesManager:"
                   << "depends break because conflict" << debFile.packageName();
        dependsStatus.package = debConflitsResult.unwrap();
        dependsStatus.status = DebListModel::DependsBreak;
    } else {
        const ConflictResult localConflictsResult = isInstalledConflict(debFile.packageName(), debFile.version(), architecture);
        if (!localConflictsResult.is_ok()) {
            qWarning() << "PackagesManager:"
                       << "depends break because conflict with local package" << debFile.packageName();
            dependsStatus.package = localConflictsResult.unwrap();
            dependsStatus.status = DebListModel::DependsBreak;
        } else {
            QSet<QString> choose_set;
            choose_set << debFile.packageName();
            QStringList dependList;
            // 依赖信息映射
            QHash<QString, DependencyInfo> dependInfoMap;
            bool isWineApplication = false;         // 判断是否是wine应用
            for (auto ditem : debFile.depends()) {  // 每一个list中的关系是或的关系
                for (auto dinfo : ditem) {
                    // Note: 此处使用依赖包的架构查找软件包版本，某些场景下deb包架构和依赖包架构不一定一致。
                    QString packageArch = dinfo.multiArchAnnotation();
                    Package *depend = packageWithArch(dinfo.packageName(), packageArch);
                    if (depend) {
                        if (depend->name() == "deepin-elf-verify" || packageArch.isEmpty())  // deepi-elf-verify 是amd64架构非i386
                            dependList << depend->name();
                        else
                            dependList << depend->name() + ":" + depend->architecture();

                        dependInfoMap.insert(depend->name(), dinfo);
                        depend = nullptr;

                        if (dinfo.packageName().contains("deepin-wine"))  // 如果依赖中出现deepin-wine字段。则是wine应用
                            isWineApplication = true;
                    }
                }
            }
            installWineDepends = false;  // 标记wine依赖下载线程开启
            isDependsExists = false;     // 标记多架构依赖冲突
            m_pair.first.clear();        // 清空available依赖
            m_pair.second.clear();       // 清空broken依赖
            if (m_dependsPackages.contains(m_currentPkgMd5))
                m_dependsPackages.remove(m_currentPkgMd5);

            dependsStatus = checkDependsPackageStatus(choose_set, debFile.architecture(), debFile.depends());
            // 删除无用冗余的日志
            // 由于卸载p7zip会导致wine依赖被卸载，再次安装会造成应用闪退，因此判断的标准改为依赖不满足即调用pkexec
            // wine应用+非wine依赖不满足即可导致出问题
            do {
                if (isWineApplication && dependsStatus.status != DebListModel::DependsOk) {  // 增加是否是wine应用的判断
                    // 额外判断wine依赖是否已安装，同时剔除非wine依赖
                    filterNeedInstallWinePackage(dependList, debFile, dependInfoMap);

                    if (dependList.isEmpty()) {  // 所有的wine依赖均已安装
                        break;
                    }
                    dependsStatus.status = DebListModel::DependsBreak;  // 只要是下载，默认当前wine应用依赖为break
                }
            } while (0);
        }
    }
    if (dependsStatus.isBreak())
        Q_ASSERT(!dependsStatus.package.isEmpty());

    return dependsStatus;
}

QStringList PackagesManager::getPackageInfo(const QString &package_path)
{
    QStringList value_list;
    const DebFile deb(package_path);
    if (!deb.isValid())
        return value_list;
    QString packageName = deb.packageName();            // 包名
    QString filePath = deb.filePath();                  // 包的路径
    QString version = deb.version();                    // 包的版本
    QString architecture = deb.architecture();          // 包可用的架构
    QString shortDescription = deb.shortDescription();  // 包的短描述
    QString longDescription = deb.longDescription();    // 包的长描述
    value_list << packageName << filePath << version << architecture << shortDescription << longDescription;
    return value_list;
}

QString PackagesManager::checkPackageValid(const QStringList &package_path)
{
    for (QString debPackage : package_path) {  // 通过循环添加所有的包
        // 处理包不在本地的情况。
        if (!Utils::checkPackageReadable(debPackage)) {  // 判断路径信息是不是本地路径
            return "You can only install local deb packages";
        }

        QString debPkg = debPackage;
        // 处理package文件路径相关问题
        debPackage = dealPackagePath(debPackage);

        QApt::DebFile pkgFile(debPackage);
        // 判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            return "The deb package may be broken";
        }
        // 获取当前文件的md5的值,防止重复添加
        // 在checkInvalid中已经获取过md5,避免2次获取影响性能
        QByteArray md5 = m_allPackages.value(debPkg);
        if (md5.isEmpty())
            md5 = pkgFile.md5Sum();

        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            return "The deb package Already Added";
        }
    }
    return "";
}

PackagesManager::PackagesManager(QObject *parent)
    : QObject(parent)
{
    m_installWineThread = new DealDependThread();
    connect(m_installWineThread, &DealDependThread::signalDependResult, this, &PackagesManager::slotDealDependResult);
    connect(m_installWineThread, &DealDependThread::signalEnableCloseButton, this, &PackagesManager::signalEnableCloseButton);

    // 批量打开 分批加载线程
    m_pAddPackageThread = new AddPackageThread(m_appendedPackagesMd5);

    // 添加经过检查的包到软件中
    connect(m_pAddPackageThread,
            &AddPackageThread::signalAddPackageToInstaller,
            this,
            &PackagesManager::addPackage,
            Qt::AutoConnection);

    // append fail reason
    connect(m_pAddPackageThread, &AddPackageThread::signalAppendFailMessage, this, &PackagesManager::signalAppendFailMessage);

    // 处理包添加结束的信号
    // 处理包添加结束的信号
    connect(m_pAddPackageThread, &AddPackageThread::signalAppendFinished, this, &PackagesManager::slotAppendPackageFinished);

    getBlackApplications();
}

bool PackagesManager::isBackendReady()
{
    return PackageAnalyzer::instance().isBackendReady();
}

bool PackagesManager::isArchError(const int idx)
{
    if (idx < 0 || idx >= m_preparedPackages.size())
        return true;

    Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return true;
    }
    DebFile deb(m_preparedPackages[idx]);

    if (!deb.isValid())
        return false;

    const QString arch = deb.architecture();

    if ("all" == arch || "any" == arch)
        return false;

    bool architectures = !backend->architectures().contains(deb.architecture());

    return architectures;
}

bool PackagesManager::isArchErrorQstring(const QString &package_name)
{
    Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return true;
    }
    DebFile deb(package_name);

    if (!deb.isValid())
        return false;

    const QString arch = deb.architecture();

    if ("all" == arch || "any" == arch)
        return false;

    bool architectures = !backend->architectures().contains(deb.architecture());

    return architectures;
}

const ConflictResult PackagesManager::packageConflictStat(const int index)
{
    if (index < 0 || index >= m_preparedPackages.size())
        return ConflictResult::err("");

    DebFile debfile(m_preparedPackages[index]);
    if (!debfile.isValid())
        return ConflictResult::err("");
    ConflictResult ConflictResult = isConflictSatisfy(debfile.architecture(), debfile.conflicts(), debfile.replaces());
    return ConflictResult;
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, Package *package)
{
    if (!package) {
        qWarning() << "invalid package pointer";
        return ConflictResult::err("");
    }

    const QString &packageName = package->name();
    const auto ret_installed = isInstalledConflict(packageName, package->version(), package->architecture());
    if (!ret_installed.is_ok()) {
        qWarning() << packageName << "check installed conflict not satisfied";
        return ret_installed;
    }

    const auto conflictStatus = isConflictSatisfy(arch, package->conflicts(), package->replaces());

    return conflictStatus;
}

const ConflictResult
PackagesManager::isInstalledConflict(const QString &packageName, const QString &packageVersion, const QString &packageArch)
{
    static QList<QPair<QString, DependencyInfo>> sysConflicts;

    if (sysConflicts.isEmpty()) {
        Backend *backend = PackageAnalyzer::instance().backendPtr();
        for (Package *pkg : backend->availablePackages()) {
            if (!pkg)
                continue;
            if (!pkg->isInstalled()) {
                pkg = nullptr;
                continue;
            }
            const auto &conflicts = pkg->conflicts();
            if (conflicts.isEmpty()) {
                pkg = nullptr;
                continue;
            }

            for (const auto &conflict_list : conflicts)
                for (const auto &conflict : conflict_list) {
                    // FIXME: 在 Realse 模式下，某些场景后续调用 QLatin1String 时可能访问失败(内存被释放)!
                    // 临时修改为拷贝数据。
                    sysConflicts << QPair<QString, DependencyInfo>(pkg->name(), conflict);
                }
            pkg = nullptr;
        }
    }

    Package *pkg = packageWithArch(packageName, packageArch);
    if (pkg && pkg->installedVersion() == packageVersion)
        return ConflictResult::ok(QString());

    for (const auto &info : sysConflicts) {
        const auto &conflict = info.second;
        const auto &pkgName = conflict.packageName();
        const auto &pkgVersion = conflict.packageVersion();
        const auto &pkgArch = conflict.multiArchAnnotation();

        if (pkgName != packageName)
            continue;

        /* 部分特殊软件包 conflicts 包名和当前包名一致，若一致，则认为无效
           e.g.: 在 debian/control 文件配置中按如下设置的软件包
            Pakcage: ImageEnhance
            Conflicts: ImageEnhance
            Replaces: ImageEnhnace
        */
        if (pkg->name() == info.first) {
            continue;
        }

        // pass if arch not match
        if (!pkgArch.isEmpty() && pkgArch != packageArch && pkgArch != "any" && pkgArch != "native")
            continue;

        if (pkgVersion.isEmpty())
            return ConflictResult::err(info.first);

        const int relation = Package::compareVersion(packageVersion, conflict.packageVersion());
        // match, so is bad
        if (dependencyVersionMatch(relation, conflict.relationType()))
            return ConflictResult::err(info.first);
    }
    return ConflictResult::ok(QString());
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch,
                                                        const QList<DependencyItem> &conflicts,
                                                        const QList<DependencyItem> &replaces)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();

            // 修复依赖中 conflict与provides 存在相同 virtual package
            // 此前使用packageWithArch, 在package打包失败时，会寻找virtual package的提供的其他包
            // 在dde-daemon中 lastore-daemon-migration与dde-daemon为conflict,
            // lastore-daemon-migration 又提供了dde-daemon,导致最后打成的包不是virtual package而是provides的包
            Backend *backend = PackageAnalyzer::instance().backendPtr();
            if (!backend)
                return ConflictResult::err(QString());
            Package *package = backend->package(name);

            if (!package)
                continue;

            if (!package->isInstalled()) {
                package = nullptr;
                continue;
            }
            // arch error, conflicts
            if (!isArchMatches(arch, package->architecture(), package->multiArchType())) {
                qWarning() << "PackagesManager:"
                           << "conflicts package installed: " << arch << package->name() << package->architecture()
                           << package->multiArchTypeString();
                package = nullptr;
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = package->installedVersion();
            const auto type = conflict.relationType();
            const auto result = Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type)) {
                package = nullptr;
                continue;
            }
            // test package
            const QString mirror_version = package->availableVersion();

            // 删除版本相同比较，如果安装且版本符合则判断冲突，此前逻辑存在问题
            //  mirror version is also break
            const auto mirror_result = Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type) && name != m_currentPkgName) {  // 此处即可确认冲突成立
                // 额外判断是否会替换此包
                bool conflict_yes = true;
                for (auto replace_list : replaces) {
                    for (auto replace : replace_list) {
                        if (replace.packageName() == name) {            // 包名符合
                            auto replaceType = replace.relationType();  // 提取版本号规则
                            auto versionCompare =
                                Package::compareVersion(installed_version, replace.packageVersion());  // 比较版本号
                            if (dependencyVersionMatch(versionCompare, replaceType)) {  // 如果版本号符合要求，即判定replace成立
                                conflict_yes = false;
                                break;
                            }
                        }
                    }
                    if (!conflict_yes) {
                        break;
                    }
                }

                if (!conflict_yes) {
                    package = nullptr;
                    continue;
                }

                qWarning() << "PackagesManager:"
                           << "conflicts package installed: " << arch << package->name() << package->architecture()
                           << package->multiArchTypeString() << mirror_version << conflict_version;
                return ConflictResult::err(name);
            }
        }
    }
    return ConflictResult::ok(QString());
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();

            // 修复依赖中 conflict与provides 存在相同 virtual package
            // 此前使用packageWithArch, 在package打包失败时，会寻找virtual package的提供的其他包
            // 在dde-daemon中 lastore-daemon-migration与dde-daemon为conflict,
            // lastore-daemon-migration 又提供了dde-daemon,导致最后打成的包不是virtual package而是provides的包
            Backend *backend = PackageAnalyzer::instance().backendPtr();
            if (!backend)
                return ConflictResult::err(QString());
            Package *package = backend->package(name);

            if (!package)
                continue;

            if (!package->isInstalled()) {
                package = nullptr;
                continue;
            }
            // arch error, conflicts
            if (!isArchMatches(arch, package->architecture(), package->multiArchType())) {
                qWarning() << "PackagesManager:"
                           << "conflicts package installed: " << arch << package->name() << package->architecture()
                           << package->multiArchTypeString();
                package = nullptr;
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = package->installedVersion();
            const auto type = conflict.relationType();
            const auto result = Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type)) {
                package = nullptr;
                continue;
            }
            // test package
            const QString mirror_version = package->availableVersion();

            // 删除版本相同比较，如果安装且版本符合则判断冲突，此前逻辑存在问题
            //  mirror version is also break
            const auto mirror_result = Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type) && name != m_currentPkgName) {
                qWarning() << "PackagesManager:"
                           << "conflicts package installed: " << arch << package->name() << package->architecture()
                           << package->multiArchTypeString() << mirror_version << conflict_version;
                package = nullptr;
                return ConflictResult::err(name);
            }
        }
    }
    return ConflictResult::ok(QString());
}

int PackagesManager::packageInstallStatus(const int index)
{
    if (index < 0 || index >= m_preparedPackages.size())
        return -1;
    // 修改安装状态的存放方式，将安装状态与MD5绑定，而非与index绑定
    // 如果此时已经刷新过安装状态，则直接返回。
    // PS: 修改原因见头文件

    // 提前获取当前的md5
    auto currentPackageMd5 = m_packageMd5[index];
    if (m_packageInstallStatus.contains(currentPackageMd5))
        return m_packageInstallStatus[currentPackageMd5];

    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return DebListModel::NotInstalled;
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return DebListModel::NotInstalled;
    }
    Package *package = packageWithArch(packageName, packageArch);

    if (!package)
        return DebListModel::NotInstalled;

    const QString installedVersion = package->installedVersion();
    package = nullptr;
    if (installedVersion.isEmpty())
        return DebListModel::NotInstalled;

    const QString packageVersion = debFile.version();
    const int result = Package::compareVersion(packageVersion, installedVersion);

    int ret;
    if (result == 0)
        ret = DebListModel::InstalledSameVersion;
    else if (result < 0)
        ret = DebListModel::InstalledLaterVersion;
    else
        ret = DebListModel::InstalledEarlierVersion;

    // 存储包的安装状态
    // 2020-11-19 修改安装状态的存储绑定方式
    m_packageInstallStatus[currentPackageMd5] = ret;
    return ret;
}

void PackagesManager::slotDealDependResult(int iAuthRes, int iIndex, const QString &dependName)
{
    if (iIndex < 0 || iIndex > m_preparedPackages.size())
        return;
    if (iAuthRes == DebListModel::AuthDependsSuccess) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsOk;  // 更换依赖的存储结构
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsOk;  // 更换依赖的存储结构
        }
        m_errorIndex.clear();
    }
    if (iAuthRes == DebListModel::CancelAuth || iAuthRes == DebListModel::AnalysisErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status =
                DebListModel::DependsAuthCancel;  // 更换依赖的存储结构
        }
        emit signalEnableCloseButton(true);
    }
    if (iAuthRes == DebListModel::AuthDependsErr || iAuthRes == DebListModel::AnalysisErr ||
        iAuthRes == DebListModel::VerifyDependsErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsBreak;  // 更换依赖的存储结构
            if (!m_errorIndex.contains(m_dependInstallMark[num]))
                m_errorIndex.insert(m_dependInstallMark[num], iAuthRes);
        }
        if (installWineDepends) {  // 下载wine依赖失败时，考虑出现依赖缺失的情况
            qInfo() << "check wine depends again !" << iIndex;
            getPackageDependsStatus(iIndex);
            if (!m_dependsPackages.isEmpty()) {
                qInfo() << m_dependsPackages.size() << m_dependsPackages.value(m_currentPkgMd5).second.size();
                if (1 == m_preparedPackages.size()) {}
                    // Q_EMIT signalDependPackages(m_dependsPackages.value(m_currentPkgMd5), false);
                else if (m_preparedPackages.size() > 1)
                    installWineDepends = false;
            }
        }
        emit signalEnableCloseButton(true);
    }
    emit signalDependResult(iAuthRes, iIndex, dependName);
}

/**
 * @brief PackagesManager::getPackageMd5 获取某个包的md5 值
 * @param index 包的下表
 * @return  包的md5
 * 现在包的状态与md5绑定，下标再与md5绑定，而非直接与下标绑定
 * 这种做法的优点在于，不需要在前端再去调整所有包的顺序，只需要获取对应下标的md5即可,调整大多数状态不需要担心状态与下标对应错乱
 * 缺点是：每次获取状态都需要读md5.频繁读取会造成性能影响
 */
QByteArray PackagesManager::getPackageMd5(const int index)
{
    if (index < m_packageMd5.size())
        return m_packageMd5[index];
    return nullptr;
}

/**
 * @brief PackagesManager::getPackageDependsStatus 获取某个包的依赖状态
 * @param index 包的下标
 * @return 包的依赖状态
 */
PackageDependsStatus PackagesManager::getPackageDependsStatus(const int index)
{
    // 提前获取需要的md5
    if (index < 0 || index >= m_preparedPackages.size()) {
        qWarning() << "invalid param index";
        return PackageDependsStatus::_break("");
    }
    auto currentPackageMd5 = m_packageMd5[index];
    m_currentPkgMd5 = currentPackageMd5;

    if (m_packageMd5DependsStatus.contains(currentPackageMd5))
        return m_packageMd5DependsStatus[currentPackageMd5];

    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return PackageDependsStatus::_break("");
    m_currentPkgName = debFile.packageName();
    m_orDepends.clear();
    m_checkedOrDependsStatus.clear();
    m_unCheckedOrDepends.clear();
    m_dependsInfo.clear();
    m_loopErrorDeepends.clear();
    // 用debFile.packageName()无法打开deb文件，故替换成debFile.filePath()
    // 更新m_dependsInfo
    getPackageOrDepends(debFile.filePath(), debFile.architecture(), true);

    const QString architecture = debFile.architecture();
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();

    if (isBlackApplication(debFile.packageName())) {
        dependsStatus.status = DebListModel::Prohibit;
        dependsStatus.package = debFile.packageName();
        m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);
        qWarning() << debFile.packageName() << "In the blacklist";
        return dependsStatus;
    }

    if (isArchError(index)) {
        dependsStatus.status = DebListModel::ArchBreak;  // 添加ArchBreak错误。
        dependsStatus.package = debFile.packageName();
        m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);  // 更换依赖的存储方式
        return dependsStatus;
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, debFile.conflicts(), debFile.replaces());

    if (!debConflitsResult.is_ok()) {
        qWarning() << "PackagesManager:"
                   << "depends break because conflict" << debFile.packageName();
        dependsStatus.package = debConflitsResult.unwrap();
        dependsStatus.status = DebListModel::DependsBreak;
    } else {
        const ConflictResult localConflictsResult = isInstalledConflict(debFile.packageName(), debFile.version(), architecture);
        if (!localConflictsResult.is_ok()) {
            qWarning() << "PackagesManager:"
                       << "depends break because conflict with local package" << debFile.packageName();
            dependsStatus.package = localConflictsResult.unwrap();
            dependsStatus.status = DebListModel::DependsBreak;
        } else {
            QSet<QString> choose_set;
            choose_set << debFile.packageName();
            QStringList dependList;
            QHash<QString, DependencyInfo> dependInfoMap;
            bool isWineApplication = false;         // 判断是否是wine应用
            for (auto ditem : debFile.depends()) {  // 每一个list中的关系是或的关系
                for (auto dinfo : ditem) {
                    // Note: 此处使用依赖包的架构查找软件包版本，某些场景下deb包架构和依赖包架构不一定一致。
                    QString packageArch = dinfo.multiArchAnnotation();
                    Package *depend = packageWithArch(dinfo.packageName(), packageArch);
                    if (depend) {
                        QString dependName;
                        if (depend->name() == "deepin-elf-verify" || packageArch.isEmpty())  // deepin-elf-verify
                                                                                             // 是amd64架构非i386
                            dependName = depend->name();
                        else
                            dependName = depend->name() + ":" + depend->architecture();

                        dependList << dependName;
                        dependInfoMap.insert(dependName, dinfo);
                        depend = nullptr;

                        if (dinfo.packageName().contains("deepin-wine"))  // 如果依赖中出现deepin-wine字段。则是wine应用
                            isWineApplication = true;
                    }
                }
            }
            installWineDepends = false;  // 标记wine依赖下载线程开启
            isDependsExists = false;     // 标记多架构依赖冲突
            m_pair.first.clear();        // 清空available依赖
            m_pair.second.clear();       // 清空broken依赖
            if (m_dependsPackages.contains(m_currentPkgMd5))
                m_dependsPackages.remove(m_currentPkgMd5);

            dependsStatus = checkDependsPackageStatus(choose_set, debFile.architecture(), debFile.depends());
            // 删除无用冗余的日志
            // 由于卸载p7zip会导致wine依赖被卸载，再次安装会造成应用闪退，因此判断的标准改为依赖不满足即调用pkexec
            // wine应用+非wine依赖不满足即可导致出问题
            do {
                if (isWineApplication && dependsStatus.status != DebListModel::DependsOk) {  // 增加是否是wine应用的判断
                    // 额外判断wine依赖是否已安装，同时剔除非wine依赖
                    filterNeedInstallWinePackage(dependList, debFile, dependInfoMap);

                    if (dependList.isEmpty()) {  // 所有的wine依赖均已安装
                        break;
                    }

                    if (!m_dependInstallMark.contains(currentPackageMd5)) {  // 更换判断依赖错误的标记
                        installWineDepends = true;
                        if (!m_installWineThread->isRunning()) {
                            m_dependInstallMark.append(currentPackageMd5);  // 依赖错误的软件包的标记 更改为md5取代验证下标
                            qInfo() << "PackagesManager:"
                                    << "wine command install depends:" << dependList;
                            m_installWineThread->setDependsList(dependList, index);
                            if (m_brokenDepend.isEmpty())
                                m_brokenDepend = dependsStatus.package;
                            m_installWineThread->setBrokenDepend(m_brokenDepend);
                            m_installWineThread->run();
                        }
                    }
                    dependsStatus.status = DebListModel::DependsBreak;  // 只要是下载，默认当前wine应用依赖为break
                }
            } while (0);
        }
    }
    if (dependsStatus.isBreak())
        Q_ASSERT(!dependsStatus.package.isEmpty());

    m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);
    return dependsStatus;
}

void PackagesManager::getPackageOrDepends(const QString &package, const QString &arch, bool flag)
{
    /*
     * 解析安装包依赖，若存在或依赖关系则进行处理并且存储
     *such as,teamviewer depends: "libc6 (>= 2.17), libdbus-1-3, libqt5gui5 (>= 5.5)| qt56-teamviewer,
     * libqt5widgets5 (>= 5.5) | qt56-teamviewer, libqt5qml5 (>= 5.5) | qt56-teamviewer, libqt5quick5 (>= 5.5) |
     *qt56-teamviewer..."
     */

    // 更新m_dependsInfo
    auto insertToDependsInfo = [this](const QList<DependencyItem> &depends) {
        for (auto candicate_list : depends) {
            for (const auto &info : candicate_list) {
                m_dependsInfo.insert(info.packageName(), info);
            }
        }
    };

    auto checkVirtualPackage = [this](QVector<QString> &dependStatus, const QString &depend) {
        QVector<QString> virtualPackage;
        Backend *backend = PackageAnalyzer::instance().backendPtr();
        for (auto *availablePackage : backend->availablePackages()) {
            if (!availablePackage->providesList().contains(depend)) {
                continue;
            }

            if (!dependStatus.contains(availablePackage->name())) {
                dependStatus.append(availablePackage->name());
                virtualPackage.append(availablePackage->name());

                // 使用虚包的依赖关系
                if (m_dependsInfo.contains(depend)) {
                    m_dependsInfo.insert(availablePackage->name(), m_dependsInfo.value(depend));
                }
            }
        }

        qInfo() << QString("Detect or depends %1 contains virtual packages: ").arg(depend) << virtualPackage;
    };

    QString packageName;
    QString controlDepends;
    if (flag) {
        DebFile debFile(package);
        if (!debFile.isValid())
            return;
        packageName = debFile.packageName();
        controlDepends = debFile.controlField("Depends");
        // 软件包
        insertToDependsInfo(debFile.depends());
    } else {
        QApt::Package *pkg = packageWithArch(package, arch);
        if (!pkg)
            return;
        packageName = pkg->name();
        controlDepends = pkg->controlField("Depends");
        // 子依赖
        insertToDependsInfo(pkg->depends());
    }
    // 仅在Debug下打印
    qDebug() << qPrintable("Package:") << packageName << qPrintable("controlDepends") << controlDepends;
    QStringList dependsList = controlDepends.split(",");

    // Fix 229757，不直接移除非或包依赖，而是判断是否存在虚包。

    // 使用二维数组进行存储
    for (QString depend : dependsList) {
        depend = depend.remove(QRegExp("\\s"));

        // 非或包，判断虚包依赖
        if (!depend.contains("|")) {
            // 截取依赖包名
            if (depend.contains("(")) {
                int mid = depend.indexOf("(");
                depend = depend.left(mid);
            }

            QVector<QString> dependStatus;
            // 判断是否为虚包，若为虚包，将实现包都加入或列表
            Package *package = packageWithArch(depend, arch, arch);
            if (package && depend != package->name()) {
                // 需要将当前包同样插入，方便查找
                dependStatus.append(depend);

                checkVirtualPackage(dependStatus, depend);
            }

            if (!dependStatus.isEmpty()) {
                m_orDepends.append(dependStatus);
                m_unCheckedOrDepends.append(dependStatus);
            }

            continue;
        }

        QStringList orDepends = depend.split("|");
        QVector<QString> dependStatus;
        for (QString ordepend : orDepends) {
            // 截取依赖包名
            if (ordepend.contains("(")) {
                int mid = ordepend.indexOf("(");
                ordepend = ordepend.left(mid);
            }

            // 判断是否为虚包，若为虚包，将实现包都加入或列表
            Package *package = packageWithArch(ordepend, arch, arch);
            if (package && ordepend != package->name()) {
                checkVirtualPackage(dependStatus, ordepend);
            } else {
                dependStatus.append(ordepend);
            }
        }
        m_orDepends.append(dependStatus);
        m_unCheckedOrDepends.append(dependStatus);
    }
    qDebug() << qPrintable("Package:") << packageName << qPrintable("orDepends") << m_orDepends;
}

const QString PackagesManager::packageInstalledVersion(const int index)
{
    // 更换安装状态的存储结构
    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return "";
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qWarning() << "libqapt backend loading error";
        return "";
    }
    Package *package = backend->package(packageName + ":" + packageArch);

    // 修复可能某些包无法package的错误，如果遇到此类包，返回安装版本为空
    if (package) {
        return package->installedVersion();  // 能正常打包，返回包的安装版本
    } else
        return "";  // 此包无法正常package，返回空
}

const QStringList PackagesManager::packageAvailableDepends(const int index)
{
    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return QStringList();
    QSet<QString> choose_set;
    const QString debArch = debFile.architecture();
    const auto &depends = debFile.depends();
    m_unCheckedOrDepends = m_orDepends;

    QString levelInfo = QString("%1:%2 (%3)").arg(debFile.packageName()).arg(debArch).arg(debFile.version());
    packageCandidateChoose(choose_set, debArch, depends, levelInfo);

    // TODO: check upgrade from conflicts
    return choose_set.toList();
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set,
                                             const QString &debArch,
                                             const QList<DependencyItem> &dependsList,
                                             const QString &levelInfo)
{
    qInfo() << "[Package Choose]" << levelInfo;

    for (auto const &candidate_list : dependsList)
        packageCandidateChoose(choosed_set, debArch, candidate_list, levelInfo);

    qInfo() << "[Package Choose End]" << levelInfo;
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set,
                                             const QString &debArch,
                                             const DependencyItem &candidateList,
                                             const QString &levelInfo)
{
    for (const auto &info : candidateList) {
        Package *package = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!package)
            continue;

        const auto choosed_name = package->name() + resolvMultiArchAnnotation(QString(), package->architecture());
        if (choosed_set.contains(choosed_name))
            break;

        QString packageInfo = QString("%1 (%2)").arg(choosed_name).arg(package->version());
        QVector<QString> infos;
        if (!m_unCheckedOrDepends.isEmpty()) {
            for (auto dInfo : m_unCheckedOrDepends) {  // 遍历或依赖容器中容器中是否存在当前依赖
                if (!dInfo.contains(package->name())) {
                    continue;
                } else {
                    infos = dInfo;
                    m_unCheckedOrDepends.removeOne(dInfo);
                }
            }
        }

        qDebug() << __func__ << infos << package->name() << "ChooseName:" << choosed_name;

        if (infos.isEmpty()) {  // 没有或依赖关系或者当前依赖不属于或依赖关系
            qDebug() << "not ordepends or not contain depend";
            // 当前依赖未安装，则安装当前依赖。
            if (package->installedVersion().isEmpty()) {
                choosed_set << choosed_name;
            } else {
                // 当前依赖已安装，判断是否需要升级
                //  修复升级依赖时，因为依赖包版本过低，造成安装循环。
                // 删除无用冗余的日志
                if (Package::compareVersion(package->installedVersion(), info.packageVersion()) < 0) {
                    Backend *backend = PackageAnalyzer::instance().backendPtr();
                    if (!backend) {
                        qWarning() << "libqapt backend loading error";
                        return;
                    }
                    Package *updatePackage =
                        backend->package(package->name() + resolvMultiArchAnnotation(QString(), package->architecture()));
                    if (updatePackage)
                        choosed_set << updatePackage->name() + resolvMultiArchAnnotation(QString(), package->architecture());
                    else
                        choosed_set << info.packageName() + " not found";

                } else {  // 若依赖包符合版本要求,则不进行升级
                    continue;
                }
            }
        } else {  // 存在或依赖且当前依赖属于或依赖关系
            bool isInstalling = false;
            for (auto iter = infos.begin(); iter != infos.end(); iter++) {
                Backend *backend = PackageAnalyzer::instance().backendPtr();
                if (!backend) {
                    qWarning() << "libqapt backend loading error";
                    return;
                }
                Package *otherPackage = backend->package(*iter + resolvMultiArchAnnotation(QString(), debArch));
                if (!otherPackage)
                    continue;
                qDebug() << __func__ << *iter << otherPackage->installedVersion() << m_dependsInfo[*iter].packageVersion();
                if (otherPackage->compareVersion(otherPackage->installedVersion(), m_dependsInfo[*iter].packageVersion()) >= 0 &&
                    !otherPackage->installedVersion().isEmpty()) {
                    // 如果或依赖中有依赖已安装且符合版本要求，则当前依赖不进行下载
                    isInstalling = true;
                    break;
                }
            }

            if (!isInstalling) {  // 若或依赖中其他依赖也需要下载或者存在缺失，则当前依赖需要下载
                qDebug() << __func__ << isInstalling << QString("%1 need to install current depend").arg(packageInfo);
                if (package->installedVersion().isEmpty()) {
                    choosed_set << choosed_name;
                } else {
                    if (Package::compareVersion(package->installedVersion(), info.packageVersion()) < 0) {
                        Backend *backend = PackageAnalyzer::instance().backendPtr();
                        if (!backend)
                            return;
                        Package *updatePackage =
                            backend->package(package->name() + resolvMultiArchAnnotation(QString(), package->architecture()));
                        if (updatePackage)
                            choosed_set << updatePackage->name() + resolvMultiArchAnnotation(QString(), package->architecture());
                        else
                            choosed_set << info.packageName() + " not found";

                    } else {
                        continue;
                    }
                }

            } else {
                qDebug() << __func__ << isInstalling << QString("%1 other ordepend is installed").arg(packageInfo);
                break;
            }
        }

        if (!isConflictSatisfy(debArch, package->conflicts(), package->replaces()).is_ok())
            continue;

        QSet<QString> upgradeDependsSet = choosed_set;
        upgradeDependsSet << choosed_name;
        const auto stat = checkDependsPackageStatus(upgradeDependsSet, package->architecture(), package->depends());
        if (stat.isBreak())
            continue;

        choosed_set << choosed_name;
        // 使用依赖包请求架构递归解析，而不是使用安装包的架构！(例如：i386 软件包依赖 amd64 软件包)
        packageCandidateChoose(
            choosed_set, package->architecture(), package->depends(), QString("%1 -> %2").arg(levelInfo).arg(packageInfo));
        break;
    }
}

QMap<QString, QString> PackagesManager::specialPackage()
{
    QMap<QString, QString> sp;
    sp.insert("deepin-wine-plugin-virtual", "deepin-wine-helper");
    sp.insert("deepin-wine32", "deepin-wine");
    sp.insert("deepin-wine-helper", "deepin-wine-plugin");

    return sp;
}

const QStringList PackagesManager::packageReverseDependsList(const QString &packageName, const QString &sysArch)
{
    Package *package = packageWithArch(packageName, sysArch);
    if (!package) {
        qWarning() << "Failed to package from" << packageName << "with" << sysArch;
        return {};
    }
    QStringList requiredList = package->requiredByList();

    package = nullptr;

    // 确定和当前包存在直接或间接反向依赖的包的集合
    QSet<QString> reverseDependSet { packageName };

    // 存放当前需要验证反向依赖的包
    QQueue<QString> reverseQueue;

    for (const auto &requiredPackage : requiredList.toSet())
        reverseQueue.append(requiredPackage);
    while (!reverseQueue.isEmpty()) {
        const auto item = reverseQueue.first();
        reverseQueue.pop_front();

        if (reverseDependSet.contains(item))
            continue;

        Package *currentPackage = packageWithArch(item, sysArch);
        if (!currentPackage || !currentPackage->isInstalled()) {
            currentPackage = nullptr;
            continue;
        }
        if (currentPackage->recommendsList().contains(packageName)) {
            currentPackage = nullptr;
            continue;
        }
        if (currentPackage->suggestsList().contains(packageName)) {
            currentPackage = nullptr;
            continue;
        }

        // Conflict / Replace / Break 的反向依赖同样跳过
        if (isNegativeReverseDepend(packageName, currentPackage)) {
            currentPackage = nullptr;
            continue;
        }

        reverseDependSet << item;

        if (specialPackage().contains(item))
            reverseQueue.append(specialPackage()[item]);

        // 判断当前反向依赖是否有反向依赖
        for (const auto &dependRequiredPackage : currentPackage->requiredByList()) {
            if (reverseDependSet.contains(dependRequiredPackage) || reverseQueue.contains(dependRequiredPackage))
                continue;
            Package *subPackage = packageWithArch(dependRequiredPackage, sysArch);
            if (dependRequiredPackage.startsWith("deepin.")) {  // 此类wine应用在系统中的存在都是以deepin.开头
                // 部分wine应用在系统中有一个替换的名字，使用requiredByList 可以获取到这些名字
                if (subPackage && !subPackage->requiredByList().isEmpty()) {  // 增加对package指针的检查
                    for (QString rdepends : subPackage->requiredByList()) {
                        reverseQueue.append(rdepends);
                    }
                }
            }
            if (!subPackage || !subPackage->isInstalled()) {  // 增加对package指针的检查
                subPackage = nullptr;
                continue;
            }
            if (subPackage->recommendsList().contains(item)) {
                subPackage = nullptr;
                continue;
            }
            if (subPackage->suggestsList().contains(item)) {
                subPackage = nullptr;
                continue;
            }
            reverseQueue.append(dependRequiredPackage);
        }

        currentPackage = nullptr;
    }
    // remove self
    reverseDependSet.remove(packageName);

    return reverseDependSet.toList();
}

bool PackagesManager::isNegativeReverseDepend(const QString &packageName, const QApt::Package *reverseDepend)
{
    if (!reverseDepend) {
        return false;
    }

    static auto containPackage = [](const QString &packageName, const QList<DependencyItem> &itemList) -> bool {
        return std::any_of(itemList.begin(), itemList.end(), [&](const DependencyItem &item) {
            return std::any_of(
                item.begin(), item.end(), [&](const DependencyInfo &info) { return info.packageName() == packageName; });
        });
    };

    // 不排除部分包设置 替换(Replace)/破坏(Breaks) 仍依赖 packageName ， Depends 字段设置视为非消极包
    if (containPackage(packageName, reverseDepend->depends())) {
        return false;
    }

    return containPackage(packageName, reverseDepend->conflicts()) || containPackage(packageName, reverseDepend->replaces()) ||
           containPackage(packageName, reverseDepend->breaks());
}

void PackagesManager::reset()
{
    m_errorIndex.clear();
    m_dependInstallMark.clear();
    m_preparedPackages.clear();
    m_packageInstallStatus.clear();
    m_packageMd5DependsStatus.clear();  // 修改依赖状态的存储结构，此处清空存储的依赖状态数据
    m_appendedPackagesMd5.clear();
    m_packageMd5.clear();
    m_dependGraph.reset();

    // reloadCache必须要加
    PackageAnalyzer::instance().backendPtr()->reloadCache();
    m_dependsPackages.clear();
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    // 查看此包是否已经存储依赖状态。
    // 提前获取package 的md5
    auto currentPackageMd5 = m_packageMd5[index];
    if (!m_packageMd5DependsStatus.contains(currentPackageMd5)) {
        return;
    } else {
        // 针对wine依赖做一个特殊处理，如果wine依赖break,则直接返回。
        if ((m_packageMd5DependsStatus[currentPackageMd5].package == "deepin-wine") &&
            m_packageMd5DependsStatus[currentPackageMd5].status != DebListModel::DependsOk)
            return;
    }
    // reload backend cache
    // reloadCache必须要加
    PackageAnalyzer::instance().backendPtr()->reloadCache();
    m_packageMd5DependsStatus.remove(currentPackageMd5);  // 删除当前包的依赖状态（之后会重新获取此包的依赖状态）
}

/**
 * @brief PackagesManager::removePackage 删除指定下标的包
 * @param index 指定的下标
 */
void PackagesManager::removePackage(const int index)
{
    if (index < 0 || index >= m_preparedPackages.size()) {
        qWarning() << "[PackagesManager]"
                   << "[removePackage]"
                   << "Subscript boundary check error";
        return;
    }

    // 如果此前的文件已经被修改,则获取到的MD5的值与之前不同,因此从现有的md5中寻找.
    const auto md5 = m_packageMd5[index];

    // 提前删除标记list中的md5 否则在删除最后一个的时候会崩溃
    if (m_dependInstallMark.contains(md5))  // 如果这个包是wine包，则在wine标记list中删除
        m_dependInstallMark.removeOne(md5);

    m_appendedPackagesMd5.remove(md5);
    m_preparedPackages.removeAt(index);

    m_appendedPackagesMd5.remove(md5);      // 在判断是否重复的md5的集合中删除掉当前包的md5
    m_packageMd5DependsStatus.remove(md5);  // 删除指定包的依赖状态
    m_packageMd5.removeAt(index);           // 在索引map中删除指定的项
    m_dependsPackages.remove(md5);          // 删除指定包的依赖关系

    m_dependGraph.remove(md5);  // 从依赖关系图中删除对应节点

    m_packageInstallStatus.clear();

    // 告诉model md5更新了
    emit signalPackageMd5Changed(m_packageMd5);

    // notify data changed
    Q_EMIT signalPackageCountChanged(m_preparedPackages.size());
}

/**
 * @brief PackagesManager::appendPackage 将前端给的包，传输到添加线程中。并开始添加
 * @param packages 要添加的包的列表
 * 此处可以优化，如果只有一两个包直接添加
 * 大于等于三个包，先添加两个再开始线程
 */
void PackagesManager::appendPackage(QStringList packages)
{
    if (packages.isEmpty())  // 当前放进来的包列表为空（可能拖入的是文件夹）
        return;
    checkInvalid(packages);
    if (1 == packages.size()) {
        appendNoThread(packages, packages.size());
    } else {
        QStringList subPackages;
        subPackages << packages[0];
        appendNoThread(subPackages, packages.size());
        packages.removeAt(0);

        if (packages.isEmpty())
            return;
        if (!m_allPackages.isEmpty())
            m_pAddPackageThread->setSamePackageMd5(m_allPackages);
        m_pAddPackageThread->setPackages(packages, m_validPackageCount);   // 传递要添加的包到添加线程中
        m_pAddPackageThread->setAppendPackagesMd5(m_appendedPackagesMd5);  // 传递当前已经添加的包的MD5 判重时使用

        m_pAddPackageThread->start();  // 开始添加线程
    }
}

/**
 * @brief AddPackageThread::checkInvalid 检查有效文件的数量
 */
void PackagesManager::checkInvalid(const QStringList &packages)
{
    m_allPackages.clear();
    m_validPackageCount = packages.size();
    int validCount = 0;    // 计入有效包的数量
    QSet<qint64> pkgSize;  // 存储安装包的安装大小,初步去除无效包以及可能重复的包
    for (auto package : packages) {
        QApt::DebFile file(package);
        if (!file.isValid()) {
            m_validPackageCount--;
            continue;
        }
        validCount++;
        auto size = file.installedSize();
        if (pkgSize.contains(size)) {
            m_validPackageCount--;
            continue;
        }
        pkgSize << size;
    }
    // m_validPackageCount==1,可能有效包都是重复包，需要最终根据md5值来判定
    QSet<QByteArray> pkgMd5;  // 最后通过md5来区分是否是重复包
    if (1 == m_validPackageCount && validCount > 1) {
        for (auto package : packages) {
            QApt::DebFile pkgFile(package);
            if (!pkgFile.isValid())
                continue;
            auto md5 = pkgFile.md5Sum();
            m_allPackages.insert(package, md5);
            if (!pkgMd5.isEmpty() && !pkgMd5.contains(md5)) {  // 根据md5判断，有不是重复的包，刷新批量界面
                m_validPackageCount = 2;
                break;
            }
            pkgMd5 << md5;
        }
    }
}

/**
 * @brief PackagesManager::dealInvalidPackage 处理不在本地的安装包
 * @param packagePath 包的路径
 * @return 包是否在本地
 *   true   : 包在本地
 *   fasle  : 文件不在本地
 */
bool PackagesManager::dealInvalidPackage(const QString &packagePath)
{
    // 判断路径信息是不是本地路径
    if (!Utils::checkPackageReadable(packagePath)) {
        QFileInfo debFileIfo(packagePath);
        // 查看包是否能够打开，无法打开说明包不在本地或无权限。
        QFile outfile(packagePath.toUtf8());
        outfile.open(QFile::ReadOnly);

        if (!outfile.isOpen()) {  // 打不开，文件不在本地或无安装权限
            QFile::FileError error = outfile.error();
            if (error == QFile::FileError::NoError) {
                // 文件不存在或路径错误
                Q_EMIT signalAppendFailMessage(Pkg::PackageNotLocal);
                return false;
            } else {
                // 无安装权限
                Q_EMIT signalAppendFailMessage(Pkg::PackageNotInstallable);
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief PackagesManager::dealPackagePath 处理路径相关的问题
 * @param packagePath 当前包的文件路径
 * @return 处理后的文件路径
 * 处理两种情况
 *      1： 相对路径             --------> 转化为绝对路径
 *      2： 包的路径中存在空格     --------> 使用软链接，链接到/tmp下
 */
QString PackagesManager::dealPackagePath(const QString &packagePath)
{
    auto tempPath = packagePath;
    // 判断当前文件路径是否是绝对路径，不是的话转换为绝对路径
    if (!tempPath.startsWith("/")) {
        QFileInfo packageAbsolutePath(tempPath);
        tempPath = packageAbsolutePath.absoluteFilePath();  // 获取绝对路径
        qInfo() << "get AbsolutePath" << packageAbsolutePath.absoluteFilePath();
    }

    // 判断当前文件路径中是否存在空格,如果存在则创建软链接并在之后的安装时使用软链接进行访问.
    if (tempPath.contains(" ")) {
        QApt::DebFile p(tempPath);
        if (p.isValid()) {
            tempPath = SymbolicLink(tempPath, p.packageName());
            qWarning() << "PackagesManager:"
                       << "There are spaces in the path, add a soft link" << tempPath;
        }
    }
    return tempPath;
}

Pkg::DependsPair PackagesManager::getPackageDependsDetail(const int index)
{
    if (index < m_packageMd5.size() && index >= 0)
        return m_dependsPackages.value(m_packageMd5[index]);
    return {};
}

/**
 * @brief PackagesManager::appendNoThread
 * @param packages
 * @param allPackageSize
 */
void PackagesManager::appendNoThread(const QStringList &packages, int allPackageSize)
{
    for (QString debPackage : packages) {  // 通过循环添加所有的包

        // 处理包不在本地的情况。
        if (!dealInvalidPackage(debPackage))
            continue;
        QString debPkg = debPackage;
        // 处理package文件路径相关问题
        debPackage = dealPackagePath(debPackage);

        // 检测到是ddim文件
        if (debPackage.endsWith(".ddim")) {
            Q_EMIT signalAppendFailMessage(Pkg::PackageNotDdim);
            continue;
        }

        QApt::DebFile pkgFile(debPackage);
        // 判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            Q_EMIT signalAppendFailMessage(Pkg::PackageInvalid);
            continue;
        }
        // 获取当前文件的md5的值,防止重复添加
        // 在checkInvalid中已经获取过md5,避免2次获取影响性能
        QByteArray md5 = m_allPackages.value(debPkg);
        if (md5.isEmpty())
            md5 = pkgFile.md5Sum();

        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            // 处理重复文件
            Q_EMIT signalAppendFailMessage(Pkg::PackageAlreadyExists);
            continue;
        }
        // 可以添加,发送添加信号

        // 管理最近文件列表
        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(debPackage, data);

        addPackage(m_validPackageCount, debPackage, md5);
    }

    // 所有包都添加结束.
    if (1 == allPackageSize) {
        emit signalAppendFinished(m_packageMd5);  // 添加一个包时 发送添加结束信号,启用安装按钮
    }
}

/**
 * @brief PackagesManager::refreshPage 根据添加包的情况 刷新页面
 * @param validPkgCount 此次添加的包的数量（一次拖入或者打开【可能是多个包】【此处只是预计能够添加到程序中的包的数量】）
 */
void PackagesManager::refreshPage(int validPkgCount)
{
    // 获取当前已经添加到程序中的包的数量
    int packageCount = m_preparedPackages.size();
    // if (1 == packageCount) { //当前程序中只添加了一个包
    //     if (1 == validPkgCount) { //此次只有一个包将会被添加的程序中
    //         emit signalRefreshSinglePage();   //刷新单包安装界面

    //     } else if (validPkgCount > 1) { //当前程序中值添加了一个包，但是这次有不止一个包将会被添加到程序中
    //         emit signalSingle2MultiPage();     //刷新批量安装界面
    //         emit signalAppendStart();          //开始批量添加
    //     }
    // } else if (2 == packageCount) {
    //     //当前程序中已经添加了两个包
    //     //1.第一次是添加了一个包，第二次又添加了多于一个包
    //     emit signalSingle2MultiPage();        //刷新批量安装界面
    //     emit signalAppendStart();             //发送批量添加信号
    // } else {
    //     //此时批量安装界面已经刷新过。如果再添加，就只刷新model
    //     emit signalRefreshMultiPage();
    //     emit signalAppendStart();
    // }

    Q_EMIT signalPackageCountChanged(packageCount);
    // If current first append and only one package, will append directly.
    if (1 != packageCount || 1 != validPkgCount) {
        Q_EMIT signalAppendStart();
    }
}

/**
 * @brief PackagesManager::slotAppendPackageFinished 此次添加已经结束
 */
void PackagesManager::slotAppendPackageFinished()
{
    // 告诉前端，此次添加已经结束
    // 向model传递 md5
    emit signalAppendFinished(m_packageMd5);
}

void PackagesManager::addPackage(int validPkgCount, const QString &packagePath, const QByteArray &packageMd5Sum)
{
    // 预先校验包是否有效或是否重复
    DebFile currentDebfile(packagePath);
    if (!currentDebfile.isValid() || m_appendedPackagesMd5.contains(packageMd5Sum)) {
        return;
    }

    // 加入md5集合
    m_appendedPackagesMd5 << packageMd5Sum;

    // 使用依赖图计算安装顺序
    auto currentDebDepends = currentDebfile.depends();
    m_dependGraph.addNode(packagePath, packageMd5Sum, currentDebfile.packageName(), currentDebDepends);  // 添加图节点
    auto installQueue = m_dependGraph.getBestInstallQueue();  // 输出最佳安装顺序
    m_preparedPackages = installQueue.first;
    m_packageMd5 = installQueue.second;
    int indexRow = 0;
    for (; indexRow != m_packageMd5.size(); ++indexRow) {
        if (m_packageMd5[indexRow] == packageMd5Sum) {
            break;
        }
    }
    if (indexRow == m_packageMd5.size()) {  // error
        return;
    }

    // 需要在此之前刷新出正确的安装顺序
    getPackageDependsStatus(indexRow);  // 刷新当前添加包的依赖
    refreshPage(validPkgCount);         // 添加后，根据添加的状态刷新界面
}

QList<QString> PackagesManager::getAllDepends(const QList<DependencyItem> &depends, const QString &architecture)
{
    // 检索当前包的所有依赖
    for (const auto &list : depends) {
        for (auto info : list) {  // 获取当前包的依赖
            QList<QString> dList = getAllDepends(info.packageName(), architecture);
            m_allDependsList << info.packageName() << dList;  // 存储所有依赖
        }
    }
    m_allDependsList = m_allDependsList.toSet().values();

    return m_allDependsList;
}

QList<QString> PackagesManager::getAllDepends(const QString &packageName, const QString &architecture)
{
    QList<QString> dDepends;  // 存储依赖的依赖
    dDepends << packageName;
    // 对安装包依赖的依赖进行检索
    Package *package = packageWithArch(packageName, architecture);
    if (package) {
        for (const auto &list : package->depends()) {
            for (auto info : list) {
                // 若列表中已存在该依赖，直接跳过
                static QSet<QString> set;
                if (set.contains(info.packageName()))
                    continue;
                set << info.packageName();
                // 将依赖存入列表并进行下一层依赖检索
                dDepends << info.packageName() << getAllDepends(info.packageName(), architecture);
            }
        }
    }
    return dDepends;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const QList<DependencyItem> &depends)
{
    // 只有单包，认为首次进入
    if (choosed_set.size() <= 1) {
        m_loopErrorDeepends.clear();
    }

    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();
    QList<Pkg::DependInfo> break_list;
    QList<Pkg::DependInfo> available_list;
    for (const auto &candicate_list : depends) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
        dependsStatus.maxEq(r);
        if (!m_dinfo.packageName.isEmpty()) {
            if (r.isBreak()) {
                break_list.append(m_dinfo);
            } else if (r.isAvailable()) {
                available_list.append(m_dinfo);
            }
        }
    }

    m_pair.first.append(available_list);
    m_pair.second.append(break_list);
    m_dependsPackages.insert(m_currentPkgMd5, m_pair);
    return dependsStatus;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyItem &candicate)
{
    PackageDependsStatus dependsStatus = PackageDependsStatus::_break(QString());

    for (const auto &info : candicate) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, info);
        dependsStatus.minEq(r);

        // 空包名表示只返回 ok
        if (!r.package.isEmpty() && !m_loopErrorDeepends.contains(r.package)) {
            m_loopErrorDeepends.insert(r.package, r.status);
        }

        // 安装包存在或依赖关系且当前依赖状态不能直接满足，筛选依赖关系最优的选项
        if (!m_unCheckedOrDepends.isEmpty() && DebListModel::DependsOk != r.status) {
            for (auto orDepends : m_unCheckedOrDepends) {  // 遍历或依赖组，检测当前依赖是否存在或依赖关系
                if (orDepends.contains(info.packageName())) {
                    m_unCheckedOrDepends.removeOne(orDepends);
                    m_checkedOrDependsStatus.insert(info.packageName(), r);
                    auto depends = orDepends;
                    depends.removeOne(info.packageName());  // 将当前依赖从或依赖中删除，检测或依赖中剩余依赖状态
                    qInfo() << depends << orDepends;
                    for (auto otherDepend : depends) {
                        // 避免检测过的或依赖重复检测
                        PackageDependsStatus status;
                        if (m_checkedOrDependsStatus.contains(otherDepend)) {
                            status = m_checkedOrDependsStatus[otherDepend];
                        } else {
                            // 虚拟或包共用，区分判断
                            if (m_dependsInfo.contains(otherDepend)) {
                                DependencyInfo dependencyInfo = m_dependsInfo.value(otherDepend);
                                if (dependencyInfo.packageName() == otherDepend) {
                                    status = checkDependsPackageStatus(choosed_set, architecture, dependencyInfo);
                                } else {
                                    // 依赖名和包名不同，为虚包依赖
                                    status = checkDependsPackageStatus(choosed_set, architecture, dependencyInfo, otherDepend);
                                }
                            } else {
                                // 虚包使用或包判断
                                status = checkDependsPackageStatus(
                                    choosed_set, architecture, m_dependsInfo.find(info.packageName()).value(), otherDepend);
                            }

                            m_checkedOrDependsStatus.insert(otherDepend, status);

                            // 空包名表示只返回 ok
                            if (!status.package.isEmpty() && !m_loopErrorDeepends.contains(status.package)) {
                                m_loopErrorDeepends.insert(status.package, status.status);
                            }
                        }
                        qInfo() << qPrintable("Orpackage depends") << status.status;
                        if (status.isBreak())  // 若剩余依赖中存在状态不为break，则说明依赖关系满足
                            continue;
                        dependsStatus.minEq(status);

                        if (DebListModel::DependsOk == dependsStatus.status) {
                            qDebug() << QString("Select or package %1 for %2.").arg(otherDepend).arg(info.packageName());
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if (dependsStatus.status == DebListModel::DependsOk) {
            break;
        }
    }
    return dependsStatus;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyInfo &dependencyInfo,
                                                                      const QString &providesName)
{
    m_dinfo.packageName.clear();
    m_dinfo.version.clear();
    const QString package_name = providesName.isEmpty() ? dependencyInfo.packageName() : providesName;
    QString realArch = architecture;

    // 对 wine 应用特殊处理，wine包依赖升级混用i386/amd64，不使用主包的架构
    if (package_name.contains(g_TagDeepinWine) && realArch == g_Tagi386) {
        realArch.clear();
    }

    Package *package = packageWithArch(package_name, realArch, dependencyInfo.multiArchAnnotation());

    if (!package) {
        qWarning() << "PackagesManager:"
                   << "depends break because package" << package_name << "not available";
        isDependsExists = true;
        m_dinfo.packageName = package_name + ":" + realArch;
        m_dinfo.version = dependencyInfo.packageVersion();
        return PackageDependsStatus::_break(package_name);
    }

    // 虚拟包版本号处理步骤
    QString pkgRealVer = package->version();
    bool isVirtualPackage = false;

#ifdef ENABLE_VIRTUAL_PACKAGE_ENHANCE
    if (package->name() != package_name) {
        auto pkgMap = package->providesListEnhance();
        auto iter = pkgMap.find(package_name);
        if (iter != pkgMap.end()) {
            pkgRealVer = *iter;
            isVirtualPackage = true;
        }
    }
#endif

    const RelationType relation = dependencyInfo.relationType();
    QString installedVersion = package->installedVersion();

    if (!installedVersion.isEmpty()) {
        if (isVirtualPackage) {
            installedVersion = pkgRealVer;
        }

        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation)) {
            return PackageDependsStatus::ok();
        } else {
            const QString &mirror_version = package->availableVersion();
            if (mirror_version != installedVersion) {
                const auto mirror_result = Package::compareVersion(mirror_version, dependencyInfo.packageVersion());
                if (dependencyVersionMatch(mirror_result, relation)) {
                    qInfo() << "PackagesManager:"
                            << "availble by upgrade package" << package->name() + ":" + package->architecture() << "from"
                            << installedVersion << "to" << mirror_version;
                    // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
                    m_dinfo.packageName = package_name + ":" + package->architecture();
                    m_dinfo.version = package->availableVersion();
                    return PackageDependsStatus::available(package->name());
                }
            }

            qWarning() << "PackagesManager:"
                       << "depends break by" << package->name() << package->architecture() << dependencyInfo.packageVersion();
            qWarning() << "PackagesManager:"
                       << "installed version not match" << installedVersion;
            m_dinfo.packageName = package_name + ":" + package->architecture();
            m_dinfo.version = dependencyInfo.packageVersion();
            return PackageDependsStatus::_break(package->name());
        }
    } else {
        const int result = Package::compareVersion(pkgRealVer, dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation)) {
            qWarning() << "PackagesManager:"
                       << "depends break by" << package->name() << package->architecture() << dependencyInfo.packageVersion();
            qWarning() << "PackagesManager:"
                       << "available version not match" << package->version();
            m_dinfo.packageName = package_name + ":" + package->architecture();
            m_dinfo.version = dependencyInfo.packageVersion();
            return PackageDependsStatus::_break(package->name());
        }

        // is that already choosed?
        if (choosed_set.contains(package->name())) {
            // 已有记录，返回之前排查的结果，而不是直接返回 Ok , 当前包名可能为虚包。
            if (m_loopErrorDeepends.contains(package->name())) {
                return PackageDependsStatus(m_loopErrorDeepends.value(package->name()), package->name());
            }

            return PackageDependsStatus::ok();
        }

        // check arch conflicts
        if (package->multiArchType() == MultiArchSame) {
            Backend *backend = PackageAnalyzer::instance().backendPtr();
            for (const auto &arch : backend->architectures()) {
                if (arch == package->architecture())
                    continue;

                Package *otherArchPackage = backend->package(package->name() + ":" + arch);
                if (otherArchPackage && otherArchPackage->isInstalled()) {
                    isDependsExists = true;  // 依赖冲突不属于依赖缺失
                    qWarning() << "PackagesManager:"
                               << "multiple architecture installed: " << package->name() << package->version()
                               << package->architecture() << "but now need" << otherArchPackage->name()
                               << otherArchPackage->version() << otherArchPackage->architecture() << isDependsExists;
                    m_brokenDepend = package->name() + ":" + package->architecture();
                    return PackageDependsStatus::available(package->name() + ":" + package->architecture());
                }
            }
        }
        // let's check conflicts
        if (!isConflictSatisfy(realArch, package).is_ok()) {
            Backend *backend = PackageAnalyzer::instance().backendPtr();
            for (auto *availablePackage : backend->availablePackages()) {
                if (!availablePackage->providesList().contains(package->name())) {
                    availablePackage = nullptr;
                    continue;
                }

                // is that already provide by another package?
                if (availablePackage->isInstalled()) {
                    qInfo() << "PackagesManager:"
                            << "find a exist provider: " << availablePackage->name();
                    availablePackage = nullptr;
                    return PackageDependsStatus::ok();
                }

                // provider is ok, switch to provider.
                if (isConflictSatisfy(realArch, availablePackage).is_ok()) {
                    qInfo() << "PackagesManager:"
                            << "switch to depends a new provider: " << availablePackage->name();
                    choosed_set << availablePackage->name();
                    availablePackage = nullptr;
                    return PackageDependsStatus::ok();
                }
                availablePackage = nullptr;
            }

            qWarning() << "PackagesManager:"
                       << "providers not found, still break: " << package->name();
            m_dinfo.packageName = package_name + ":" + package->architecture();
            m_dinfo.version = dependencyInfo.packageVersion();
            return PackageDependsStatus::_break(package->name());
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << package->name();
        // 判断并获取依赖的或依赖关系
        getPackageOrDepends(package->name(), package->architecture(), false);
        const auto dependsStatus = checkDependsPackageStatus(choosed_set, package->architecture(), package->depends());
        if (dependsStatus.isBreak()) {
            choosed_set.remove(package->name());
            qWarning() << "PackagesManager:"
                       << "depends break by direct depends" << package->name() << package->architecture() << dependsStatus.package
                       << isDependsExists;
            if (!isDependsExists) {
                m_dinfo.packageName = package_name + ":" + package->architecture();
                m_dinfo.version = dependencyInfo.packageVersion();
            } else {
                m_dinfo.packageName = "";
                m_dinfo.version = "";
            }
            return PackageDependsStatus::_break(package->name());
        }

        qInfo() << "PackagesManager:"
                << "Check finished for package" << package->name();
        // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
        m_dinfo.packageName = package_name;
        m_dinfo.version = package->availableVersion();
        return PackageDependsStatus::available(package->name());
    }
}

/**
 * @brief 检测通过包名 \a packageName 获取的软件包 \a package 架构是否支持建议架构 \a suggestArch 。
 *      判断是否指定安装包架构类型，未指定的依赖包将按照安装包架构进行判断，注意，部分软件包提供多架构支持，
 *      例如部分 amd64 软件包同样提供 i386 支持。
 *
 * @bug https://pms.uniontech.com/bug-view-220019.html
 *
 * @note 此函数期望查找包架构能满足架构依赖，参考 apt 实际行为处理，后续底层改造为 apt-pkg 后，建议重构为调用 apt-pkg 借口。
 *      1. 通过命令行指令 `apt-cache showpkg [package name]` 在字段 `Provides` 中可以取得软件包支持的包别名，
 *         `Provides` 会展示软件包提供的包名列表，这些包名会包含支持的虚包、不同的架构等;
 *      2. 部分软件包架构标注 all/any , 但实际并未提供多架构支持，通过 multiArchType() 类型过滤;
 *      3. 部分软件包虽然架构未标注 all/any ，但同样提供其他架构的支持，多出现于 amd64/i386 架构软件包互补;
 *      4. 如果传入的建议架构 \a suggestArch 为空或支持多架构，同样允许此类架构支持。
 */
bool PackagesManager::checkPackageArchValid(const QApt::Package *package, const QString &packageName, const QString &suggestArch)
{
    QString resloveArch = suggestArch;
    resloveArch.remove(':');
    if (!package) {
        // 不打印信息，由后续错误信息提示
        return false;
    } else if (resloveArch.isEmpty() || (package->architecture() == resloveArch) || archMultiSupport(resloveArch)) {
        // 部分传入包名写携带了版本，若版本一致，则无需继续判断
        // 建议架构为多架构支持，则同样允许安装
        return true;
    }

    QApt::MultiArchType multiType = package->multiArchType();
    QString pkgArch = package->architecture();
    // 判断传入包是否符合多架构兼容校验
    bool archMatch = isArchMatches(resloveArch, pkgArch, multiType);
    // 部分软件包虽然标识 "all" "any", 但仍需进一步判断包支持类型是否为多架构支持。
    bool archTypeMatch = bool(InvalidMultiArchType != multiType);

    bool archIsValidRet = false;
    if (archMatch && archTypeMatch) {
        archIsValidRet = true;
    } else if (archMatch /* && !archTypeMatch */) {
        // 软件包标识 "all" "any"，但不支持多架构
        Backend *backend = PackageAnalyzer::instance().backendPtr();
        if (!backend) {
            return false;
        }

        // 如果建议架构是当前架构，同样认为支持
        archIsValidRet = bool(resloveArch == backend->nativeArchitecture());
    } else if (/* !archMatch && */ archTypeMatch) {
        // 部分软件包虽然架构和建议架构不同，但软件包提供多架构支持，多为 amd64/i386 相互支持
        QStringList providesList = package->providesList();

#if 0
        // TODO: 改造 QApt 以对 providesList 同样检测架构
        Backend *backend = PackageAnalyzer::instance().backendPtr();
        if (!backend) {
            return false;
        }

        // 判断建议架构是否为当前系统架构，当前系统架构默认无后缀，同时若应用已包含架构名，则不再插入
        bool isNativeArch = bool(resloveArch == backend->nativeArchitecture());
        QString findPackage = (isNativeArch || packageName.contains(":"))
                ? packageName : (packageName + ":" + resloveArch);
#else
        // QApt 返回 providesList 不带 arch 后缀, 移除查找包架构后缀
        // providesList 列表中含当前包名的则认为满足多架构支持
        // i386 包一般不提供 amd64 架构支持，而 amd64 经常提供 i386
        QString findPackage = packageName;
        findPackage.remove(QRegExp(":[^:]*$"));
#endif

        // 查找当前软件包支持的包名中是否包含对应架构软件包
        if (providesList.contains(findPackage)) {
            qInfo() << QString("Package %1 %2 provides %3, all provides: ")
                           .arg(package->name())
                           .arg(package->architecture())
                           .arg(findPackage)
                    << providesList;
            archIsValidRet = true;
        }
    } else /* !archTypeMatch && !archMatch */ {
        // 等同 archIsValid = false;
    }

    if (archIsValidRet) {
        qInfo() << QString("Auto detect multi arch package %1 (arch:%2, ver:%3), suggestArch: %4 multiArchType: %5")
                       .arg(packageName)
                       .arg(package->architecture())
                       .arg(package->version())
                       .arg(resloveArch)
                       .arg(package->multiArchType());
    } else {
        qWarning()
            << QString("Find package %1 (arch:%2, ver:%3)").arg(packageName).arg(package->architecture()).arg(package->version());
        qWarning() << QString("but not compitable with %1, multiArchType: %2(%3)")
                          .arg(resloveArch)
                          .arg(package->multiArchTypeString())
                          .arg(package->multiArchType());
    }

    return archIsValidRet;
}

Package *PackagesManager::packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation)
{
    Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return nullptr;
    }

    QString suggestArch = resolvMultiArchAnnotation(annotation, sysArch);
    Package *package = backend->package(packageName + suggestArch);

    // change: 按照当前支持的CPU架构进行打包。取消对deepin-wine的特殊处理
    if (!package)
        package = backend->package(packageName);

    if (checkPackageArchValid(package, packageName, suggestArch))
        return package;
    for (QString arch : backend->architectures()) {
        package = backend->package(packageName + ":" + arch);
        if (checkPackageArchValid(package, packageName, suggestArch))
            return package;
    }

    // check virtual package providers
    for (auto *virtualPackage : backend->availablePackages()) {
        if (virtualPackage->name() != packageName && virtualPackage->providesList().contains(packageName)) {
            return packageWithArch(virtualPackage->name(), sysArch, annotation);
        }
    }

    return nullptr;
}

/**
 * @brief PackagesManager::SymbolicLink 创建软连接
 * @param previousName 原始路径
 * @param packageName 软件包的包名
 * @return 软链接的路径
 */
QString PackagesManager::SymbolicLink(const QString &previousName, const QString &packageName)
{
    if (!mkTempDir()) {  // 如果创建临时目录失败,则提示
        qWarning() << "PackagesManager:"
                   << "Failed to create temporary folder";
        return previousName;
    }
    // 成功则开始创建
    return link(previousName, packageName);
}

/**
 * @brief PackagesManager::mkTempDir 创建软链接存放的临时目录
 * @return 创建目录的结果
 */
bool PackagesManager::mkTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (!tempPath.exists())  // 如果临时目录不存在则返回创建结果
        return tempPath.mkdir(m_tempLinkDir);
    else
        return true;
}

/**
 * @brief PackagesManager::link 创建软链接
 * @param linkPath              原文件的路径
 * @param packageName           包的packageName
 * @return                      软链接之后的路径
 */
QString PackagesManager::link(const QString &linkPath, const QString &packageName)
{
    QFile linkDeb(linkPath);

    // 创建软链接时，如果当前临时目录中存在同名文件，即同一个名字的应用，考虑到版本可能有变化，将后续添加进入的包重命名为{packageName}_i
    // 删除后再次添加会在临时文件的后面添加_1,此问题不影响安装。如果有问题，后续再行修改。
    int count = 1;
    QString tempName = packageName;

    // 命名创建的软链接文件
    while (true) {
        QFile tempLinkPath(m_tempLinkDir + tempName);
        // 对已经存在重名文件的处理
        if (tempLinkPath.exists()) {  // 命名方式为在包名后+"_i" PS:i 为当前重复的数字,无实际意义,只是为了区别不同的包
            tempName = packageName + "_" + QString::number(count);
            qWarning() << "PackagesManager:"
                       << "A file with the same name exists in the current temporary directory,"
                          "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            break;
        }
    }
    // 创建软链接
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName)) {
        return m_tempLinkDir + tempName;  // 创建成功,返回创建的软链接的路径.
    } else {
        // 创建失败,直接返回路径
        qWarning() << "PackagesManager:"
                   << "Failed to create Symbolick link error.";
        return linkPath;
    }
}

/**
 * @brief PackagesManager::rmTempDir 删除存放软链接的临时目录
 * @return 删除临时目录的结果
 * PS: 移动创建临时目录 创建软链接的函数到 AddPackageThread中
 *
 */
bool PackagesManager::rmTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (tempPath.exists())  // 如果临时目录存在，则删除临时目录
        return tempPath.removeRecursively();
    else
        return true;  // 临时目录不存在，返回删除成功
}

QString PackagesManager::package(const int index) const
{
    return m_preparedPackages[index];
}

void PackagesManager::getBlackApplications()
{
    QFile blackListFile(BLACKFILE);
    if (blackListFile.exists()) {
        blackListFile.open(QFile::ReadOnly);
        QString blackApplications = blackListFile.readAll();
        blackApplications.replace(" ", "");
        blackApplications = blackApplications.replace("\n", "");
        m_blackApplicationList = blackApplications.split(",");
        blackListFile.close();
        return;
    }
    qWarning() << "Black File not Found";
}

/**
   @brief 判断 \a debFile 的依赖列表 \a dependList 中 wine 依赖是否已安装，同时剔除非 wine 依赖。
        \a dependInfoMap 提供详细的依赖项架构、版本信息。
   @note Wine安装包升级无架构信息(i386 amd64混用)，仅使用安装包信息判断无法覆盖所有场景，
        升降级存在问题。依赖包 deepin-wine-helper 由 i386 变更为 amd64。
 */
void PackagesManager::filterNeedInstallWinePackage(QStringList &dependList,
                                                   const DebFile &debFile,
                                                   const QHash<QString, DependencyInfo> &dependInfoMap)
{
    // 额外判断wine依赖是否已安装，同时剔除非wine依赖
    auto removedIter =
        std::remove_if(dependList.begin(), dependList.end(), [&debFile, &dependInfoMap, this](const QString &eachDepend) {
            if (!eachDepend.contains("deepin-wine")) {
                return true;
            }

            // 使用软件包架构查询
            Package *package = nullptr;
            const DependencyInfo &info = dependInfoMap.value(eachDepend);
            if (Q_UNLIKELY(info.multiArchAnnotation().isEmpty())) {
                qInfo() << QString("Wine package %1 without architecture info").arg(eachDepend);

                // 无依赖架构信息，使用默认包
                Backend *backend = PackageAnalyzer::instance().backendPtr();
                if (!backend) {
                    qWarning() << qPrintable("Failed to load libqapt backend");
                    return false;
                }

                // 使用默认无版本查找，由apt判断，优先版本高的架构
                package = backend->package(eachDepend);
                if (!package) {
                    package = packageWithArch(eachDepend, debFile.architecture());
                }
            } else {
                // 有依赖架构信息，判断对应架构是否安装
                package = packageWithArch(eachDepend, info.multiArchAnnotation());
            }

            if (!package) {
                qWarning() << QString("Wine package %1 not found!").arg(eachDepend);
                return false;
            }
            qInfo() << QString("Wine package %1 detect current package: %2, arch: %3, version: %4")
                           .arg(eachDepend)
                           .arg(package->name())
                           .arg(package->architecture())
                           .arg(package->version());

            // 检查安装包版本是否需要更新。
            QString currentInstall = package->installedVersion();
            if (currentInstall.isEmpty()) {
                // 未安装软件包，需进行安装
                qInfo() << QString("Wine package %1 not installed").arg(eachDepend);
                return false;
            } else {
                // 判断是否需要更新，满足需求则不处理
                const auto cmpVersion = Package::compareVersion(currentInstall, info.packageVersion());
                bool notNeedUpdate = dependencyVersionMatch(cmpVersion, info.relationType());

                qInfo() << QString("Wine package %1 check version. current installed: %2 require: %3 not need update: %4")
                               .arg(eachDepend)
                               .arg(currentInstall)
                               .arg(info.packageVersion())
                               .arg(notNeedUpdate);
                return notNeedUpdate;
            }
        });

    dependList.erase(removedIter, dependList.end());
}

bool PackagesManager::isBlackApplication(const QString &applicationName)
{
    if (m_blackApplicationList.contains(applicationName))
        return true;
    return false;
}

PackagesManager::~PackagesManager()
{
    // 删除 临时目录，会尝试四次，四次失败后退出。
    int rmTempDirCount = 0;
    while (true) {
        if (rmTempDir())  // 删除成功
            break;
        qWarning() << "PackagesManager:"
                   << "Failed to delete temporary folder， Current attempts:" << rmTempDirCount << "/3";
        if (rmTempDirCount > 3) {  // 删除三次仍然失败则警告 不过每次重启都会删除临时目录
            qWarning() << "PackagesManager:"
                       << "Failed to delete temporary folder, Exit application";
            break;
        }
        rmTempDirCount++;
    }
    delete m_installWineThread;
    delete m_pAddPackageThread;
}
