// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package.h"
#include "uab_backend.h"
#include "utils/utils.h"

namespace Uab {

/**
 * @class UabPackage
 * @brief Uab package, contains package information and runtime status.
 *        Store version, install status, operation status, error code, and error message.
 */
UabPackage::UabPackage(const UabPkgInfo::Ptr &metaPtr)
    : m_metaPtr(metaPtr)
{
    reset();
}

const UabPkgInfo::Ptr &UabPackage::info() const
{
    return m_metaPtr;
}

bool UabPackage::isValid() const
{
    if (!m_metaPtr || !fileExists()) {
        return false;
    }

    if (m_metaPtr->id.isEmpty() || m_metaPtr->appName.isEmpty()) {
        return false;
    }

    return true;
}

void UabPackage::setDependsStatus(Pkg::DependsStatus status)
{
    m_dependsStatus = status;
    switch (m_dependsStatus) {
        case Pkg::DependsBreak:
            m_failReason = QObject::tr("The system has not installed Linglong environment, please install it first");
            break;
        default:
            break;
    }
}

void UabPackage::setProcessError(Pkg::ErrorCode err, const QString &errorString)
{
    m_errorCode = err;
    m_processError = errorString;
}

bool UabPackage::fileExists() const
{
    return m_exists;
}

void UabPackage::markNotExists()
{
    m_exists = false;
}

Pkg::DependsStatus UabPackage::dependsStatus() const
{
    return m_dependsStatus;
}

Pkg::PackageInstallStatus UabPackage::installStatus() const
{
    return m_installStatus;
}

Pkg::PackageOperationStatus UabPackage::operationStatus() const
{
    return m_operationStatus;
}

Pkg::ErrorCode UabPackage::errorCode() const
{
    return m_errorCode;
}

QString UabPackage::installedVersion() const
{
    return m_installedVersion;
}

QString UabPackage::failedReason() const
{
    if (m_failReason.isEmpty()) {
        return m_processError;
    }

    return m_failReason;
}

QString UabPackage::processError() const
{
    return m_processError;
}

UabPackage::Ptr UabPackage::fromInfo(const UabPkgInfo::Ptr &infoPtr)
{
    return Uab::UabPackage::Ptr::create(infoPtr);
}

UabPackage::Ptr UabPackage::fromFilePath(const QString &filePath)
{
    QString error;
    auto infoPtr = Uab::UabBackend::packageFromMetaData(filePath, &error);
    auto uabPtr = Uab::UabPackage::Ptr::create(infoPtr);

    if (!error.isEmpty()) {
        qWarning() << qPrintable("Uab from path:") << error;
    }
    return uabPtr;
}

void UabPackage::reset()
{
    if (!m_metaPtr) {
        return;
    }

    m_operationStatus = Pkg::Prepare;

    m_installedVersion.clear();
    m_installStatus = Pkg::NotInstalled;

    m_errorCode = Pkg::NoError;
    m_processError.clear();

    if (!Uab::UabBackend::instance()->linglongExists()) {
        setDependsStatus(Pkg::DependsBreak);
    } else {
        setDependsStatus(Pkg::DependsOk);
    }

    if (Uab::UabBackend::instance()->backendInited()) {
        auto uabInfoPtr = Uab::UabBackend::instance()->findPackage(m_metaPtr->id);
        if (uabInfoPtr) {
            m_installedVersion = uabInfoPtr->version;
            const int ret = Utils::compareVersion(m_metaPtr->version, uabInfoPtr->version);

            if (ret == 0) {
                m_installStatus = Pkg::InstalledSameVersion;
            } else if (ret < 0) {
                m_installStatus = Pkg::InstalledLaterVersion;
            } else {
                m_installStatus = Pkg::InstalledEarlierVersion;
            }
        }
    }
}

}  // namespace Uab
