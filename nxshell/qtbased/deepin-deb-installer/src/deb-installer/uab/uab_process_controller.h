// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPROCESSCONTROLLER_H
#define UABPROCESSCONTROLLER_H

#include "uab_defines.h"

#include <QObject>

class QProcess;

namespace Uab {

class UabProcessController : public QObject
{
    Q_OBJECT

public:
    explicit UabProcessController(QObject *parent = nullptr);
    ~UabProcessController() = default;

    enum ProcFlag {
        Prepare = 1 << 0,
        Uninstalling = 1 << 1,
        Installing = 1 << 2,
        Upgrading = 1 << 3,
        Finish = 1 << 4,
        Error = 1 << 5,
    };
    Q_DECLARE_FLAGS(ProcFlags, ProcFlag)
    Q_FLAG(ProcFlags)
    ProcFlags procFlag() const;
    bool isRunning() const;

    bool install(const UabPkgInfo::Ptr &installPtr);
    bool uninstall(const UabPkgInfo::Ptr &unisntallPtr);
    bool upgradge(const UabPkgInfo::Ptr &installPtr, const UabPkgInfo::Ptr &uninstallPtr);

    QString lastError() const;

    Q_SIGNAL void processStart();
    Q_SIGNAL void processFinished(bool success);
    Q_SIGNAL void processOutput(const QString &output);
    Q_SIGNAL void progressChanged(float progress);

private:
    bool ensureProcess();
    Q_SLOT void onReadOutput();
    Q_SLOT void onFinished(int exitCode, int exitStatus);

    bool commitChange(const UabPkgInfo::Ptr &installPtr, const UabPkgInfo::Ptr &uninstallPtr);

    void installImpl();
    void uninstallImpl();
    void upgradgeImpl();

private:
    ProcFlags m_procFlag { Prepare };
    QProcess *m_process { nullptr };

    UabPkgInfo::Ptr m_uninstallPkg;
    UabPkgInfo::Ptr m_installPkg;

    Q_DISABLE_COPY(UabProcessController);
};

}  // namespace Uab

#endif  // UABPROCESSCONTROLLER_H
