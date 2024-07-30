/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <kicondialog.h>

#include <QApplication>
#include <QTextStream>

#include <iostream>

class Listener : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void dialog1Done()
    {
        KIconDialog *dialog = new KIconDialog();
        dialog->setup(KIconLoader::Toolbar, KIconLoader::Action);
        QString icon = dialog->openDialog();
        QTextStream(stdout) << "Icon \"" << icon << "\" was chosen (openDialog)\n";
        delete dialog;

        icon = KIconDialog::getIcon(KIconLoader::Desktop, KIconLoader::MimeType, true, 48, true, nullptr, QStringLiteral("Test dialog"));
        QTextStream(stdout) << "Icon \"" << icon << "\" was chosen (getIcon)\n";
    }

    void iconChosen(const QString &name)
    {
        QTextStream(stdout) << "Icon \"" << name << "\" was chosen (showDialog).\n";
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Listener listener;

    KIconDialog dialog;
    QObject::connect(&dialog, &KIconDialog::newIconName, &listener, &Listener::iconChosen);
    QObject::connect(&dialog, &QDialog::finished, &listener, &Listener::dialog1Done);

    dialog.showDialog();

    return app.exec();
}

#include "kicondialogtest.moc"
