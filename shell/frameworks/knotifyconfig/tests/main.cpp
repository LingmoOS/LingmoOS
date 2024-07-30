/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
*/

#include <KLocalizedString>
#include <QApplication>

#include "knotifytestwindow.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("knotifytest");
    QApplication::setApplicationDisplayName(i18n("KNotifyTest"));

    KNotifyTestWindow *knotifytestwindow = new KNotifyTestWindow;
    knotifytestwindow->show();

    return a.exec();
}
