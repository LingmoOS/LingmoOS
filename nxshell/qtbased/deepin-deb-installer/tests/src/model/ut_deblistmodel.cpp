// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/manager/PackageDependsStatus.h"
#include "../deb-installer/utils/utils.h"
#include "../deb-installer/utils/result.h"
#include "../deb-installer/utils/hierarchicalverify.h"

#include <stub.h>

#include <QProcess>
#include <QList>
#include <QSignalSpy>

#include <fstream>

using namespace QApt;

class ut_DebListModel_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;
    void stubFunction();

    DebListModel *m_debListModel = nullptr;
    Stub stub;
    Stub stubHierachicalInvalid;
};

bool model_backend_init()
{
    return true;
}

void model_checkSystemVersion() {}

bool model_BackendReady()
{
    return true;
}

bool stub_model_is_open()
{
    qDebug() << "stb——is_open";
    return true;
}

void stub_model_open(const std::string &__s, std::ios_base::openmode __mode)
{
    Q_UNUSED(__s);
    Q_UNUSED(__mode);
    qDebug() << "stb——open";
}

QString model_deb_arch_i386()
{
    return "i386";
}

bool model_deb_isValid()
{
    return true;
}

QByteArray model_deb_md5Sum()
{
    return nullptr;
}

int model_deb_installSize()
{
    return 0;
}

QString model_deb_packageName()
{
    return "";
}

QString model_deb_longDescription()
{
    return "longDescription";
}

QString model_deb_shortDescription()
{
    return "shortDescription";
}

QString model_deb_version()
{
    return "version";
}

QList<DependencyItem> model_deb_conflicts()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

Package *model_packageWithArch(QString, QString, QString)
{
    return nullptr;
}

QStringList model_backend_architectures()
{
    return {"i386", "amd64"};
}
PackageDependsStatus model_getPackageDependsStatus(const int index)
{
    Q_UNUSED(index);
    PackageDependsStatus status;
    return status;
}

bool model_stud_recheckPackagePath_true(QString)
{
    return true;
}

bool model_stud_recheckPackagePath_false(QString)
{
    return false;
}
bool model_stub_dealInvalidPackage(QString)
{
    return true;
}

Package *model_package_package(const QString &name)
{
    return nullptr;
}

QList<DependencyItem> model_deb_depends()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

void model_initRowStatus()
{
    return;
}

void model_installNextDeb()
{
    return;
}

void model_transaction_run()
{
    return;
}

QString model_packageManager_package(const int index)
{
    return "";
}

const QStringList model_packageManager_packageReverseDependsList(const QString &, const QString &)
{
    return {};
}

void model_backend_markPackageForRemoval(const QString &)
{
    return;
}

void model_refreshOperatingPackageStatus(const DebListModel::PackageOperationStatus)
{
    return;
}

QApt::Transaction *model_backend_commitChanges()
{
    return nullptr;
}

QByteArray model_package_getPackageMd5(const int)
{
    return "";
}

bool model_package_isArchError(const int)
{
    return true;
}

bool model_package_isArchError1(const int)
{
    return false;
}

QString stub_readLink_empty()
{
    return "";
}

QString stub_readLink()
{
    return "1";
}

bool stub_exists_true()
{
    return true;
}

bool stub_exists_false()
{
    return false;
}

static Dtk::Core::DSysInfo::UosEdition model_uosEditionType_UosEnterprise()
{
    return Dtk::Core::DSysInfo::UosEnterprise;
}

static Dtk::Core::DSysInfo::UosEdition model_uosEditionType_UosProfessional()
{
    return Dtk::Core::DSysInfo::UosProfessional;
}

static Dtk::Core::DSysInfo::UosEdition model_uosEditionType_UosHome()
{
    return Dtk::Core::DSysInfo::UosHome;
}

static Dtk::Core::DSysInfo::UosEdition model_uosEditionType_UosCommunity()
{
    return Dtk::Core::DSysInfo::UosCommunity;
}

static Dtk::Core::DSysInfo::UosEdition model_uosEditionType_default()
{
    return Dtk::Core::DSysInfo::UosEditionUnknown;
}

bool stud_toBool()
{
    return true;
}
Utils::VerifyResultCode model_Digital_Verify(QString)
{
    qDebug() << "model_Digital_Verify";
    return Utils::VerifySuccess;
}

Utils::VerifyResultCode model_Digital_Verify1(QString)
{
    qDebug() << "model_Digital_Verify";
    return Utils::DebfileInexistence;
}

Utils::VerifyResultCode model_Digital_Verify2(QString)
{
    qDebug() << "model_Digital_Verify";
    return Utils::ExtractDebFail;
}

Utils::VerifyResultCode model_Digital_Verify3(QString)
{
    return Utils::DebVerifyFail;
}

Utils::VerifyResultCode model_Digital_Verify4(QString)
{
    return Utils::OtherError;
}

