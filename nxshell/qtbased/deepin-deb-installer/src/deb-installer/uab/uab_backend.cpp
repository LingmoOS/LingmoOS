// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_backend.h"

#include <mutex>

#include <QApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QProcess>
#include <QtConcurrent/QtConcurrentRun>

#include "utils/utils.h"
// depends deb pacakge arch
#include "model/packageanalyzer.h"

namespace Uab {

// bin command
static const QString kUabCliBin = "ll-cli";
static const QString kUabCliList = "list";
// e.g.: [path to package] --print-meta
static const QString kUabPkgCmdPrintMeta = "--print-meta";
// json field
static const QString kUabLayers = "layers";
static const QString kUabInfo = "info";
static const QString kUabKind = "kind";
static const QString kUabKindAppTag = "app";
// json package info field
static const QString kUabId = "id";
static const QString kUabName = "name";
static const QString kUabVersion = "version";
static const QString kUabArch = "arch";
static const QString kUabChannel = "channel";
static const QString kUabDescription = "description";

UabBackend::UabBackend(QObject *parent)
    : QObject { parent }
{
    qRegisterMetaType<QList<UabPkgInfo::Ptr>>("QList<UabPkgInfo::Ptr>");
}

UabBackend::~UabBackend() { }

UabBackend *UabBackend::instance()
{
    static UabBackend ins;
    return &ins;
}

UabPkgInfo::Ptr UabBackend::packageFromMetaData(const QString &uabPath)
{
    const QFileInfo info(uabPath);
    if (!info.exists()) {
        m_lastError = QString("uab file not exists");
        return {};
    }

    // check uab package executable
    if (!info.isExecutable()) {
        m_lastError = QString("uab file not execuatable");
        return {};
    }

    const QByteArray output = uabExecuteOutput(uabPath);
    if (output.isEmpty()) {
        return {};
    }

    auto uabPtr = UabBackend::packageFromMetaJson(output);
    if (uabPtr) {
        uabPtr->filePath = uabPath;
    }
    return uabPtr;
}

/**
   @brief Find package named \a packageName , the return package is
        the latest version of the installed packages.
   @return Uab package pointer, or null if not found.
 */
UabPkgInfo::Ptr UabBackend::findPackage(const QString &packageName)
{
    if (!backendInited()) {
        m_lastError = QString("uab backend not init");
        return {};
    }

    if (m_packageList.isEmpty()) {
        return {};
    }

    auto itr = std::upper_bound(m_packageList.begin(),
                                m_packageList.end(),
                                packageName,
                                [](const QString &findPkg, const UabPkgInfo::Ptr &uabPtr) { return findPkg <= uabPtr->id; });

    if (m_packageList.end() != itr) {
        return *itr;
    }

    return {};
}

void UabBackend::initBackend()
{
    static std::once_flag kUabBackendFlag;
    std::call_once(kUabBackendFlag, [this]() { QtConcurrent::run(UabBackend::backendProcess, this); });
}

bool UabBackend::backendInited() const
{
    return m_init;
}

QString UabBackend::lastError() const
{
    return m_lastError;
}

void UabBackend::dumpPackageList() const
{
    qInfo() << QString("Uab package list(count %1) support archs:").arg(m_packageList.size()) << m_supportArchSet;
    for (const auto &uabPtr : m_packageList) {
        qInfo() << "    " << uabPtr;
    }
}

void UabBackend::backendInitData(const QList<UabPkgInfo::Ptr> &packageList, const QSet<QString> &archs)
{
    m_packageList = packageList;
    m_supportArchSet = archs;
    m_init = true;
    Q_EMIT backendInitFinsihed();
}

/**
   @brief Get Linglong package list from `ll-cli list`
        The packages are sorted by package name and package version.

   @note This function will be run in QtConcurrent::run()
*/
void UabBackend::backendProcess(const QPointer<Uab::UabBackend> &notifyPtr)
{
    QProcess process;
    process.start(kUabCliBin, { kUabCliList });
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QTextStream stream(&output, QIODevice::ReadOnly);

    // remove title
    stream.readLine();

    QString arch;
    QString deprecated;
    QList<UabPkgInfo::Ptr> packageList;

    while (!stream.atEnd()) {
        auto uabPtr = UabPkgInfo::Ptr::create();

        // title field format: id  name  version  arch  channel  module  description
        stream >> uabPtr->id;
        stream >> uabPtr->appName;
        stream >> uabPtr->version;
        stream >> arch;
        stream >> uabPtr->channel;
        stream >> deprecated;

        uabPtr->architecture.append(arch);
        // to the end of the line
        uabPtr->shortDescription = stream.readLine().simplified();

        packageList.append(uabPtr);
    }

    sortPackages(packageList);

    // detect deb package init
    QSet<QString> archs;
    if (PackageAnalyzer::instance().isBackendReady()) {
        QStringList archList = PackageAnalyzer::instance().supportArchList();
        archs = QSet<QString>(archList.begin(), archList.end());

        // adapt arch name for uab
        if (archs.contains("amd64")) {
            archs.insert("x86_64");
        }

        // TODO(renbin): loongarch64 and loong64 diff arch
    }

    if (!qApp->closingDown() && !notifyPtr.isNull()) {
        QMetaObject::invokeMethod(
            notifyPtr, "backendInitData", Q_ARG(QList<UabPkgInfo::Ptr>, packageList), Q_ARG(QSet<QString>, archs));
    }
}

/**
   @brief Sort \a packageList by package id and version.
    Package id in ascending order, version in descending order.
    Make sure the latest version of a package is listed first.
 */
void UabBackend::sortPackages(QList<UabPkgInfo::Ptr> &packageList)
{
    if (packageList.isEmpty()) {
        return;
    }

    std::sort(packageList.begin(), packageList.end(), [](const UabPkgInfo::Ptr &left, const UabPkgInfo::Ptr &right) {
        const int ret = QString::compare(left->id, right->id);
        if (!!ret) {
            return ret < 0;
        }
        return Utils::compareVersion(left->version, right->version) > 0;
    });
}

UabPkgInfo::Ptr UabBackend::packageFromMetaJson(const QByteArray &json)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (QJsonParseError::NoError != error.error) {
        m_lastError = QString("uab json parse erorr: %1, offset: %2").arg(error.errorString()).arg(error.offset);
        return {};
    }

