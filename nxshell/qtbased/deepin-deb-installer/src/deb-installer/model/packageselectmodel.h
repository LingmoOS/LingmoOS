// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESELECTMODEL_H
#define PACKAGESELECTMODEL_H

#include <QObject>
#include <QFileInfoList>
#include <QSet>
#include <QApt/DependencyInfo>

class PackageAnalyzer;
class QStandardItemModel;

// 初步解析DDIM的结果
struct DdimSt
{
    bool isAvailable = false;
    QString dirPath;                // ddim所在目录
    QString version;                // ddim版本
    QString token;                  // ddim标识，作为ddim去重的依据
    QStringList selectAppNameList;  // 可选项里的应用名称
    QFileInfoList selectList;       // 可选
    QFileInfoList dependList;       // 依赖
    QFileInfoList mustInstallList;  // 必装
};

// 单个包的基础信息
// TODO：就程序设计来说，需要一个单独生成deb IR的函数
struct DebIr
{
    QString filePath;                     // 完整绝对路径
    QString appName;                      // 应用名
    QString packageName;                  // 包名
    QString architecture;                 // 包支持的架构
    QString version;                      // 包版本
    QString shortDescription;             // 短描述
    QByteArray md5;                       // 包的md5码
    QStringList virtualPackages;          // 提供的虚拟包 QApt似乎不支持读Provides项
    QList<QApt::DependencyItem> depends;  // 完整的包依赖

    bool archMatched;  // 是否与当前架构匹配
    bool isValid;  // 包是否有效（根据以前的老代码，此处如果无效，可以暂时当做未安装处理，由后续的apt安装时进行报错）

    bool operator==(const DebIr &rhs) { return this->md5 == rhs.md5; }
};

// 单个DDIM下的包信息
struct DdimIrPackage
{
    QString dirPath;  // ddim所在目录
    QString version;  // ddim版本
    QString token;    // ddim标识
    QList<DebIr> selectInfos;
    QList<DebIr> dependInfos;
    QList<DebIr> mustInstallInfos;
};

class PackageSelectModel : public QObject
{
    Q_OBJECT

public:
    explicit PackageSelectModel(QObject *parent = nullptr);

    // 添加解析好的ddims
    void appendDdimPackages(const QList<DdimSt> &ddims);

    // 获取最终需要安装的包列表，传入参数为选中的selectInfos的index
    QList<DebIr> analyzePackageInstallNeeded(const QList<int> &selectIndexes) const;

    QList<DebIr> selectData() const { return selectInfos; }

    QList<DebIr> dependData() const { return dependInfos; }

    QList<DebIr> mustInstallData() const { return mustInstallInfos; }

    QStandardItemModel *viewModel() const;  // 先留着，后面性能不够再启用

signals:
    void selectInfosChanged(const QList<DebIr> &infos);  // 刷新选择面板
    void selectInfosDoNotHaveChange();                   // 上一轮执行的appendDdimPackages没有刷新选择面板的必要

private:
    bool collectData();  // 汇聚数据，返回值表示可选项是否存在更新

    // 暂存汇聚好的数据，用于后续安装使用
    QList<DebIr> selectInfos;
    QList<DebIr> dependInfos;
    QList<DebIr> mustInstallInfos;
    QStandardItemModel *model;

    // 解析为IR格式的ddim，需要依据内部的token进行去重
    QList<DdimIrPackage> ddimIrs;
};

#endif  // PACKAGESELECTMODEL_H