void model_bumpInstallIndex()
{
    return;
}
QApt::ErrorCode model_transaction_error()
{
    return QApt::AuthError;
}

QApt::ExitStatus model_transaction_exitStatus()
{
    return QApt::ExitFailed;
}

QApt::ExitStatus model_transaction_exitStatus1()
{
    return QApt::ExitSuccess;
}

QString model_transaction_errorString()
{
    return "error";
}

bool model_transaction_isCancellable()
{
    return false;
}

bool model_transaction_isCancelled()
{
    return true;
}

QByteArray model_getPackageMd5()
{
    return "1";
}

QString model_transaction_errorDetails()
{
    return "";
}

bool model_transaction_setProperty(const char *, const QVariant &)
{
    return true;
}

bool ut_process_startDetached(qint64 *)
{
    return false;
}
qint64 ut_process_write(const QByteArray &)
{
    return 0;
}

bool stub_DebListModel_Hierarchical_Invalid()
{
    return false;
}

bool stub_DebListModel_Hierarchical_Valid()
{
    return true;
}

static int g_VerifyDeependErrCount = 0;
void stub_DebListModel_showHierarchicalVerifyWindow_DoNothing()
{
    g_VerifyDeependErrCount++;
}

void ut_DebListModel_test::stubFunction()
{
    stub.set((void(std::fstream::*)(const std::string &__s, std::ios_base::openmode __mode))ADDR(std::fstream, open),
             stub_model_open);
    stub.set((bool(std::fstream::*)())ADDR(std::fstream, is_open), stub_model_is_open);

    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, commitChanges), model_backend_commitChanges);
    stub.set(ADDR(Backend, reloadCache), model_backend_init);

    stub.set(ADDR(Transaction, run), model_transaction_run);
    stub.set(ADDR(Transaction, errorString), model_transaction_errorString);
    stub.set(ADDR(Transaction, isCancellable), model_transaction_isCancellable);
    stub.set(ADDR(Transaction, isCancelled), model_transaction_isCancelled);
    stub.set(ADDR(Transaction, errorDetails), model_transaction_errorDetails);
    stub.set(ADDR(Transaction, setProperty), model_transaction_setProperty);
    stub.set(ADDR(Transaction, exitStatus), model_transaction_exitStatus);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(DebFile, isValid), model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), model_deb_installSize);
    stub.set(ADDR(DebFile, packageName), model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription), model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription), model_deb_shortDescription);
    stub.set(ADDR(DebFile, version), model_deb_version);
    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);
    stub.set(ADDR(DebFile, depends), model_deb_depends);

    stub.set(ADDR(PackagesManager, getPackageMd5), model_package_getPackageMd5);
    stub.set(ADDR(PackagesManager, isArchError), model_package_isArchError);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), model_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);
    stub.set(ADDR(PackagesManager, package), model_packageManager_package);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), model_stub_dealInvalidPackage);

    stub.set((Package * (Backend::*)(const QString &) const) ADDR(Backend, package), model_package_package);

    stubHierachicalInvalid.set(ADDR(HierarchicalVerify, isValid), stub_DebListModel_Hierarchical_Invalid);
}

void ut_DebListModel_test::SetUp()
{
    stubFunction();

    m_debListModel = new DebListModel();
    usleep(10 * 1000);
}

void ut_DebListModel_test::TearDown()
{
    delete m_debListModel;
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_reset)
{
    m_debListModel->reset();
    EXPECT_EQ(m_debListModel->m_workerStatus, DebListModel::WorkerPrepare);
    EXPECT_EQ(0, m_debListModel->m_operatingIndex);
    EXPECT_TRUE(m_debListModel->m_operatingPackageMd5.isEmpty());
    EXPECT_EQ(0, m_debListModel->m_operatingStatusIndex);
    EXPECT_TRUE(m_debListModel->m_packageOperateStatus.isEmpty());
    EXPECT_TRUE(m_debListModel->m_packageFailCode.isEmpty());
    EXPECT_TRUE(m_debListModel->m_packageFailReason.isEmpty());
    EXPECT_TRUE(m_debListModel->m_packagesManager->m_errorIndex.isEmpty());
    EXPECT_FALSE(m_debListModel->m_hierarchicalVerifyError);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_reset_filestatus)
{
    m_debListModel->m_packageOperateStatus[0] = 1;
    m_debListModel->resetFileStatus();
    EXPECT_TRUE(m_debListModel->m_packageOperateStatus.isEmpty());
    EXPECT_TRUE(m_debListModel->m_packageFailReason.isEmpty());
    EXPECT_TRUE(m_debListModel->m_packageFailCode.isEmpty());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_isReady)
{
    EXPECT_TRUE(m_debListModel->isReady());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_isWorkerPrepare)
{
    EXPECT_TRUE(m_debListModel->isWorkerPrepare());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_preparedPackages)
{
    EXPECT_TRUE(m_debListModel->preparedPackages().isEmpty());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_slotAppendPackage)
{
    QStringList list;
    list << "/";

    m_debListModel->slotAppendPackage(list);

    EXPECT_EQ(m_debListModel->preparedPackages().size(), 1);
    EXPECT_EQ(DebListModel::WorkerPrepare, m_debListModel->m_workerStatus);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_first)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    ASSERT_EQ(m_debListModel->first().row(), 0);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_rowCount)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    QModelIndex index;
    ASSERT_EQ(m_debListModel->rowCount(index), 1);
}