    const QJsonArray layers = doc.object().value(kUabLayers).toArray();
    if (layers.isEmpty()) {
        m_lastError = QString("uab json not contains 'layers'");
        return {};
    }

    for (const QJsonValue &layerItem : layers) {
        if (!layerItem.isObject()) {
            continue;
        }

        QJsonObject info = layerItem.toObject().value(kUabInfo).toObject();
        if (info.isEmpty()) {
            continue;
        }

        if (kUabKindAppTag != info.value(kUabKind).toString()) {
            continue;
        }

        auto uabPtr = UabPkgInfo::Ptr::create();
        uabPtr->id = info[kUabId].toString();
        uabPtr->appName = info[kUabName].toString();
        uabPtr->version = info[kUabVersion].toString();
        uabPtr->shortDescription = info[kUabDescription].toString();
        uabPtr->channel = info[kUabChannel].toString();

        const QJsonArray archArray = info[kUabArch].toArray();
        for (const QJsonValue &archValue : archArray) {
            const QString curArch = archValue.toString();
            uabPtr->architecture.append(curArch);
        }

        return uabPtr;
    }

    m_lastError = QString("uab json not contains app info node");
    return {};
}

QByteArray UabBackend::uabExecuteOutput(const QString &uabPath)
{
    QProcess proc;
    proc.setProgram(uabPath);
    proc.setArguments({ kUabPkgCmdPrintMeta });
    proc.start();
    proc.waitForFinished();

    if (0 != proc.exitCode() || QProcess::NormalExit != proc.exitStatus()) {
        m_lastError = QString("exec uab package failed: %1").arg(proc.errorString());
        return {};
    }

    return proc.readAllStandardOutput();
}

}  // namespace Uab

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug out, const Uab::UabPkgInfo &uabPkg)
{
    out << "UabPackageInfo(" << QString("0x%0").arg(reinterpret_cast<quintptr>(&uabPkg), 0, 16) << "){";
    out << Uab::kUabId << ":" << uabPkg.id << ";";
    out << Uab::kUabName << ":" << uabPkg.appName << ";";
    out << Uab::kUabVersion << ":" << uabPkg.version << ";";
    out << Uab::kUabArch << ":" << uabPkg.architecture << ";";
    out << Uab::kUabChannel << ":" << uabPkg.channel << ";";
    out << Uab::kUabDescription << ":" << uabPkg.shortDescription << ";";
    out << "filePath:" << uabPkg.filePath << ";";

    return out;
}

Q_CORE_EXPORT QDebug operator<<(QDebug out, const Uab::UabPkgInfo::Ptr &uabPkgPtr)
{
    if (uabPkgPtr) {
        out << *uabPkgPtr;
    } else {
        out << QString("UabPackageInfo:Ptr(nullptr)");
    }

    return out;
}
#endif  // QT_NO_DEBUG_STREAM
QT_END_NAMESPACE
