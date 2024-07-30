/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QDebug>
#include <kmimetypechooser.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    const QString text = QStringLiteral("Select the MimeTypes you want for this file type.");
    QStringList list({QStringLiteral("inode/directory"), QStringLiteral("text/plain")});

    KMimeTypeChooserDialog dlg(QStringLiteral("Select Mime Types"),
                               text,
                               list,
                               QStringLiteral("text"),
                               QStringList(),
                               KMimeTypeChooser::Comments | KMimeTypeChooser::Patterns | KMimeTypeChooser::EditButton,
                               (QWidget *)nullptr);

    if (dlg.exec() == QDialog::Accepted) {
        qDebug() << dlg.chooser()->patterns();
        qDebug() << dlg.chooser()->mimeTypes().join(QLatin1Char(';'));
    }

    return 0; // app.exec();
}