bool stub_recheckPackagePath(QString)
{
    return true;
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_data)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    QModelIndex index = m_debListModel->index(0);
    int i = 1;
    m_debListModel->data(index, 1);
    i = 2;
    m_debListModel->slotAppendPackage(QStringList() << "deb");
    stub.set(ADDR(PackagesManager, packageReverseDependsList), model_packageManager_packageReverseDependsList);
    stub.set(ADDR(DebListModel, recheckPackagePath), stub_recheckPackagePath);
    while (i <= 13) {
        m_debListModel->data(index, i);
        i++;
    }

    for (int i = 257; i <= 269; i++) {
        m_debListModel->data(index, i);
    }

    m_debListModel->data(index, 1);

    m_debListModel->m_packageOperateStatus[""] = 1;
    m_debListModel->data(index, 268);
    EXPECT_EQ(1, m_debListModel->m_packagesManager->m_preparedPackages.size());
}
TEST_F(ut_DebListModel_test, deblistmodel_UT_data_recheck)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    QModelIndex index = m_debListModel->index(0);

    EXPECT_EQ(1, m_debListModel->m_packagesManager->m_preparedPackages.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_isDevelopMode)
{
    m_debListModel->m_isDevelopMode = true;

    EXPECT_TRUE(m_debListModel->isDevelopMode());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_initPrepareStatus)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    m_debListModel->initPrepareStatus();
    ASSERT_EQ(m_debListModel->m_packageOperateStatus[0], DebListModel::Prepare);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_index)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    ASSERT_EQ(m_debListModel->index(0).data(DebListModel::PackageDependsStatusRole).toInt(), DebListModel::DependsOk);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_getInstallFileSize)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    ASSERT_EQ(m_debListModel->getInstallFileSize(), 1);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_setCurrentIndex)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    QModelIndex index = m_debListModel->index(0);
    m_debListModel->slotSetCurrentIndex(index);
    ASSERT_EQ(m_debListModel->m_currentIdx, m_debListModel->index(0));
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_installPackages)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    m_debListModel->slotInstallPackages();
    ASSERT_EQ(m_debListModel->m_workerStatus, DebListModel::WorkerProcessing);
    ASSERT_EQ(0, m_debListModel->m_operatingIndex);
    ASSERT_EQ(0, m_debListModel->m_operatingStatusIndex);
}

Package *stub_model_packageWithArch(QString)
{
    Backend *bac = nullptr;
    pkgCache::PkgIterator packageIter;
    QScopedPointer<Package> package(new Package(bac, packageIter));
    return package.get();
}

void stub_setPurge()
{
    return;
}

Package *ut_package(QString, QString, QString)
{
    Backend *bac = nullptr;
    pkgCache::PkgIterator packageIter;
    //    Package *package = new Package(bac,packageIter);
    QScopedPointer<Package> package(new Package(bac, packageIter));
    return package.get();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_uninstallPackage)
{
    stub.set(ADDR(PackagesManager, packageReverseDependsList), model_packageManager_packageReverseDependsList);
    stub.set(ADDR(Backend, markPackageForRemoval), model_backend_markPackageForRemoval);
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    m_debListModel->slotUninstallPackage(0);
    ASSERT_EQ(m_debListModel->m_workerStatus, DebListModel::WorkerProcessing);
    ASSERT_EQ(m_debListModel->m_operatingIndex, 0);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_uninstallPackage1)
{
    stub.set(ADDR(PackagesManager, packageReverseDependsList), model_packageManager_packageReverseDependsList);
    stub.set(ADDR(Backend, markPackageForRemoval), model_backend_markPackageForRemoval);
    //    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), stub_model_packageWithArch);
    stub.set(ADDR(Package, setPurge), stub_setPurge);
    QStringList list;
    list << "/";
    m_debListModel->m_workerStatus = DebListModel::WorkerProcessing;
    m_debListModel->slotAppendPackage(list);

    m_debListModel->slotUninstallPackage(0);
    ASSERT_EQ(m_debListModel->m_workerStatus, DebListModel::WorkerProcessing);
}

bool ut_model_isBreak()
{
    return true;
}

