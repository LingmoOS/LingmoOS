/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#ifndef KYNATIVEFILEDIALOGPRIVATE_H
#define KYNATIVEFILEDIALOGPRIVATE_H

#include <QFileDialog>
#include <QTimer>

namespace Peony {
class DirectoryViewContainer;
}
namespace LINGMOFileDialog {

class KyFileDialogHelper;
class KyNativeFileDialogPrivate
{
    friend class KyNativeFileDialog;
public:
    KyNativeFileDialogPrivate();
    ~KyNativeFileDialogPrivate();

private:
    QStringList typedFiles();

    QStringList nameFilters;
    QFileDialog::AcceptMode acceptMode;
    QFileDialog::FileMode fileMode;
    QFileDialog::Options options;
    Peony::DirectoryViewContainer *m_container = nullptr;
    QString currentInputName;
    QTimer *m_timer = nullptr;
    QDir::Filters filters = QDir::AllEntries;
};
}

#endif // KYNATIVEFILEDIALOGPRIVATE_H
/*
KyNativeFileDialogPrivate::KyNativeFileDialogPrivate()
{
//    Q_Q(KyNativeFileDialog);

    m_timer = new QTimer;
//    m_container = q->m_container;
}

QStringList KyNativeFileDialogPrivate::typedFiles() const
{
    Q_Q(KyNativeFileDialog);
    QStringList files;
    QString editText = q->mk ->text();
    if (!editText.contains(QLatin1Char('"'))) {
#ifdef Q_OS_UNIX
        const QString prefix = q->directory().absolutePath() + QDir::separator();
        if (QFile::exists(prefix + editText))
            files << editText;
        else
            files << qt_tildeExpansion(editText);
#else
        files << editText;
        Q_UNUSED(q)
#endif
    } else {
        // " is used to separate files like so: "file1" "file2" "file3" ...
        // ### need escape character for filenames with quotes (")
        QStringList tokens = editText.split(QLatin1Char('\"'));
        for (int i=0; i<tokens.size(); ++i) {
            if ((i % 2) == 0)
                continue; // Every even token is a separator
#ifdef Q_OS_UNIX
            const QString token = tokens.at(i);
            const QString prefix = q->directory().absolutePath() + QDir::separator();
            if (QFile::exists(prefix + token))
                files << token;
            else
                files << qt_tildeExpansion(token);
#else
            files << toInternal(tokens.at(i));
#endif
        }
    }
    return addDefaultSuffixToFiles(files);

    return QStringList();
}
*/
