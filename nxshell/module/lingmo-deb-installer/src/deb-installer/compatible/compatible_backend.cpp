// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_backend.h"

#include <QApplication>
#include <QProcess>
#include <QtConcurrent/QtConcurrentRun>
#include <QTextStream>
#include <QRegExp>
#include <QStandardPaths>

#include <mutex>

namespace Compatible {

// compatible controller params
static const QString kCompatibleBin = "lingmo-compatible-ctl";
static const QString kCompList = "list";
static const QString kCompSepecificRootFs = "r";
static const QString kCompRootFs = "rootfs";
static const QString kCompApp = "app";
static const QString kCompInstall = "install";
static const QString kCompRemove = "remove";

CompatibleBackend::CompatibleBackend(QObject *parent)
    : QObject{parent}
{
    recheckCompatibleExists();
}

CompatibleBackend *CompatibleBackend::instance()
{
    static CompatibleBackend ins;
    return &ins;
}

#ifndef DISABLE_COMPATIBLE
bool CompatibleBackend::compatibleValid() const
{
    return m_init && !m_rootfsList.isEmpty();
}

bool CompatibleBackend::compatibleExists() const
{
    return m_compatibleExists;
}

bool CompatibleBackend::recheckCompatibleExists()
{
    // find ll-cli in $PATH
    const QString execPath = QStandardPaths::findExecutable(kCompatibleBin);
    m_compatibleExists = !execPath.isEmpty();

    return m_compatibleExists;
}
#endif

QList<RootfsInfo::Ptr> CompatibleBackend::rootfsList() const
{
    return m_rootfsList;
}

QString CompatibleBackend::osName(const QString &rootfsName) const
{
    auto findItr = std::find_if(m_rootfsList.begin(), m_rootfsList.end(), [&](const RootfsInfo::Ptr &rootfsPtr){
        return rootfsPtr->name == rootfsName;
    });

    if (findItr != m_rootfsList.end()) {
        return (*findItr)->osName;
    }

    return {};
}

CompPkgInfo::Ptr CompatibleBackend::containsPackage(const QString &packageName)
{
    return m_packages.value(packageName);
}

void CompatibleBackend::packageInstalled(const CompPkgInfo::Ptr &appendPtr)
{
    if (!appendPtr) {
        return;
    }
    appendPtr->rootfs = appendPtr->targetRootfs;

    m_packages.insert(appendPtr->name, appendPtr);
}

void CompatibleBackend::packageRemoved(const CompPkgInfo::Ptr &removePtr)
{
    if (!removePtr) {
        return;
    }
    removePtr->rootfs.clear();

    m_packages.remove(removePtr->name);
}

QList<RootfsInfo::Ptr> CompatibleBackend::parseRootfsFromRawOutput(const QByteArray &output)
{
    QTextStream stream(output, QIODevice::ReadOnly);

    // remove title
    stream.readLine();
    stream.readLine();

    bool convert{false};
    QString temp;
    QList<RootfsInfo::Ptr> rootfsList;
    QRegExp priorityReg("^\\d+");
    QRegExp createTimeReg("\\d{4}-\\d{2}-\\d{2}");

    while (!stream.atEnd()) {
        auto rootfsPtr = RootfsInfo::Ptr::create();
        stream >> temp;
        rootfsPtr->prioriy = temp.toInt(&convert);
        if (!convert) {
            int priority = priorityReg.indexIn(temp);
            if (priority != -1) {
                rootfsPtr->prioriy = priorityReg.cap().toInt();
            }
        }

        stream >> rootfsPtr->name;

        // special field: os name ( contains space ), wen
        temp = stream.readLine();
        int createTimePos = createTimeReg.indexIn(temp);
        if (-1 != createTimePos) {
            rootfsPtr->osName = temp.left(createTimePos).trimmed();
        }

        rootfsList.append(rootfsPtr);
    }

    return rootfsList;
}

QHash<QString, CompPkgInfo::Ptr> CompatibleBackend::parseAppListFromRawOutput(const QByteArray &output)
{
    QHash<QString, CompPkgInfo::Ptr> packageList;

    QTextStream stream(output, QIODevice::ReadOnly);

    // remove title
    stream.readLine();
    stream.readLine();

    while (!stream.atEnd()) {
        auto pkgPtr = CompPkgInfo::Ptr::create();

        stream >> pkgPtr->name;
        stream >> pkgPtr->version;
        stream >> pkgPtr->arch;
        stream >> pkgPtr->rootfs;

        packageList.insert(pkgPtr->name, pkgPtr);
    }

    return packageList;
}

void CompatibleBackend::initBackend(bool async)
{
    if (!compatibleExists()) {
        return;
    }

    static std::once_flag kCompInitFlag;
    std::call_once(kCompInitFlag, [this, async]() {
        if (async) {
            QtConcurrent::run([this]() { CompatibleBackend::backendProcess(this); });
        } else {
            CompatibleBackend::backendProcess(this);
        }
    });
}

void CompatibleBackend::backendProcess(CompatibleBackend *backend)
{
    QProcess queryProcess;
    queryProcess.setProgram(kCompatibleBin);
    queryProcess.setArguments({kCompRootFs, kCompList});
    queryProcess.start();
    queryProcess.waitForFinished();

    QByteArray output = queryProcess.readAllStandardOutput();
    auto rootfsList = parseRootfsFromRawOutput(output);

    queryProcess.setArguments({kCompApp, kCompList});
    queryProcess.start();
    queryProcess.waitForFinished();
    output = queryProcess.readAllStandardOutput();
    auto packages = parseAppListFromRawOutput(output);

    // NOTE: ComaptibleBackend might not inited in main thread( no event loop ), so use qApp instaed.
    QMetaObject::invokeMethod(qApp, [backend, rootfsList, packages]() { backend->initFinished(rootfsList, packages); }, Qt::QueuedConnection);
}

void CompatibleBackend::initFinished(const QList<RootfsInfo::Ptr> &rootfsList, const QHash<QString, CompPkgInfo::Ptr> &packages)
{
    m_init = true;
    m_rootfsList = rootfsList;
    m_packages = packages;

    Q_EMIT compatibleInitFinished();
}

};  // namespace Compatible