ConflictResult ut_packageConflictStat()
{
    return ConflictResult::err("");
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_packageFailedReason)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    ASSERT_STREQ(m_debListModel->packageFailedReason(0).toLocal8Bit(), "Unmatched package architecture");
    Stub stub;
    stub.set(ADDR(PackagesManager, isArchError), model_package_isArchError1);
    stub.set(ADDR(PackageDependsStatus, isBreak), ut_model_isBreak);
    stub.set(ADDR(PackagesManager, packageConflictStat), ut_packageConflictStat);
    m_debListModel->packageFailedReason(0);
    EXPECT_TRUE(m_debListModel->packageFailedReason(0).contains("Broken dependencies"));
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_initRowStatus)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    m_debListModel->m_packageMd5.insert(0, "deb");

    m_debListModel->initRowStatus();

    ASSERT_EQ(m_debListModel->m_packageOperateStatus.find("deb").value(), DebListModel::Waiting);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkSystemVersion_UosEnterprise)
{
    stub.set(ADDR(Dtk::Core::DSysInfo, uosEditionType), model_uosEditionType_UosEnterprise);
    EXPECT_TRUE(Utils::isDevelopMode());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkSystemVersion_UosProfessional)
{
    stub.set(ADDR(Dtk::Core::DSysInfo, uosEditionType), model_uosEditionType_UosProfessional);
    Utils::isDevelopMode();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkSystemVersion_UosHome)
{
    stub.set(ADDR(Dtk::Core::DSysInfo, uosEditionType), model_uosEditionType_UosHome);
    Utils::isDevelopMode();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkSystemVersion_UosCommunity)
{
    stub.set(ADDR(Dtk::Core::DSysInfo, uosEditionType), model_uosEditionType_UosCommunity);
    EXPECT_TRUE(Utils::isDevelopMode());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkSystemVersion_default)
{
    stub.set(ADDR(Dtk::Core::DSysInfo, uosEditionType), model_uosEditionType_default);
    EXPECT_TRUE(Utils::isDevelopMode());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkDigitalSignature)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    stub.set((Utils::VerifyResultCode(*)(QString))ADDR(Utils, Digital_Verify), model_Digital_Verify);

    m_debListModel->m_isDevelopMode = false;

    m_debListModel->m_operatingIndex = 0;
    ASSERT_TRUE(m_debListModel->checkDigitalSignature());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkDigitalSignature_01)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    stub.set((Utils::VerifyResultCode(*)(QString))ADDR(Utils, Digital_Verify), model_Digital_Verify1);

    m_debListModel->m_isDevelopMode = false;

    m_debListModel->m_operatingIndex = 0;
    ASSERT_FALSE(m_debListModel->checkDigitalSignature());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkDigitalSignature_02)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    stub.set((Utils::VerifyResultCode(*)(QString))ADDR(Utils, Digital_Verify), model_Digital_Verify2);

    m_debListModel->m_isDevelopMode = false;

    m_debListModel->m_operatingIndex = 0;
    ASSERT_FALSE(m_debListModel->checkDigitalSignature());

    Stub stub1;
    stub1.set((Utils::VerifyResultCode(*)(QString))ADDR(Utils, Digital_Verify), model_Digital_Verify3);
    ASSERT_FALSE(m_debListModel->checkDigitalSignature());
    Stub stub2;
    stub2.set((Utils::VerifyResultCode(*)(QString))ADDR(Utils, Digital_Verify), model_Digital_Verify4);
    ASSERT_FALSE(m_debListModel->checkDigitalSignature());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkDigitalSignature_03)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    m_debListModel->m_operatingIndex = 0;
    Stub stub;
    m_debListModel->m_isDevelopMode = true;
    m_debListModel->m_isDigitalVerify = true;
    stub.set((Utils::VerifyResultCode(*)(QString))ADDR(Utils, Digital_Verify), model_Digital_Verify2);
    ASSERT_TRUE(m_debListModel->checkDigitalSignature());
    m_debListModel->showDevelopDigitalErrWindow(DebListModel::NoDigitalSignature);
    EXPECT_EQ(1, m_debListModel->preparedPackages().size());
}

void model_digitalVerifyFailed()
{
    return;
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_showNoDigitalErrWindow)
{
    stub.set(ADDR(DebListModel, digitalVerifyFailed), model_digitalVerifyFailed);
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    m_debListModel->m_operatingIndex = 0;

    m_debListModel->m_packagesManager->m_preparedPackages.append("1");

    m_debListModel->showNoDigitalErrWindow();
    EXPECT_TRUE(m_debListModel->m_operatingIndex < m_debListModel->m_packagesManager->m_preparedPackages.size() - 1);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_removePackage)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);

    m_debListModel->m_operatingIndex = 0;

    m_debListModel->m_packagesManager->m_preparedPackages.append("1");

    m_debListModel->removePackage(0);

    ASSERT_EQ(m_debListModel->m_packagesManager->m_preparedPackages.size(), 1);
    ASSERT_TRUE(m_debListModel->m_packageOperateStatus.isEmpty());
    m_debListModel->slotAppendPackage(list);
    m_debListModel->m_packageOperateStatus[""] = 1;
    m_debListModel->removePackage(0);
    ASSERT_EQ(m_debListModel->m_packagesManager->m_preparedPackages.size(), 0);
    ASSERT_TRUE(m_debListModel->m_packageOperateStatus.isEmpty());
}

