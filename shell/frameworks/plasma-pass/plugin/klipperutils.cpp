// SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "klipperutils.h"

#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace
{

const int klipperMinMajorVersion = 5;
const int klipperMinMinorVersion = 13;

}

using namespace PlasmaPass;

KlipperUtils::State KlipperUtils::getState()
{
    const auto klipperExecutable = QStandardPaths::findExecutable(QStringLiteral("klipper"));
    if (klipperExecutable.isEmpty()) {
        return State::Missing;
    }

    QProcess process{};
    process.setProgram(klipperExecutable);
    process.setArguments({QStringLiteral("--version")});
    process.start(QIODevice::ReadOnly);
    process.waitForFinished();
    if (process.exitStatus() != QProcess::NormalExit) {
        return State::Missing;
    }
    const auto output = process.readAllStandardOutput();

    const QRegularExpression rx(QStringLiteral(R"/(^klipper ([0-9]+)\.([0-9]+))/"));
    const auto match = rx.match(QString::fromUtf8(output));
    if (!match.hasMatch()) {
        return State::Available;
    }

    const auto majorVersionStr = match.capturedView(1);
    bool ok = false;
    const int majorVersion = majorVersionStr.toInt(&ok);
    if (!ok) {
        return State::Available;
    }

    const auto minorVersionStr = match.capturedView(2);
    ok = false;
    const int minorVersion = minorVersionStr.toInt(&ok);
    if (!ok) {
        return State::Available;
    }

    // Klipper supports x-kde-passwordManagerHint since Plasma 5.13
    const auto verdict = ((majorVersion > klipperMinMajorVersion) || (majorVersion == klipperMinMajorVersion && minorVersion >= klipperMinMinorVersion));
    return verdict ? State::SupportsPasswordManagerHint : State::Available;
}