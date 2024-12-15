// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROXY_PACKAGE_LIST_MODEL_H
#define PROXY_PACKAGE_LIST_MODEL_H

#include "abstract_package_list_model.h"

class ProxyPackageListModel : public AbstractPackageListModel
{
    Q_OBJECT

public:
    using ModelPtr = AbstractPackageListModel *;

    explicit ProxyPackageListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_SLOT void slotAppendPackage(const QStringList &packageList) override;
    void removePackage(int index) override;
    QString checkPackageValid(const QString &packagePath) override;

    Pkg::PackageInstallStatus checkInstallStatus(const QString &packagePath) override;
    Pkg::DependsStatus checkDependsStatus(const QString &packagePath) override;
    QStringList getPackageInfo(const QString &packagePath) override;
    QString lastProcessError() override;
    bool containsSignatureFailed() const override;

    Q_SLOT bool slotInstallPackages() override;
    Q_SLOT bool slotUninstallPackage(int index) override;

    void reset() override;
    void resetInstallStatus() override;

    ModelPtr modelFromType(Pkg::PackageType type);
    ModelPtr addModelFromFile(const QString &packagePath);

private:
    bool nextModelInstall();

    ModelPtr addModel(Pkg::PackageType type);
    void connectModel(ModelPtr model);

    QPair<ModelPtr, int> findFromProxyIndex(int proxyIndex) const;
    int proxyIndexFromModel(ModelPtr findModel, int index);

    // signals forwarded through
    Q_SLOT void onSourcePacakgeCountChanged(int count);
    Q_SLOT void onSourceWholeProgressChanged(int progress);
    Q_SLOT void onSourceCurrentProcessPackageIndex(int index);
    Q_SLOT void onSoureWorkerFinished();
    Q_SLOT void onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    int m_procModelIndex{-1};  // current processing model index

    struct ModelInfo
    {
        ModelPtr model{nullptr};
        int count{0};       // cached model item count
        int rightCount{0};  // index counts for current and previous models
    };
    QList<ModelInfo> m_packageModels;  // all package list models (deb/uab)
};

#endif  // PROXY_PACKAGE_LIST_MODEL_H