QApt::ErrorCode model_transaction_commitError()
{
    return QApt::CommitError;
}

QObject *ut_sender()
{
    Transaction *transaction = new Transaction("1");
    return qobject_cast<QObject *>(transaction);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_onTransactionErrorOccurred)
{
    stub.set(ADDR(Transaction, error), model_transaction_error);
    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageOperateStatus.insert("deb", QApt::CommitError);
    m_debListModel->m_packageFailCode.insert("deb", QApt::FetchError);
    m_debListModel->m_isDevelopMode = Utils::isDevelopMode();
    m_debListModel->m_workerStatus = DebListModel::WorkerProcessing;
    m_debListModel->m_packageMd5.insert(0, "00000");
    m_debListModel->m_operatingIndex = 0;

    Stub stub1;
    stub1.set(ADDR(QObject, sender), ut_sender);

    m_debListModel->slotTransactionErrorOccurred();
    EXPECT_EQ(QApt::AuthError, m_debListModel->m_packageFailCode[m_debListModel->m_operatingPackageMd5]);
    EXPECT_EQ(DebListModel::Failed, m_debListModel->m_packageOperateStatus[m_debListModel->m_operatingPackageMd5]);
    delete ut_sender();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_DealDependResult)
{
    stub.set(ADDR(DebListModel, refreshOperatingPackageStatus), model_refreshOperatingPackageStatus);
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_bumpInstallIndex);
    stub.set(ADDR(DebListModel, getPackageMd5), model_getPackageMd5);

    m_debListModel->slotDealDependResult(1, 0, "");
    m_debListModel->slotDealDependResult(2, 0, "");
    EXPECT_EQ(DebListModel::Prepare, m_debListModel->m_packageOperateStatus["1"]);
    m_debListModel->slotDealDependResult(3, 0, "");
    m_debListModel->slotDealDependResult(4, 0, "");
    EXPECT_EQ(DebListModel::Prepare, m_debListModel->m_packageOperateStatus["1"]);
    EXPECT_EQ(DebListModel::Prepare, m_debListModel->m_workerStatus);
    m_debListModel->slotDealDependResult(5, 0, "");
    EXPECT_EQ("", m_debListModel->m_brokenDepend);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_DealDependResult_VerifyDependsErr)
{
    Stub tmpStub;
    tmpStub.set(ADDR(DebListModel, showHierarchicalVerifyWindow), stub_DebListModel_showHierarchicalVerifyWindow_DoNothing);

    g_VerifyDeependErrCount = 0;
    m_debListModel->slotDealDependResult(DebListModel::VerifyDependsErr, 0, "");
    EXPECT_EQ(g_VerifyDeependErrCount, 1);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_onTransactionStatusChanged)
{
    TransactionStatus stat = TransactionStatus::AuthenticationStatus;
    m_debListModel->slotTransactionStatusChanged(stat);
    stat = TransactionStatus::WaitingStatus;
    m_debListModel->slotTransactionStatusChanged(stat);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_setEndEnable)
{
    QSignalSpy spy(m_debListModel, SIGNAL(signalEnableReCancelBtn(bool)));
    m_debListModel->setEndEnable();
    EXPECT_EQ(1, spy.count());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkBoxStatus)
{
    m_debListModel->checkBoxStatus();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_bumpInstallIndex)
{
    stub.set(ADDR(DebListModel, installNextDeb), model_installNextDeb);
    m_debListModel->m_operatingIndex = 0;
    m_debListModel->m_packageMd5.append("\n");
    m_debListModel->m_packageMd5.append("1");
    m_debListModel->bumpInstallIndex();
    EXPECT_EQ("", m_debListModel->m_operatingPackageMd5);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_bumpInstallIndex_showHierarchicalVerifyWindow)
{
    stub.set(ADDR(DebListModel, installNextDeb), model_installNextDeb);
    m_debListModel->m_operatingIndex = 0;
    m_debListModel->m_packageMd5.append("\n");
    m_debListModel->m_packageMd5.append("1");
    m_debListModel->m_hierarchicalVerifyError = true;

    Stub tmpStub;
    tmpStub.set(ADDR(DebListModel, showHierarchicalVerifyWindow), stub_DebListModel_showHierarchicalVerifyWindow_DoNothing);

    g_VerifyDeependErrCount = 0;
    m_debListModel->bumpInstallIndex();
    EXPECT_EQ(g_VerifyDeependErrCount, 1);
    EXPECT_FALSE(m_debListModel->m_hierarchicalVerifyError);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_ConfigInstallFinish)
{
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_bumpInstallIndex);
    m_debListModel->m_packagesManager->m_preparedPackages.append("/");
    m_debListModel->slotConfigInstallFinish(1);
    EXPECT_EQ(DebListModel::WorkerPrepare, m_debListModel->m_workerStatus);
}

