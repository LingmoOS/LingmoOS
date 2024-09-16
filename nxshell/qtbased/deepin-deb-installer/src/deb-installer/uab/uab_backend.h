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

    UabPkgInfo::Ptr packageFromMetaData(const QString &uabPath);
    UabPkgInfo::Ptr findPackage(const QString &packageName);

    void initBackend();
    bool backendInited() const;
    Q_SIGNAL void backendInitFinsihed();

    QString lastError() const;
    void dumpPackageList() const;

    // internal
    Q_SLOT void backendInitData(const QList<UabPkgInfo::Ptr> &packageList, const QSet<QString> &archs);
    static void backendProcess(const QPointer<Uab::UabBackend> &notifyPtr);
    static void sortPackages(QList<UabPkgInfo::Ptr> &packageList);

private:
    explicit UabBackend(QObject *parent = nullptr);
    ~UabBackend() override;

    UabPkgInfo::Ptr packageFromMetaJson(const QByteArray &json);
    QByteArray uabExecuteOutput(const QString &uabPath);

private:
    bool m_init { false };
    QList<UabPkgInfo::Ptr> m_packageList;
    QSet<QString> m_supportArchSet;
    QString m_lastError;

    Q_DISABLE_COPY(UabBackend)
};

}  // namespace Uab

#endif  // UABBACKEND_H
