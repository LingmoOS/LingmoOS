// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packageselectmodel.h"
#include "model/packageanalyzer.h"

#include <QStandardItemModel>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>

#include <QApt/DebFile>

PackageSelectModel::PackageSelectModel(QObject *parent)
    : QObject(parent)
    , model(new QStandardItemModel)
{
}

QStandardItemModel *PackageSelectModel::viewModel() const
{
    return model;
}

void PackageSelectModel::appendDdimPackages(const QList<DdimSt> &ddims)
{
    for (const auto &ddim : ddims) {
        // 初始化ddim ir包
        DdimIrPackage ddimIrPkg;
        ddimIrPkg.dirPath = ddim.dirPath;
        ddimIrPkg.version = ddim.version;
        ddimIrPkg.token = ddim.token;

        // 逐个转换为IR模式
        // 去重优先级：必装 > 选装 > 依赖
        PackageAnalyzer::instance().startPkgAnalyze(ddim.mustInstallList.size() + ddim.selectList.size() +
                                                    ddim.dependList.size());

        QSet<QByteArray> md5s;
        auto currentMustInstallInfos =
            PackageAnalyzer::instance().analyzeDebFiles(ddim.mustInstallList, &md5s, nullptr, true, true);
        QStringList selectAppNameList = ddim.selectAppNameList;
        auto currentSelectInfos =
            PackageAnalyzer::instance().analyzeDebFiles(ddim.selectList, &md5s, &selectAppNameList, false, false);
        auto currentDependInfos = PackageAnalyzer::instance().analyzeDebFiles(ddim.dependList, &md5s, nullptr, true, false);

        PackageAnalyzer::instance().stopPkgAnalyze();

        for (int i = 0; i != currentSelectInfos.size(); ++i) {
            currentSelectInfos[i].appName = selectAppNameList[i];
        }

        ddimIrPkg.selectInfos = currentSelectInfos;
        ddimIrPkg.dependInfos = currentDependInfos;
        ddimIrPkg.mustInstallInfos = currentMustInstallInfos;

        // 查询是否有相同token的ddim，有相同的执行更新，没有相同的执行新增
        auto ddimIter = std::find_if(ddimIrs.begin(), ddimIrs.end(), [ddimIrPkg](const DdimIrPackage &currentDdimIrPkg) {
            return currentDdimIrPkg.token == ddimIrPkg.token;
        });
        if (ddimIter != ddimIrs.end()) {
            *ddimIter = ddimIrPkg;
        } else {
            ddimIrs.push_back(ddimIrPkg);
        }
    }

    // 汇聚全部数据（依据ddimIrs目前的情况，刷新selectInfos，dependInfos，mustInstallInfos）
    if (collectData()) {
        emit selectInfosChanged(selectInfos);  // 刷新选择界面
    } else {
        emit selectInfosDoNotHaveChange();  // 告知没有必要刷新选择界面
    }
}

QList<DebIr> PackageSelectModel::analyzePackageInstallNeeded(const QList<int> &selectIndexes) const
{
    QList<DebIr> installIrs;
    QList<DebIr> dependIrs;

    // 1.筛选需要安装的可选包
    for (int i : selectIndexes) {
        installIrs.push_back(selectInfos.at(i));
    }

    // 2.汇集可选包和必装包为需要安装的包
    installIrs.append(mustInstallInfos);
    dependIrs = dependInfos;

    // 3.将需要安装的包和本地可用依赖包传入分析工具，获取最佳安装顺序（原则是能不装就不装，确保安装过程最小化）
    PackageAnalyzer::instance().startPkgAnalyze(installIrs.size());
    auto result = PackageAnalyzer::instance().bestInstallQueue(installIrs, dependIrs);
    PackageAnalyzer::instance().stopPkgAnalyze();

    return result;
}

template <typename Func>
void getDebIrs(QList<DebIr> *container, QSet<QString> *md5s, const QList<DdimIrPackage> &ddimIrs, Func &&dataGetter)
{
    for (auto &eachIrPkg : ddimIrs) {
        auto infos = dataGetter(eachIrPkg);
        for (auto &eachInfo : infos) {
            if (!md5s->contains(eachInfo.md5)) {
                md5s->insert(eachInfo.md5);
                container->push_back(eachInfo);
            }
        }
    }
}

bool PackageSelectModel::collectData()
{
    // 1.构建新数据包，由于只需要监控可选项，因此其它两个直接清空
    QList<DebIr> newSelectInfos;
    dependInfos.clear();
    mustInstallInfos.clear();

    // 2.处理ddimIrs的数据，需要依据包的md5进行去重
    QSet<QString> md5s;

    // 2.1汇聚必装包
    getDebIrs(&mustInstallInfos, &md5s, ddimIrs, [](const DdimIrPackage &info) { return info.mustInstallInfos; });

    // 2.2汇聚可选包
    getDebIrs(&newSelectInfos, &md5s, ddimIrs, [](const DdimIrPackage &info) { return info.selectInfos; });

    // 2.3汇聚依赖包
    getDebIrs(&dependInfos, &md5s, ddimIrs, [](const DdimIrPackage &info) { return info.dependInfos; });

    // 3.查看是否存在改变
    bool ret = !(newSelectInfos == selectInfos);

    // 4.清理老数据
    selectInfos = newSelectInfos;

    return ret;
}
