// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

#include "base/process.h"

namespace Lingmo {

// When this process writes to the journal, its output and errors move to a
// stream of its own under `identifier`; otherwise they stay where they are.
void useOwnJournalStream(const QString &identifier);

// Starts a program whose output is forwarded, on a journal stream of its own
// named after the program, when this process writes to the journal.
bool startWithOwnJournalStream(Lingmo::Base::Process &process,
                                const QString &program,
                                const QStringList &arguments = {});

} // namespace Lingmo
