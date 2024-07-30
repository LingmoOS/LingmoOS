/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include <QApplication>

#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow mw;
    mw.show();

    return app.exec();
}