// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPACKAGELISTMODEL_H
#define UABPACKAGELISTMODEL_H

#include <QObject>

#include "uab_package.h"
#include "uab_process_controller.h"
#include "model/abstract_package_list_model.h"

class QFileSystemWatcher;

namespace Uab {

class UabPackageListModel : public AbstractPackageListModel
{
    Q_OBJECT
public:
    explicit UabPackageListModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_SLOT void slotAppendPackage(const QStringList &packageList) override;
    void removePackage(int index) override;
    [[nodiscard]] QString checkPackageValid(const QString &packagePath) override;

    [[nodiscard]] Pkg::PackageInstallStatus checkInstallStatus(const QString &packagePath) override;
    [[nodiscard]] Pkg::DependsStatus checkDependsStatus(const QString &packagePath) override;
    [[nodiscard]] QStringList getPackageInfo(const QString &packagePath) override;
    [[nodiscard]] QString lastProcessError() override;
    [[nodiscard]] bool containsSignatureFailed() const override;

    Q_SLOT bool slotInstallPackages() override;
    Q_SLOT bool slotUninstallPackage(int index) override;

    void reset() override;
    void resetInstallStatus() override;

private:
    bool installNextUab();

    Q_SLOT void slotBackendProgressChanged(float progress);
    Q_SLOT void slotBackendProcessFinished(bool success);

    void setCurrentOperation(Pkg::PackageOperationStatus s);
    bool checkIndexValid(int index) const;
    UabPackage::Ptr preCheckPackage(const QString &packagePath);
    bool packageExists(const UabPackage::Ptr &uabPtr) const;
    bool linglongExists();

    Q_SLOT void slotFileChanged(const QString &filePath);

private:
    int m_operatingIndex{-1};
    QList<UabPackage::Ptr> m_uabPkgList;
    UabProcessController *m_processor{nullptr};

    QStringList m_delayAppendPackages;  // wait for backend inited.

    QFileSystemWatcher *m_fileWatcher{nullptr};
};

}  // namespace Uab

#endif  // UABPACKAGELISTMODEL_H
