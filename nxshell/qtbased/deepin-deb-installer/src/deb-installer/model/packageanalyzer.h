// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGEANALYZER_H
#define PACKAGEANALYZER_H

#include "model/packageselectmodel.h"

#include <QObject>
#include <atomic>

namespace QApt {
class Backend;
class Package;
}  // namespace QApt

class PackageAnalyzer : public QObject
{
    Q_OBJECT
public:
    // 包安装状态
    enum PackageInstallStatus {
        NotInstalled,             // 当前包没有被安装
        InstalledSameVersion,     // 当前已经安装过相同的版本
        InstalledEarlierVersion,  // 当前已经安装过较早的版本
        InstalledLaterVersion     // 当前已经安装过更新的版本
    };

    static PackageAnalyzer &instance();

    // 异步信号

    // 告知外部的UI已经退出了
    void setUiExit();

    // 初始化阶段

    void initBackend();
    bool isBackendReady();
    QApt::Backend *backendPtr();

    // 选择阶段

    // 当前APT支持的架构
    bool supportArch(const QString &arch) const { return archs.contains(arch); }
    inline QStringList supportArchList() const { return archs; }

    // 软件包安装状态，first:安装状态，secend:已安装版本（当状态不为NotInstalled时有效）
    QPair<PackageInstallStatus, QString> packageInstallStatus(const DebIr &ir) const;

    // 分析包结构

    // 启动/停止包分析，内部会根据分析进度对外发送信号
    void startPkgAnalyze(int total);
    void stopPkgAnalyze();

    // 参数：文件信息，md5码过滤集，应用名，是否去除架构不满足项，是否去除已安装或安装了高版本的项
    QList<DebIr> analyzeDebFiles(const QFileInfoList &infos,
                                 QSet<QByteArray> *md5s,
                                 QStringList *appNames,
                                 bool excludeArchNotMatched,
                                 bool excludeInstalledOrLaterVersion);

    // 提取未安装依赖，传入包名和对应的架构，返回未安装的依赖列表
    QList<QApt::DependencyItem> debDependNotInstalled(const DebIr &ir) const;

    // 依赖是否就绪
    bool dependIsReady(const QApt::DependencyItem &depend) const;

    // 虚拟包是否已安装
    bool virtualPackageIsExist(const QString &virtualPackageName) const;

    // 版本号是否匹配
    // rhs是否以relationType的方式匹配lhs
    bool versionMatched(const QString &lhs, const QString &rhs, QApt::RelationType relationType) const;

    // 抽取需要的依赖包
    void chooseDebFromDepend(QList<DebIr> *result,
                             QSet<QByteArray> *md5s,
                             const QList<QApt::DependencyItem> &depends,
                             const QList<DebIr> &debIrs) const;

    // 返回最佳安装顺序
    QList<DebIr> bestInstallQueue(const QList<DebIr> &installIrs, const QList<DebIr> &dependIrs);

signals:
    // 正在初始化后端，true：启动，false：完成
    void runBackend(bool inProcess);

    // 正在分析包情况
    void runAnalyzeDeb(bool inProcess, int currentRote, int pkgCount);

private:
    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation) const;
    QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, int multiArchType) const;

    explicit PackageAnalyzer(QObject *parent = nullptr);
    PackageAnalyzer(const PackageAnalyzer &) = delete;
    PackageAnalyzer operator=(const PackageAnalyzer &) = delete;

    QStringList archs;
    QApt::Backend *backend = nullptr;
    std::atomic_bool backendInInit;
    std::atomic_bool inPkgAnalyze;
    int pkgWaitToAnalyzeTotal = -1;
    int alreadyAnalyzed = 0;
    std::atomic_bool uiExited;
};

Q_DECLARE_METATYPE(QList<DebIr>);

#endif  // PACKAGEANALYZER_H
