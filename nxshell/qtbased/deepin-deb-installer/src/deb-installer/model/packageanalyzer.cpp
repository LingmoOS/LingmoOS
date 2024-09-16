// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packageanalyzer.h"
#include "singleInstallerApplication.h"

#include <QtDebug>
#include <QThread>
#include <QApplication>
#include <QMetaType>

#include <QApt/Backend>
#include <QApt/DebFile>

PackageAnalyzer &PackageAnalyzer::instance()
{
    static PackageAnalyzer analyzer;
    while (analyzer.backendInInit) {
        QThread::msleep(10);
        QApplication::processEvents();
    }
    return analyzer;
}

PackageAnalyzer::PackageAnalyzer(QObject *parent)
    : QObject(parent)
{
    backendInInit = false;
    inPkgAnalyze = false;
    uiExited = false;

    qRegisterMetaType<QList<DebIr>>("QList<DebIr>");
}

void PackageAnalyzer::setUiExit()
{
    uiExited = true;
}

void PackageAnalyzer::initBackend()
{
    if (backend != nullptr) {
        return;
    }

    emit runBackend(true);
    backendInInit = true;

    while (SingleInstallerApplication::BackendIsRunningInit) {
        QThread::msleep(10);
    }

    SingleInstallerApplication::BackendIsRunningInit = true;

    backend = new QApt::Backend;
    bool initSuccess = backend->init();

    SingleInstallerApplication::BackendIsRunningInit = false;

    if (!initSuccess) {
        qFatal("%s", backend->initErrorMessage().toStdString().c_str());
    }

    archs = backend->architectures();
    archs.append("all");
    archs.append("any");

    backendInInit = false;
    emit runBackend(false);
}

bool PackageAnalyzer::isBackendReady()
{
    return backend != nullptr;
}

QApt::Backend *PackageAnalyzer::backendPtr()
{
    return backend;
}

QPair<PackageAnalyzer::PackageInstallStatus, QString> PackageAnalyzer::packageInstallStatus(const DebIr &ir) const
{
    PackageAnalyzer::PackageInstallStatus status;
    QString installedVersion;

    do {
        if (!ir.isValid) {
            status = PackageAnalyzer::NotInstalled;
            break;
        }

        QApt::Package *package = packageWithArch(ir.packageName, ir.architecture, "");

        if (package == nullptr) {
            status = PackageAnalyzer::NotInstalled;
            break;
        }

        installedVersion = package->installedVersion();
        if (installedVersion.isEmpty()) {
            status = PackageAnalyzer::NotInstalled;
            break;
        }

        int result = QApt::Package::compareVersion(ir.version, installedVersion);
        if (result == 0) {
            status = PackageAnalyzer::InstalledSameVersion;
        } else if (result < 0) {
            status = PackageAnalyzer::InstalledLaterVersion;
        } else {
            status = PackageAnalyzer::InstalledEarlierVersion;
        }
    } while (0);

    return {status, installedVersion};
}

QApt::Package *
PackageAnalyzer::packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation) const
{
    QApt::Package *package =
        backend->package(packageName + resolvMultiArchAnnotation(annotation, sysArch, QApt::InvalidMultiArchType));

    if (!package) {
        package = backend->package(packageName);
    }
    if (package) {
        return package;
    }

    for (QString arch : backend->architectures()) {
        package = backend->package(packageName + ":" + arch);
        if (package)
            return package;
    }

    for (auto *virtualPackage : backend->availablePackages()) {
        if (virtualPackage->name() != packageName && virtualPackage->providesList().contains(packageName)) {
            return packageWithArch(virtualPackage->name(), sysArch, annotation);
        }
    }

    return nullptr;
}

QString PackageAnalyzer::resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, int multiArchType) const
{
    if ("native" == annotation || "any" == annotation) {
        return QString();
    }
    if ("all" == annotation) {
        return QString();
    }
    if (multiArchType == QApt::MultiArchForeign) {
        return QString();
    }

    QString arch;
    if (annotation.isEmpty()) {
        arch = debArch;
    } else {
        arch = annotation;
    }

    if (!arch.startsWith(':') && !arch.isEmpty()) {
        return arch.prepend(':');
    } else {
        return arch;
    }
}

