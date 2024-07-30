/*
    kassistantdialogtest - a test program for the KAssistantDialog class
    SPDX-FileCopyrightText: 1998 Thomas Tanghus <tanghus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <kassistantdialog.h>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication a(argc, argv);

    KAssistantDialog *dlg = new KAssistantDialog();
    QObject::connect(dlg, &QDialog::finished, &a, &QCoreApplication::quit);
    for (int i = 1; i < 11; i++) {
        QWidget *p = new QWidget;
        QString msg = QStringLiteral("This is page %1 out of 10").arg(i);
        QLabel *label = new QLabel(msg, p);
        QHBoxLayout *layout = new QHBoxLayout(p);
        label->setAlignment(Qt::AlignCenter);
        label->setFixedSize(300, 200);
        layout->addWidget(label);
        QString title = QStringLiteral("%1. page").arg(i);
        dlg->addPage(p, title);
    }

    dlg->show();
    return a.exec();
}
