// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPROCESSCONTROLLER_H
#define UABPROCESSCONTROLLER_H

#include <QObject>

#include "uab_package.h"

namespace Konsole {
class Pty;
}  // namespace Konsole

namespace Uab {

class UabProcessController : public QObject
{
    Q_OBJECT

public:
    explicit UabProcessController(QObject *parent = nullptr);
    ~UabProcessController() override = default;

    enum ProcessType {
        Unknown,
        BackendCli,  // transport to lingmo-deb-installer-dependsInstall, use ll-cli interface
        DirectCli,   // means direct qprocess execute, command line interface
    };
    void setProcessType(ProcessType type);
    [[nodiscard]] ProcessType processType() const;

    enum ProcFlag {
        Prepare = 1 << 0,
        Uninstalling = 1 << 1,
        Installing = 1 << 2,
        Processing = 1 << 3,
        Finish = 1 << 4,
        Error = 1 << 5,
    };
    Q_DECLARE_FLAGS(ProcFlags, ProcFlag)
    Q_FLAG(ProcFlags)
    [[nodiscard]] ProcFlags procFlag() const;
    [[nodiscard]] bool isRunning() const;

    bool reset();
    bool markInstall(const UabPackage::Ptr &installPtr);
    bool markUninstall(const UabPackage::Ptr &unisntallPtr);
    [[nodiscard]] bool commitChanges();

    Q_SIGNAL void processStart();
    Q_SIGNAL void processFinished(bool success);
    Q_SIGNAL void processOutput(const QString &output);
    Q_SIGNAL void progressChanged(float progress);

private:
    [[nodiscard]] bool ensureProcess();
    void parseProgressFromJson(const QByteArray &jsonData);
    void parseProgressFromRawOutput(const QByteArray &output);
    void updateWholeProgress(float currentTaskProgress);
    Q_SLOT void onReadOutput(const char *buffer, int length, bool isCommandExec);
    Q_SLOT void onFinished(int exitCode, int exitStatus);
    Q_SLOT void onDBusProgressChanged(int progress, const QString &message);

    bool nextProcess();

    bool installBackendCliImpl(const UabPackage::Ptr &installPtr);
    bool uninstallBackendCliImpl(const UabPackage::Ptr &uninstallPtr);
    bool installCliImpl(const UabPackage::Ptr &installPtr);
    bool uninstallCliImpl(const UabPackage::Ptr &uninstallPtr);

    [[nodiscard]] bool checkIndexValid();
    [[nodiscard]] UabPackage::Ptr currentPackagePtr();

    void commitCurrentChangeToBackend();

private:
    Konsole::Pty *m_process{nullptr};
    ProcessType m_type{Unknown};

    ProcFlags m_procFlag{Prepare};
    int m_currentIndex{-1};
    QList<QPair<ProcFlag, UabPackage::Ptr>> m_procList;  // install/uninstall package list

    Q_DISABLE_COPY(UabProcessController)
};

}  // namespace Uab

#endif  // UABPROCESSCONTROLLER_H
