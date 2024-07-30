/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <QByteArray>

namespace LinuxProc
{
enum class Check {
    DeletedMarker, // < checks the data without disk IO (purely relies on deleted annotation - process must be running!)
    Stat, // < includes inode check
};

// Checks if the /maps content has deleted libraries, or the executable is deleted.
bool hasMapsDeletedFiles(const QString &exePathString, const QByteArray &maps, Check check);

// Checks if a given path is a library path (this is also true if it has a "(deleted)" qualifier)
// This is a standalone function to ease testing.
bool isLibraryPath(const QString &path);
}