bool PackageAnalyzer::virtualPackageIsExist(const QString &virtualPackageName) const
{
    // 由于没法搜索虚拟包，此处只能进行全包遍历
    for (auto *package : backend->availablePackages()) {
        if (package->name() != virtualPackageName && package->providesList().contains(virtualPackageName)) {
            return true;
        }
    }
    return false;
}

bool PackageAnalyzer::versionMatched(const QString &lhs, const QString &rhs, QApt::RelationType relationType) const
{
    if (relationType == QApt::NoOperand) {
        return true;
    }

    int compareResult = QApt::Package::compareVersion(lhs, rhs);
    bool isMatched = false;
    if (compareResult == 0) {  // lhs == rhs
        if (relationType == QApt::LessOrEqual || relationType == QApt::GreaterOrEqual || relationType == QApt::Equals) {
            isMatched = true;
        }
    } else if (compareResult < 0) {  // lhs < rhs
        if (relationType == QApt::GreaterOrEqual || relationType == QApt::GreaterThan || relationType == QApt::NotEqual) {
            isMatched = true;
        }
    } else {  // lhs > rhs
        if (relationType == QApt::LessOrEqual || relationType == QApt::LessThan || relationType == QApt::NotEqual) {
            isMatched = true;
        }
    }
    return isMatched;
}

bool PackageAnalyzer::dependIsReady(const QApt::DependencyItem &depend) const
{
    // 1.每个item内部为或关系，只要有一个满足条件，即可认为该依赖已就绪
    bool isReady = false;
    for (const auto &item : depend) {
        // 2.简单检查item是否安装

        // 2.1获取基本数据
        auto name = item.packageName();
        auto version = item.packageVersion();
        auto type = item.relationType();
        auto arch = item.multiArchAnnotation();

        // 2.2获取包状态
        auto package = packageWithArch(name, arch, "");
        if (package != nullptr && package->isInstalled()) {
            // 如果已安装，则检查版本情况
            auto pkgVersion = package->version();

            if (versionMatched(version, pkgVersion, type)) {
                isReady = true;
                break;
            }
        } else if (virtualPackageIsExist(name)) {  // 3.如果没有安装，则检查其作为虚拟包是否已安装
            isReady = true;
            break;
        }
    }
    return isReady;
}

QList<QApt::DependencyItem> PackageAnalyzer::debDependNotInstalled(const DebIr &ir) const
{
    // 获取依赖项
    auto debDepends = ir.depends;

    // 获取安装状态，已安装的就丢出去
    for (int i = 0; i != debDepends.size(); ++i) {
        if (dependIsReady(debDepends.at(i))) {
            debDepends.removeAt(i);
            --i;
        }
    }

    return debDepends;
}

void PackageAnalyzer::startPkgAnalyze(int total)
{
    if (total > 0) {
        pkgWaitToAnalyzeTotal = total;
        alreadyAnalyzed = 0;
    }
}

void PackageAnalyzer::stopPkgAnalyze()
{
    pkgWaitToAnalyzeTotal = -1;
    alreadyAnalyzed = 0;
    emit runAnalyzeDeb(false, 0, 0);
}

