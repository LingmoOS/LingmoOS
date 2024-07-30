/*
    SPDX-FileCopyrightText: 2014 Nicolás Alvarez <nicolas.alvarez@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "faceicontest.h"

#include <QApplication>
#include <QListWidget>
#include <QVBoxLayout>
#include <kuser.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    FaceIconTest *mainWin = new FaceIconTest();
    mainWin->show();
    return app.exec();
}
FaceIconTest::FaceIconTest()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    listWidget = new QListWidget(this);
    layout->addWidget(listWidget);

    const QList<KUser> users = KUser::allUsers();
    for (const KUser &u : users) {
        QPixmap pixmap(u.faceIconPath());
        if (pixmap.isNull()) {
            pixmap = QPixmap(QSize(48, 48));
            pixmap.fill();
        } else {
            pixmap = pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        QListWidgetItem *item = new QListWidgetItem(u.loginName(), listWidget);
        item->setData(Qt::DecorationRole, pixmap);
    }
}

#include "moc_faceicontest.cpp"
