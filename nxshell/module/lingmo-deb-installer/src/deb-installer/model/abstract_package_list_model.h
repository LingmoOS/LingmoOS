// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACT_PACKAGE_LIST_MODEL_H
#define ABSTRACT_PACKAGE_LIST_MODEL_H

#include "utils/package_defines.h"

#include <QAbstractListModel>

// interface for deb / uab package
class AbstractPackageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // package role for list model
    enum PackageRole {
        PackageNameRole = Qt::DisplayRole,
        UnusedRole = Qt::UserRole,
        WorkerIsPrepareRole,  // ready to work (install/uninstall) sa WorkerStatus
        ItemIsCurrentRole,
        PackageVersionRole,
        PackagePathRole,
        PackageInstalledVersionRole,  // installed version
        PackageShortDescriptionRole,
        PackageLongDescriptionRole,
        PackageVersionStatusRole,         // sa Pkg::PackageInstallStatus
        PackageDependsStatusRole,         // sa Pkg::DependsStatus
        PackageAvailableDependsListRole,  // available dependencies for the package
        PackageFailReasonRole,            // fail reason for install / uninstall failed
        PackageOperateStatusRole,         // sa Pkg::PackageOperationStatus
        PackageReverseDependsListRole,    // list of packages that reversely depend on the current package

        PackageTypeRole,           // is uab or deb package, sa Pkg::PackageType
        PackageDependsDetailRole,  // details for unfinished depends, empty if no errors occur, sa Pkg::DependsPair
        PackageRemoveDependsRole,  // package will be remove if current install

        CompatibleRootfsRole,        // compatible mode only, provides current compatible rootfs name
        CompatibleTargetRootfsRole,  // target rootfs name
    };

    // the list model backend installer status
    enum WorkerStatus {
        WorkerPrepare,     // ready to work
        WorkerProcessing,  // installing / upgrading
        WorkerFinished,    // install / uninstall finished
        WorkerUnInstall
    };

    // wine or other predepends install status
    enum DependsAuthStatus {
        AuthBefore,          // before the authentication dialog pops up
        AuthPop,             // the authentication dialog pops up
        CancelAuth,          // authentication cancellation
        AuthConfirm,         // after authentication confirmation
        AuthDependsSuccess,  // install success
        AuthDependsErr,      // install failed
        AnalysisErr,         // pacakge analysis erorr
        VerifyDependsErr,    // signature verification failed (hierarchical verify)
    };

    explicit AbstractPackageListModel(QObject *parent = nullptr);

    [[nodiscard]] inline Pkg::PackageType supportPackage() const { return m_supportPackageType; }

    [[nodiscard]] WorkerStatus getWorkerStatus() const;
    void setWorkerStatus(WorkerStatus status);
    [[nodiscard]] inline bool isWorkerPrepare() const { return WorkerPrepare == getWorkerStatus(); }

    Q_SLOT virtual void slotAppendPackage(const QStringList &packageList) = 0;
    virtual void removePackage(int index) = 0;
    virtual QString checkPackageValid(const QString &packagePath) = 0;

    // package base info
    virtual Pkg::PackageInstallStatus checkInstallStatus(const QString &packagePath) = 0;
    virtual Pkg::DependsStatus checkDependsStatus(const QString &packagePath) = 0;
    virtual QStringList getPackageInfo(const QString &packagePath) = 0;

    // raw output of install/uninstall failures
    virtual QString lastProcessError() = 0;
    // a package signature verification fails
    virtual bool containsSignatureFailed() const = 0;

    // trigger install / uninstall
    Q_SLOT virtual bool slotInstallPackages() = 0;
    Q_SLOT virtual bool slotUninstallPackage(int index) = 0;

    virtual void reset() = 0;
    virtual void resetInstallStatus() = 0;

Q_SIGNALS:
    // package append flow control
    void signalAppendStart();
    void signalAppendFinished();
    // manange package insert failed reason
    void signalAppendFailMessage(Pkg::AppendFailReason reason, Pkg::PackageType type = Pkg::Deb);
    // manage package appended and remove
    void signalPackageCountChanged(int count);

    // install/upgrade/uninstall process output info
    void signalAppendOutputInfo(const QString &output);
    // current install progress
    void signalCurrentPacakgeProgressChanged(int progress);
    // whole install progress
    void signalWholeProgressChanged(int progress);
    // current process install package
    void signalCurrentProcessPackageIndex(int index);

    // package removed or rename
    void signalPackageCannotFind(QString packageName) const;

    // install/uninstall flow status, start and finish
    void signalWorkerStart();
    void signalWorkerFinished();

    // These interfaces are used by authorization and wine dependencies, no changes for compatibility.
    // They may be moved to singletons.
    void signalLockForAuth(bool locked) const;
    void signalAuthCancel();
    // set cancel button enabled while open authorization check dialog (pkexec)
    void signalEnableReCancelBtn(bool bEnable);

    // wine depends install progress info.
    void signalDependResult(int, QString);
    // set close button enabled while install wine depends
    void signalEnableCloseButton(bool);

protected:
    Pkg::PackageType m_supportPackageType{Pkg::UnknownPackage};
    WorkerStatus m_workerStatus{WorkerPrepare};  // current worker status

private:
    Q_DISABLE_COPY(AbstractPackageListModel)
};

#endif  // ABSTRACT_PACKAGE_LIST_MODEL_H
