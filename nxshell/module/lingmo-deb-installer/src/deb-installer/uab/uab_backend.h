// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABBACKEND_H
#define UABBACKEND_H

#include <QObject>
#include <QPointer>
#include <QSet>

#include "uab_defines.h"

namespace Uab {

class UabBackend : public QObject
{
    Q_OBJECT

public:
    static UabBackend *instance();

    [[nodiscard]] UabPkgInfo::Ptr findPackage(const QString &packageId, const QString &version = {});

    void initBackend(bool async = true);
    [[nodiscard]] bool backendInited() const;
    Q_SIGNAL void backendInitFinsihed();

    [[nodiscard]] bool linglongExists() const;
    bool recheckLinglongExists();

    [[nodiscard]] QString lastError() const;
    void dumpPackageList() const;

    [[nodiscard]] static UabPkgInfo::Ptr packageFromMetaData(const QString &uabPath, QString *errorString = nullptr);
    [[nodiscard]] static UabPkgInfo::Ptr packageFromMetaJson(const QByteArray &json, QString *errorString = nullptr);
    [[nodiscard]] static QByteArray uabExecuteOutput(const QString &uabPath, QString *errorString = nullptr);

    // internal
    Q_SLOT void backendInitData(const QList<UabPkgInfo::Ptr> &packageList, const QSet<QString> &archs);
    static void backendProcess(const QPointer<Uab::UabBackend> &notifyPtr);
    static bool parsePackagesFromRawJson(const QByteArray &jsonData, QList<UabPkgInfo::Ptr> &packageList);
    static bool parsePackagesFromRawOutput(const QByteArray &output, QList<UabPkgInfo::Ptr> &packageList);
    static void sortPackages(QList<UabPkgInfo::Ptr> &packageList);

    // update backend database after controller process finished.
    void packageInstalled(const UabPkgInfo::Ptr &appendPtr);
    void packageRemoved(const UabPkgInfo::Ptr &removePtr);

private:
    explicit UabBackend(QObject *parent = nullptr);
    ~UabBackend() override;

private:
    bool m_init{false};
    bool m_linglongExists{false};  // check Linglong executable (ll-cli) exists.
    QList<UabPkgInfo::Ptr> m_packageList;
    QSet<QString> m_supportArchSet;
    QString m_lastError;

    Q_DISABLE_COPY(UabBackend)
};

}  // namespace Uab

#endif  // UABBACKEND_H