QList<DebIr> PackageAnalyzer::analyzeDebFiles(const QFileInfoList &infos,
                                              QSet<QByteArray> *md5s,
                                              QStringList *appNames,
                                              bool excludeArchNotMatched,
                                              bool excludeInstalledOrLaterVersion)
{
    QList<DebIr> irs;
    QList<int> appNameNeedRemove;
    for (int i = 0; i != infos.size(); ++i) {
        qWarning() << __FUNCTION__ << "analyze deb file:" << infos[i].absoluteFilePath();

        if (uiExited) {
            break;
        }

        if (pkgWaitToAnalyzeTotal > 0) {
            emit runAnalyzeDeb(true, alreadyAnalyzed++, pkgWaitToAnalyzeTotal);
        }

        auto path = infos[i].absoluteFilePath();
        QApt::DebFile deb(path);

        if (!deb.isValid()) {  // 无效包直接去除
            appNameNeedRemove.append(i);
            continue;
        }

        // 如果需要丢掉不匹配的架构
        bool archMatched = supportArch(deb.architecture());
        if (excludeArchNotMatched && !archMatched) {
            appNameNeedRemove.append(i);
            continue;
        }

        // 如果需要丢掉已安装或已安装高版本
        if (excludeInstalledOrLaterVersion) {
            auto pkg = packageWithArch(deb.packageName(), deb.architecture(), "");
            if (pkg != nullptr && pkg->isInstalled()) {
                if (QApt::Package::compareVersion(pkg->version(), deb.version()) >= 0) {
                    appNameNeedRemove.append(i);
                    continue;
                }
            }
        }

        auto packageMd5 = deb.md5Sum();
        if (md5s->contains(packageMd5)) {  // 包已存在，去重
            appNameNeedRemove.append(i);
            continue;
        } else {
            md5s->insert(packageMd5);
        }

        // 生成ir
        DebIr ir;
        ir.filePath = path;
        ir.version = deb.version();
        ir.shortDescription = deb.shortDescription();
        ir.packageName = deb.packageName();
        ir.architecture = deb.architecture();
        ir.archMatched = archMatched;
        ir.md5 = deb.md5Sum();
        ir.isValid = deb.isValid();
        ir.depends = deb.depends();

        // TODO：QApt不支持从deb内部提取提供的虚拟包

        irs.push_back(ir);
    }

    if (appNames != nullptr && !uiExited) {
        for (int i = appNameNeedRemove.size() - 1; i != -1; --i) {
            appNames->removeAt(appNameNeedRemove[i]);
        }
    }

    return irs;
}

void PackageAnalyzer::chooseDebFromDepend(QList<DebIr> *result,
                                          QSet<QByteArray> *md5s,
                                          const QList<QApt::DependencyItem> &depends,
                                          const QList<DebIr> &debIrs) const
{
    for (const auto &depend : depends) {  // 对每一个不满足的依赖项
        bool finded = false;
        for (const auto &item : depend) {  // 对每一个或依赖
            auto name = item.packageName();
            auto version = item.packageVersion();
            auto type = item.relationType();
            for (const auto &ir : debIrs) {  // 对每一个候选包
                if (ir.packageName == name && versionMatched(version, ir.version, type)) {
                    auto depdeps = debDependNotInstalled(ir);  // 依赖包的依赖
                    if (!depdeps.isEmpty()) {  // 有未就绪的依赖，在依赖包集合里面执行递归搜索
                        chooseDebFromDepend(result, md5s, depdeps, debIrs);
                    }
                    if (!md5s->contains(ir.md5)) {
                        md5s->insert(ir.md5);
                        result->push_back(ir);
                    }
                    finded = true;
                    break;
                }
            }
            if (finded) {
                break;
            }
        }
    }
}

QList<DebIr> PackageAnalyzer::bestInstallQueue(const QList<DebIr> &installIrs, const QList<DebIr> &dependIrs)
{
    // TODO：后面重构的时候需要实现安装顺序计算，本轮需求仅实现抽取需要的包

    // 1.检查依赖是否已就绪，将未就绪的项抽取出来
    QList<QApt::DependencyItem> installDeps;  // 记录每一个安装项的依赖情况
    for (const auto &installIr : installIrs) {
        qWarning() << __FUNCTION__ << "analyze deb file:" << installIr.filePath;

        if (uiExited) {
            return QList<DebIr>();
        }

        if (pkgWaitToAnalyzeTotal > 0) {
            emit runAnalyzeDeb(true, alreadyAnalyzed++, pkgWaitToAnalyzeTotal);
        }

        auto notInstallDepends = debDependNotInstalled(installIr);
        if (!notInstallDepends.isEmpty()) {
            installDeps.append(notInstallDepends);
        }
    }

    // 2.从依赖包集合中抽取目前尚未就绪的项
    QList<DebIr> realDepends;
    QSet<QByteArray> md5s;
    chooseDebFromDepend(&realDepends, &md5s, installDeps, dependIrs);

    // 3.融合并返回结果
    return realDepends + installIrs;
}
