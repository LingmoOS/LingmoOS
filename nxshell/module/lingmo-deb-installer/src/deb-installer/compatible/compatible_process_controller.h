// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLEPROCESSCONTROLLER_H
#define COMPATIBLEPROCESSCONTROLLER_H

#include <QObject>

#include "utils/deb_package.h"

namespace Konsole {
class Pty;
}  // namespace Konsole

namespace Compatible {

class CompatibleProcessController : public QObject
{
    Q_OBJECT

public:
    explicit CompatibleProcessController(QObject *parent = nullptr);
    ~CompatibleProcessController() override = default;

    [[nodiscard]] const Deb::DebPackage::Ptr &currentPackage() const;

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] bool install(const Deb::DebPackage::Ptr &package);
    [[nodiscard]] bool uninstall(const Deb::DebPackage::Ptr &package);

    [[nodiscard]] bool needTemplates() const;
    void writeConfigData(const QString &configData);

    Q_SIGNAL void processStart();
    Q_SIGNAL void processFinished(bool success);
    Q_SIGNAL void processOutput(const QString &output);
    Q_SIGNAL void progressChanged(float progress);

private:
    [[nodiscard]] bool ensureProcess();

    Q_SLOT void onReadOutput(const char *buffer, int length, bool isCommandExec);
    Q_SLOT void onFinished(int exitCode, int exitStatus);

    enum ProcessType {
        Install,
        Uninstall,
    };
    ProcessType m_type{Install};
    QStringList m_outputList;  // for simunalte progress

    Konsole::Pty *m_process{nullptr};
    Deb::DebPackage::Ptr m_currentPackage;

    Q_DISABLE_COPY(CompatibleProcessController)
};

};  // namespace Compatible

#endif  // COMPATIBLEPROCESSCONTROLLER_H