QByteArray model_readAllStandardOutput()
{
    return "StartInstallAptConfig";
}

QByteArray model_readAllStandardOutput1()
{
    return "StartInstall";
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_ConfigReadOutput)
{
    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageOperateStatus.insert("deb", QApt::CommitError);

    stub.set(ADDR(Konsole::Pty, receivedData), model_readAllStandardOutput);
    // asan检查 内存泄露
    QString buffer = "11111111";
    int length = sizeof(buffer);
    bool isCommandExec = false;
    m_debListModel->slotConfigReadOutput(buffer.toStdString().c_str(), length, isCommandExec);
    EXPECT_EQ(DebListModel::PackageOperationStatus::Operating,
              m_debListModel->m_packageOperateStatus[m_debListModel->m_operatingPackageMd5]);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_onTransactionFinished)
{
    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageFailCode.insert("deb", QApt::CommitError);
    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageFailCode.insert("deb", QApt::CommitError);
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_bumpInstallIndex);
    stub.set(ADDR(Transaction, error), model_transaction_error);
    m_debListModel->m_operatingIndex = 0;
    m_debListModel->m_packageMd5.append("test");
    m_debListModel->m_packageMd5.append("test1");
    m_debListModel->m_packagesManager->m_preparedPackages.append(QString("deb"));
    m_debListModel->m_packagesManager->m_preparedPackages.append(QString("deb1"));

    Stub stub1;
    stub1.set(ADDR(QObject, sender), ut_sender);
    m_debListModel->slotTransactionFinished();
    EXPECT_EQ(nullptr, m_debListModel->m_currentTransaction);
    EXPECT_EQ(QApt::AuthError, m_debListModel->m_packageFailCode[m_debListModel->m_operatingPackageMd5]);
    delete ut_sender();
}

static Transaction *g_transaction = nullptr;
QObject *stub_UT_Transaction_Sender()
{
    return qobject_cast<QObject *>(g_transaction);
}

void stub_showDigitalErrWindow()
{
    // Do nothing.
}

QString stub_Transacton_ErrorDetails_Failed()
{
    // 65280 is hierarchical verfiy error code.
    return QString("deepin hook exit code:65280");
}

TEST_F(ut_DebListModel_test, onTransactionFinished_HierarchicalVerify_Failed)
{
    // Enable hierarchical verify
    stubHierachicalInvalid.set(ADDR(HierarchicalVerify, isValid), stub_DebListModel_Hierarchical_Valid);

    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageFailCode.insert("deb", QApt::CommitError);
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_bumpInstallIndex);
    stub.set(ADDR(DebListModel, showDigitalErrWindow), stub_showDigitalErrWindow);
    stub.set(ADDR(Transaction, error), model_transaction_error);
    stub.set(ADDR(Transaction, errorDetails), stub_Transacton_ErrorDetails_Failed);
    m_debListModel->m_operatingIndex = 0;
    m_debListModel->m_packageMd5.append("test");
    m_debListModel->m_packageMd5.append("test1");
    m_debListModel->m_packagesManager->m_preparedPackages.append(QString("deb"));
    m_debListModel->m_packagesManager->m_preparedPackages.append(QString("deb1"));

    Stub stub1;
    g_transaction = new Transaction("1");
    stub1.set(ADDR(QObject, sender), stub_UT_Transaction_Sender);
    m_debListModel->m_hierarchicalVerifyError = false;

    m_debListModel->slotTransactionFinished();
    EXPECT_EQ(nullptr, m_debListModel->m_currentTransaction);
    EXPECT_EQ(DebListModel::DigitalSignatureError, m_debListModel->m_packageFailCode[m_debListModel->m_operatingPackageMd5]);
    EXPECT_TRUE(m_debListModel->m_hierarchicalVerifyError);

    delete g_transaction;
    g_transaction = nullptr;
}

QString stub_Transacton_ErrorDetails_Pass()
{
    return QString("deepin hook exit code:65277");
}

