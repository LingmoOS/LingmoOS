// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPACKAGE_H
#define UABPACKAGE_H

#include "utils/package_defines.h"
#include "uab_defines.h"

namespace Uab {

class UabPackage
{
public:
    using Ptr = QSharedPointer<UabPackage>;

    explicit UabPackage(const Uab::UabPkgInfo::Ptr &metaPtr);

    [[nodiscard]] const Uab::UabPkgInfo::Ptr &info() const;
    [[nodiscard]] bool isValid() const;
    void setDependsStatus(Pkg::DependsStatus status);
    void setProcessError(Pkg::ErrorCode err, const QString &errorString);
    [[nodiscard]] bool fileExists() const;
    void markNotExists();

    [[nodiscard]] Pkg::DependsStatus dependsStatus() const;
    [[nodiscard]] Pkg::PackageInstallStatus installStatus() const;
    [[nodiscard]] Pkg::PackageOperationStatus operationStatus() const;
    [[nodiscard]] Pkg::ErrorCode errorCode() const;

    [[nodiscard]] QString installedVersion() const;
    [[nodiscard]] QString failedReason() const;
    [[nodiscard]] QString processError() const;

    [[nodiscard]] static UabPackage::Ptr fromInfo(const Uab::UabPkgInfo::Ptr &infoPtr);
    [[nodiscard]] static UabPackage::Ptr fromFilePath(const QString &filePath);

private:
    void reset();

private:
    Uab::UabPkgInfo::Ptr m_metaPtr;

    bool m_exists{true};

    Pkg::DependsStatus m_dependsStatus{Pkg::DependsOk};            // package's depends info
    Pkg::PackageInstallStatus m_installStatus{Pkg::NotInstalled};  // version check
    Pkg::PackageOperationStatus m_operationStatus{Pkg::Prepare};   // status for operation flow
    Pkg::ErrorCode m_errorCode{Pkg::NoError};

    QString m_installedVersion;
    QString m_failReason;
    QString m_processError;  // runtime install/uninstall error raw output

    friend class UabPackageListModel;

    Q_DISABLE_COPY(UabPackage)
};

}  // namespace Uab

#endif  // UABPACKAGE_H
