/*
 * Copyright (C) 2021 Lingmo Technology Co., Ltd.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "debinstaller.h"
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStringBuilder>

#include <QApt/Backend>
#include <QApt/Package>
#include <apt-pkg/debversion.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/version.h>

#include <QThread>
#include <QDebug>

static QString formatByteSize(double size, int precision)
{
    int unit = 0;
    double multiplier = 1024.0;

    while (qAbs(size) >= multiplier && unit < int(8)) {
        size /= multiplier;
        ++unit;
    }

    if (unit == 0) {
        precision = 0;
    }

    QString numString = QString::number(size, 'f', precision);

    switch (unit) {
    case 0:
        return QString("%1 B").arg(numString);
    case 1:
        return QString("%1 KB").arg(numString);
    case 2:
        return QString("%1 MB").arg(numString);
    case 3:
        return QString("%1 GB").arg(numString);
    case 4:
        return QString("%1 TB").arg(numString);
    case 5:
        return QString("%1 PB").arg(numString);
    case 6:
        return QString("%1 EB").arg(numString);
    case 7:
        return QString("%1 ZB").arg(numString);
    case 8:
        return QString("%1 YB").arg(numString);
    default:
        return QString();
    }

    return QString();
}

DebInstaller::DebInstaller(QObject *parent)
    : QObject(parent)
    , m_backend(new QApt::Backend(this))
    , m_debFile(nullptr)
    , m_transaction(nullptr)
    , m_isValid(false)
    , m_canInstall(false)
    , m_isInstalled(false)
    , m_status(DebInstaller::Begin)
{
}

DebInstaller::~DebInstaller() {
    // 确保释放动态分配的资源
    delete m_debFile;
    delete m_transaction;
}

QString DebInstaller::fileName() const
{
    return m_fileName;
}

void DebInstaller::setFileName(const QString &fileName)
{
    if (fileName.isEmpty() || !m_backend->init() || m_fileName == fileName) {
        qDebug() << "Invalid file name or backend initialization failed.";
        return;
    }

    QString newPath = fileName;
    newPath.remove("file://");
    QFileInfo info(newPath);
    QString mimeType = QMimeDatabase().mimeTypeForFile(info.absoluteFilePath()).name();

    if (mimeType != "application/vnd.debian.binary-package") {
        qDebug() << "Invalid MIME type:" << mimeType;
        return;
    }

    m_fileName = info.absoluteFilePath();
    emit fileNameChanged(); // 立即发出信号

    QApt::FrontendCaps caps = (QApt::FrontendCaps)(QApt::DebconfCap);
    m_backend->setFrontendCaps(caps);
    m_debFile = new QApt::DebFile(m_fileName);

    QApt::Package *package = m_backend->package(m_debFile->packageName());

    if (package) {
        m_isInstalled = package->isInstalled();
        emit isInstalledChanged();
    } else {
        m_isInstalled = false;
    }

    m_isValid = m_debFile->isValid();
    emit isValidChanged();

    if (!m_isValid) {
        return;
    }

    m_packageName = m_debFile->packageName();
    emit packageNameChanged();

    m_version = m_debFile->version();
    emit versionChanged();

    m_maintainer = m_debFile->maintainer();
    emit maintainerChanged();

    m_description = m_debFile->longDescription();
    emit descriptionChanged();

    m_homePage = m_debFile->homepage();
    emit homePageChanged();

    m_installedSize = formatByteSize(m_debFile->installedSize() * 1024.0, 1);
    emit installedSizeChanged();

    m_canInstall = checkDeb();
    emit canInstallChanged();

    QApt::Package *pkg = m_backend->package(m_debFile->packageName());
    if (pkg) {
        m_installedVersion = pkg->availableVersion();
    } else {
        m_installedVersion.clear();
    }

    emit installedVersionChanged();

    // QStringList fileList = m_debFile->fileList();
    // std::sort(fileList.begin(), fileList.end());

    emit fileNameChanged();
}

QString DebInstaller::packageName() const
{
    return m_packageName;
}

QString DebInstaller::version() const
{
    return m_version;
}

QString DebInstaller::maintainer() const
{
    return m_maintainer;
}

QString DebInstaller::description() const
{
    return m_description;
}

bool DebInstaller::isValid() const
{
    return m_isValid;
}

bool DebInstaller::canInstall() const
{
    return m_canInstall;
}

QString DebInstaller::homePage() const
{
    return m_homePage;
}

QString DebInstaller::installedSize() const
{
    return m_installedSize;
}

QString DebInstaller::installedVersion() const
{
    return m_installedVersion;
}

bool DebInstaller::isInstalled() const
{
    return m_isInstalled;
}

void DebInstaller::install()
{
    if (m_backend->markedPackages().size()) {
        m_transaction = m_backend->commitChanges();
    } else {
        m_transaction = m_backend->installFile(*m_debFile);
    }

    setupTransaction();
    m_transaction->run();
}

void DebInstaller::setupTransaction()
{
    if (!m_transaction)
        return;

    m_transaction->setLocale(setlocale(LC_MESSAGES, 0));

    connect(m_transaction, &QApt::Transaction::statusChanged, this, &DebInstaller::transactionStatusChanged);
    connect(m_transaction, &QApt::Transaction::errorOccurred, this, &DebInstaller::errorOccurred);
    connect(m_transaction, &QApt::Transaction::statusDetailsChanged, this, &DebInstaller::statusDetailsChanged);
}

void DebInstaller::setStatus(DebInstaller::Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

bool DebInstaller::checkDeb()
{
    QStringList arches = m_backend->architectures();
    arches.append(QLatin1String("all"));
    QString debArch = m_debFile->architecture();

    // Check if we support the arch at all
    if (debArch != m_backend->nativeArchitecture()) {
        if (!arches.contains(debArch)) {
            // Wrong arch
            qDebug() << "wrong arch";
            m_preInstallMessage = tr("Error: Wrong architecture %1").arg(debArch);
            m_preInstallMessage.prepend(QLatin1String("<font color=\"#ff0000\">"));
            m_preInstallMessage.append(QLatin1String("</font>"));
            return false;
        }

        // We support this foreign arch
        m_foreignArch = debArch;
    }

    QApt::PackageList conflicts = checkConflicts();
    if (!conflicts.isEmpty()) {
        return false;
    }

    QApt::Package *willBreak = checkBreaksSystem();
    if (willBreak) {
        m_preInstallMessage = tr("Error: Breaks the existing package %1").arg(willBreak->name());
        m_preInstallMessage.prepend(QLatin1String("<font color=\"#ff0000\">"));
        m_preInstallMessage.append(QLatin1String("</font>"));
        emit preInstallMessageChanged();
        return false;
    }

    satisfyDepends();

//    if (!satisfyDepends()) {
//        // create status message
//        m_preInstallMessage = tr("Error: Cannot satisfy dependencies");
//        m_preInstallMessage.prepend(QLatin1String("<font color=\"#ff0000\">"));
//        m_preInstallMessage.append(QLatin1String("</font>"));
//        emit preInstallMessageChanged();
//        return false;
//    }

    int toInstall = m_backend->markedPackages().size();
    if (toInstall) {
        m_preInstallMessage = tr("Requires the installation of %1 additional package.").arg(toInstall);
    } else {
        m_preInstallMessage = "";
    }

    emit preInstallMessageChanged();

    return true;
}

QString DebInstaller::maybeAppendArchSuffix(const QString &pkgName, bool checkingConflicts)
{
    // Trivial cases where we don't append
    if (m_foreignArch.isEmpty())
        return pkgName;

    QApt::Package *pkg = m_backend->package(pkgName);
    if (!pkg || pkg->architecture() == QLatin1String("all"))
        return pkgName;

    // Real multiarch checks
    QString multiArchName = pkgName % ':' % m_foreignArch;
    QApt::Package *multiArchPkg = m_backend->package(multiArchName);

    // Check for a new dependency, we'll handle that later
    if (!multiArchPkg)
        return multiArchName;

    // Check the multi arch state
    QApt::MultiArchType type = multiArchPkg->multiArchType();

    // Add the suffix, unless it's a pkg that can satify foreign deps
    if (type == QApt::MultiArchForeign)
        return pkgName;

    // If this is called as part of a conflicts check, any not-multiarch
    // enabled package is a conflict implicitly
    if (checkingConflicts && type == QApt::MultiArchSame)
        return pkgName;

    return multiArchName;
}

QApt::PackageList DebInstaller::checkConflicts()
{
    QApt::PackageList conflictingPackages;
    QList<QApt::DependencyItem> conflicts = m_debFile->conflicts();

    QApt::Package *pkg = 0;
    QString packageName;
    bool ok = true;
    foreach(const QApt::DependencyItem &item, conflicts) {
        foreach (const QApt::DependencyInfo &info, item) {
            packageName = maybeAppendArchSuffix(info.packageName(), true);
            pkg = m_backend->package(packageName);

            if (!pkg) {
                // FIXME: Virtual package, must check provides
                continue;
            }

            std::string pkgVer = pkg->version().toStdString();
            std::string depVer = info.packageVersion().toStdString();

            ok = _system->VS->CheckDep(pkgVer.c_str(),
                                       info.relationType(),
                                       depVer.c_str());

            if (ok) {
                // Group satisfied
                break;
            }
        }

        if (!ok && pkg) {
            conflictingPackages.append(pkg);
        }
    }

    return conflictingPackages;
}

QApt::Package *DebInstaller::checkBreaksSystem()
{
    QApt::PackageList systemPackages = m_backend->availablePackages();
    std::string debVer = m_debFile->version().toStdString();

    foreach (QApt::Package *pkg, systemPackages) {
        if (!pkg->isInstalled()) {
            continue;
        }

        // Check for broken depends
        foreach(const QApt::DependencyItem &item, pkg->depends()) {
            foreach (const QApt::DependencyInfo &dep, item) {
                if (dep.packageName() != m_debFile->packageName()) {
                    continue;
                }

                std::string depVer = dep.packageVersion().toStdString();

                if (!_system->VS->CheckDep(debVer.c_str(), dep.relationType(),
                                           depVer.c_str())) {
                    return pkg;
                }
            }
        }

        // Check for existing conflicts against the .deb
        // FIXME: Check provided virtual packages too
        foreach(const QApt::DependencyItem &item, pkg->conflicts()) {
            foreach (const QApt::DependencyInfo &conflict, item) {
                if (conflict.packageName() != m_debFile->packageName()) {
                    continue;
                }

                std::string conflictVer = conflict.packageVersion().toStdString();

                if (_system->VS->CheckDep(debVer.c_str(),
                                          conflict.relationType(),
                                          conflictVer.c_str())) {
                    return pkg;
                }
            }
        }
    }

    return 0;
}

bool DebInstaller::satisfyDepends()
{
//    foreach(const QApt::DependencyItem &item, m_debFile->depends()) {
//        foreach (const QApt::DependencyInfo &dep, item) {
//            const QString &packageName = maybeAppendArchSuffix(dep.packageName());
//            QApt::Package *pkg = m_backend->package(packageName);

//            if (!pkg)
//                continue;

//            if (pkg->isInstalled())
//                continue;

//            m_backend->markPackageForInstall(packageName);
//        }
//    }

//    return true;

    QApt::Package *pkg = 0;
    QString packageName;

    foreach(const QApt::DependencyItem &item, m_debFile->depends()) {
        bool oneSatisfied = false;
        foreach (const QApt::DependencyInfo &dep, item) {
            packageName = maybeAppendArchSuffix(dep.packageName());
            pkg = m_backend->package(packageName);

            if (!pkg) {
                // FIXME: virtual package handling
                continue;
            }

            std::string debVersion = dep.packageVersion().toStdString();

            // If we're installed, see if we already satisfy the dependency
            if (pkg->isInstalled()) {
                std::string pkgVersion = pkg->installedVersion().toStdString();

                if (_system->VS->CheckDep(pkgVersion.c_str(),
                                          dep.relationType(),
                                          debVersion.c_str())) {
                    oneSatisfied = true;
                    break;
                }
            }

            // else check if cand ver will satisfy, then mark
            std::string candVersion = pkg->availableVersion().toStdString();

            if (!_system->VS->CheckDep(candVersion.c_str(),
                                       dep.relationType(),
                                       debVersion.c_str())) {
                continue;
            }

            pkg->setInstall();

            if (!pkg->wouldBreak()) {
                oneSatisfied = true;
                break;
            }
        }

        if (!oneSatisfied) {
            return false;
        }
    }

    return true;
}

void DebInstaller::transactionStatusChanged(QApt::TransactionStatus status)
{
    switch (status) {
    case QApt::SetupStatus:
    case QApt::WaitingStatus:
    case QApt::AuthenticationStatus:
        m_statusMessage = tr("Starting");
        emit statusMessageChanged();
        break;
    case QApt::WaitingMediumStatus:
    case QApt::WaitingLockStatus:
    case QApt::WaitingConfigFilePromptStatus:
        m_statusMessage = tr("Waiting");
        emit statusMessageChanged();
        emit requestSwitchToInstallPage();
        setStatus(DebInstaller::Installing);
        break;
    case QApt::RunningStatus:
        m_statusMessage = tr("Installing");
        emit statusMessageChanged();
        emit requestSwitchToInstallPage();
        setStatus(DebInstaller::Installing);
        break;
    case QApt::LoadingCacheStatus:
        m_statusMessage = tr("Loading Cache");
        emit statusMessageChanged();
        setStatus(DebInstaller::Installing);
        break;
    case QApt::DownloadingStatus:
        m_statusMessage = tr("Downloading Packages");
        emit statusMessageChanged();
        setStatus(DebInstaller::Installing);
        break;
    case QApt::CommittingStatus:
        break;
    case QApt::FinishedStatus:
        if (m_transaction->role() == QApt::CommitChangesRole) {
            delete m_transaction;
            m_transaction = m_backend->installFile(*m_debFile);
            setupTransaction();
            m_transaction->run();
        } else if (m_transaction->role() == QApt::InstallFileRole) {
            m_statusMessage = tr("Installation successful");
            emit statusMessageChanged();
            setStatus(DebInstaller::Succeeded);
        }
        break;
    default:
        break;
    }
}

void DebInstaller::errorOccurred(QApt::ErrorCode error)
{
    switch (error) {
        case QApt::InitError:
        case QApt::WrongArchError: {
            m_statusMessage = tr("Installation failed");
            emit statusMessageChanged();
            setStatus(DebInstaller::Error);
            break;
        }
        default:
            break;
    }
}

void DebInstaller::statusDetailsChanged(const QString &message)
{
    m_statusDetails = m_statusDetails + message;

    if (message.endsWith('\n'))
        m_statusDetails.append('\n');

    emit statusDetailsTextChanged();
}

QString DebInstaller::statusDetails() const
{
    return m_statusDetails;
}

QString DebInstaller::preInstallMessage() const
{
    return m_preInstallMessage;
}

DebInstaller::Status DebInstaller::status() const
{
    return m_status;
}

QString DebInstaller::statusMessage() const
{
    return m_statusMessage;
}