TEST_F(ut_DebListModel_test, onTransactionFinished_HierarchicalVerify_Pass)
{
    // Enable hierarchical verify
    stubHierachicalInvalid.set(ADDR(HierarchicalVerify, isValid), stub_DebListModel_Hierarchical_Valid);

    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageFailCode.insert("deb", QApt::CommitError);
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_bumpInstallIndex);
    stub.set(ADDR(DebListModel, showDigitalErrWindow), stub_showDigitalErrWindow);
    stub.set(ADDR(Transaction, error), model_transaction_error);
    stub.set(ADDR(Transaction, errorDetails), stub_Transacton_ErrorDetails_Pass);
    m_debListModel->m_operatingIndex = 0;
    m_debListModel->m_packageMd5.append("test");
    m_debListModel->m_packageMd5.append("test1");
    m_debListModel->m_packagesManager->m_preparedPackages.append(QString("deb"));
    m_debListModel->m_packagesManager->m_preparedPackages.append(QString("deb1"));

    Stub stub1;
    g_transaction = new Transaction("1");
    stub1.set(ADDR(QObject, sender), stub_UT_Transaction_Sender);
    m_debListModel->m_hierarchicalVerifyError = false;

    m_debListModel->slotTransactionFinished();
    EXPECT_EQ(nullptr, m_debListModel->m_currentTransaction);
    EXPECT_EQ(QApt::AuthError, m_debListModel->m_packageFailCode[m_debListModel->m_operatingPackageMd5]);
    EXPECT_FALSE(m_debListModel->m_hierarchicalVerifyError);

    delete g_transaction;
    g_transaction = nullptr;
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_refreshOperatingPackageStatus)
{
    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageOperateStatus.insert("deb", QApt::CommitError);
    m_debListModel->refreshOperatingPackageStatus(DebListModel::PackageOperationStatus::Failed);
    EXPECT_EQ(DebListModel::PackageOperationStatus::Failed,
              m_debListModel->m_packageOperateStatus[m_debListModel->m_operatingPackageMd5]);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_slotDependsInstallTransactionFinished)
{
    stub.set(ADDR(DebListModel, refreshOperatingPackageStatus), model_refreshOperatingPackageStatus);
    stub.set(ADDR(DebListModel, getPackageMd5), model_getPackageMd5);
    stub.set(ADDR(DebListModel, installNextDeb), model_installNextDeb);
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_installNextDeb);
    stub.set(ADDR(Transaction, error), model_transaction_error);

    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->m_packageFailCode.insert("deb", QApt::CommitError);

    Stub stub1;
    stub1.set(ADDR(QObject, sender), ut_sender);
    m_debListModel->slotDependsInstallTransactionFinished();
    EXPECT_EQ(nullptr, m_debListModel->m_currentTransaction);
    EXPECT_EQ(QApt::AuthError, m_debListModel->m_packageFailCode[m_debListModel->m_operatingPackageMd5]);
    delete ut_sender();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_showDevelopModeWindow)
{
    stub.set((bool(QProcess::*)(qint64 *))ADDR(QProcess, startDetached), ut_process_startDetached);
    m_debListModel->slotShowDevelopModeWindow();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_slotUpWrongStatusRow)
{
    stub.set(ADDR(DebListModel, refreshOperatingPackageStatus), model_refreshOperatingPackageStatus);
    stub.set(ADDR(DebListModel, getPackageMd5), model_getPackageMd5);
    stub.set(ADDR(DebListModel, installNextDeb), model_installNextDeb);
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_installNextDeb);
    stub.set(ADDR(Transaction, error), model_transaction_error);
    m_debListModel->m_packageOperateStatus.insert("key", DebListModel::Failed);
    m_debListModel->m_packageOperateStatus.insert("key1", DebListModel::Success);
    m_debListModel->slotUpWrongStatusRow();
    EXPECT_EQ(2, m_debListModel->m_packagesManager->m_packageMd5.size());
    EXPECT_EQ(2, m_debListModel->m_packagesManager->m_preparedPackages.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_ConfigInputWrite)
{
    stub.set((qint64(QProcess::*)(const QByteArray &))ADDR(QProcess, write), ut_process_write);

    m_debListModel->slotConfigInputWrite("\n");
    EXPECT_TRUE(m_debListModel->m_procInstallConfig);
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_onTransactionOutput)
{
    Stub stub1;
    stub1.set(ADDR(QObject, sender), ut_sender);
    m_debListModel->m_packageOperateStatus.insert("deb", DebListModel::Prepare);
    m_debListModel->m_operatingPackageMd5 = "deb";
    m_debListModel->slotTransactionOutput();
    EXPECT_EQ(DebListModel::Operating, m_debListModel->m_packageOperateStatus[m_debListModel->m_operatingPackageMd5]);
    delete ut_sender();
}

