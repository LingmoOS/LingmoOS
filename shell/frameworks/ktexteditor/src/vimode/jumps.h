/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_JUMPS_H
#define KATEVI_JUMPS_H

#include <ktexteditor/cursor.h>

#include <KConfigGroup>

#include <QList>

namespace KateVi
{
class Jumps
{
public:
    explicit Jumps() = default;
    ~Jumps() = default;

    Jumps(const Jumps &) = delete;
    Jumps &operator=(const Jumps &) = delete;

    void add(const KTextEditor::Cursor cursor);
    KTextEditor::Cursor next(const KTextEditor::Cursor cursor);
    KTextEditor::Cursor prev(const KTextEditor::Cursor cursor);

    void writeSessionConfig(KConfigGroup &config) const;
    void readSessionConfig(const KConfigGroup &config);

private:
    QList<KTextEditor::Cursor> m_jumps;
    QList<KTextEditor::Cursor>::iterator m_current = m_jumps.begin();
};

}

#endif // KATEVI_JUMPS_H
