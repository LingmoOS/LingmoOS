/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpixmapregionselectordialog.h"
#include <QApplication>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc <= 1) {
        std::cerr << "Usage: kpixmapregionselectordialogtest <imageFile>" << std::endl;
        return 1;
    }

    QPixmap pix(QFile::decodeName(argv[1]));
    QImage image = KPixmapRegionSelectorDialog::getSelectedImage(pix, 100, 100);
    image.save(QStringLiteral("output.png"), "PNG");

    return 0;
}