ExitStatus ut_exitStatus()
{
    return ExitStatus::ExitSuccess;
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_uninstallFinished)
{
    stub.set(ADDR(Transaction, error), model_transaction_run);
    Stub stub1;
    stub1.set(ADDR(QObject, sender), ut_sender);
    m_debListModel->slotUninstallFinished();
    stub1.set(ADDR(Transaction, exitStatus), ut_exitStatus);
    m_debListModel->slotUninstallFinished();
    EXPECT_EQ(DebListModel::WorkerFinished, m_debListModel->m_workerStatus);
    delete ut_sender();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_checkInstallStatus)
{
    stub.set(ADDR(DebListModel, bumpInstallIndex), model_installNextDeb);
    stub.set(ADDR(DebListModel, refreshOperatingPackageStatus), model_refreshOperatingPackageStatus);

    QString str = "Error executing command as another user: Request dismissed";
    m_debListModel->slotCheckInstallStatus(str);
    EXPECT_EQ(DebListModel::WorkerFinished, m_debListModel->m_workerStatus);
    EXPECT_EQ(1, m_debListModel->m_packageFailCode.size());
    EXPECT_EQ(1, m_debListModel->m_packageFailReason.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_initConnections)
{
    m_debListModel->initConnections();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_slotShowProhibitWindow)
{
    m_debListModel->slotShowProhibitWindow();
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    m_debListModel->m_isDevelopMode = true;
    m_debListModel->slotShowProhibitWindow();
    EXPECT_EQ(1, m_debListModel->m_packageFailCode.size());
    m_debListModel->showProhibitWindow();
    m_debListModel->m_operatingIndex = -1;
    m_debListModel->showProhibitWindow();
    EXPECT_EQ(1, m_debListModel->m_packageFailCode.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_initAppendConnection)
{
    m_debListModel->initAppendConnection();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_initRefreshPageConnecions)
{
    m_debListModel->initRefreshPageConnecions();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_initInstallConnections)
{
    m_debListModel->initInstallConnections();
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_recheckPackagePath_readRealPathExist)
{
    stub.set((QString(QFile::*)(void) const)ADDR(QFile, readLink), stub_readLink_empty);
    stub.set((bool(QFile::*)(void) const)ADDR(QFile, exists), stub_exists_true);

    ASSERT_TRUE(m_debListModel->recheckPackagePath("/0"));
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_recheckPackagePath_readLinkPathExist)
{
    stub.set((QString(QFile::*)(void) const)ADDR(QFile, readLink), stub_readLink);
    stub.set((bool(QFile::*)(void) const)ADDR(QFile, exists), stub_exists_true);

    ASSERT_TRUE(m_debListModel->recheckPackagePath("/0"));
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_recheckPackagePath_readLinkPathNotExist)
{
    stub.set((QString(QFile::*)(void) const)ADDR(QFile, readLink), stub_readLink);
    stub.set((bool(QFile::*)(void) const)ADDR(QFile, exists), stub_exists_false);

    ASSERT_FALSE(m_debListModel->recheckPackagePath("/0"));
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_netErrors)
{
    EXPECT_TRUE(m_debListModel->netErrors().at(0).contains("Address family"));
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_workerErrorString)
{
    EXPECT_TRUE(m_debListModel->workerErrorString(1, "").contains("Installation Failed"));
    EXPECT_TRUE(m_debListModel->workerErrorString(2, "No space left on device").contains("insufficient disk space"));
    EXPECT_TRUE(m_debListModel->workerErrorString(2, "").contains("Installation Failed"));
    EXPECT_TRUE(m_debListModel->workerErrorString(3, "").contains("insufficient disk space"));
    EXPECT_TRUE(m_debListModel->workerErrorString(4, "").contains("network connection"));
    EXPECT_TRUE(m_debListModel->workerErrorString(5, "").contains("Installation Failed"));
    EXPECT_TRUE(m_debListModel->workerErrorString(5, "Network is unreachable;http").contains("network connection"));
    EXPECT_TRUE(m_debListModel->workerErrorString(5, "No space left on device").contains("insufficient disk space"));
    EXPECT_TRUE(m_debListModel->workerErrorString(9, "").contains("network connection"));
    EXPECT_TRUE(m_debListModel->workerErrorString(10, "").contains("Control Center"));
    EXPECT_TRUE(m_debListModel->workerErrorString(101, "").contains("No digital signature"));
    EXPECT_TRUE(m_debListModel->workerErrorString(102, "").contains("Invalid digital signature"));
    EXPECT_FALSE(m_debListModel->workerErrorString(127, "").contains("iAuthentication"));
}

const QList<QString> ut_preparedPackages()
{
    QList<QString> list;
    list << "deb"
         << "rpm";
    return list;
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_digitalVerifyFailed)
{
    //    stub.set(ADDR(DebListModel,preparedPackages),ut_preparedPackages);
    m_debListModel->digitalVerifyFailed(DebListModel::ErrorCode::DigitalSignatureError);
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    m_debListModel->m_isDevelopMode = true;
    m_debListModel->digitalVerifyFailed(DebListModel::ErrorCode::DigitalSignatureError);
    EXPECT_EQ(1, m_debListModel->m_packageFailCode.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_slotDigitalSignatureError)
{
    m_debListModel->slotDigitalSignatureError();
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    m_debListModel->m_isDevelopMode = true;
    m_debListModel->slotDigitalSignatureError();
    EXPECT_EQ(1, m_debListModel->m_packageFailCode.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_slotNoDigitalSignature)
{
    QStringList list;
    list << "/";
    m_debListModel->slotAppendPackage(list);
    m_debListModel->m_isDevelopMode = true;
    m_debListModel->slotNoDigitalSignature();
    EXPECT_EQ(1, m_debListModel->m_packageFailCode.size());
}

TEST_F(ut_DebListModel_test, deblistmodel_UT_isWorkPrepare)
{
    m_debListModel->m_workerStatus = DebListModel::WorkerPrepare;
    ASSERT_TRUE(m_debListModel->isWorkerPrepare());
}
