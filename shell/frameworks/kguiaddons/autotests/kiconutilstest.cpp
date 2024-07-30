/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "kiconutilstest.h"
#include <kiconutils.h>

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QTest>

QTEST_MAIN(KIconUtilsTest)

void KIconUtilsTest::addOverlayTest()
{
    QPixmap rectanglePixmap(32, 32);
    rectanglePixmap.fill(Qt::red);

    QIcon icon(rectanglePixmap);

    QPixmap overlay(32, 32);
    overlay.fill(Qt::blue);

    QIcon overlayIcon(overlay);

    QIcon iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::BottomRightCorner);
    QImage result = iconWithOverlay.pixmap(32, 32).toImage();

    int bluePixels = 0;
    int redPixels = 0;

    // Go over the image and count red and blue pixels
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            if (qRed(result.pixel(x, y)) == 255) {
                redPixels++;
            } else if (qBlue(result.pixel(x, y)) == 255) {
                bluePixels++;
            }
        }
    }

    // For icon of size 32x32, the overlay should be 16x16 (=256)
    QCOMPARE(bluePixels, 256);
    QCOMPARE(redPixels, 768);

    // Try different size and position
    rectanglePixmap = rectanglePixmap.scaled(96, 96);

    icon = QIcon(rectanglePixmap);

    overlay = overlay.scaled(96, 96);

    overlayIcon = QIcon(overlay);

    iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::BottomRightCorner);

    // Test if unsetting the overlay works;
    // the result should have just one blue square
    iconWithOverlay = KIconUtils::addOverlay(icon, QIcon(), Qt::BottomRightCorner);

    iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::TopLeftCorner);
    result = iconWithOverlay.pixmap(96, 96).toImage();

    bluePixels = 0;
    redPixels = 0;

    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            if (qRed(result.pixel(x, y)) == 255) {
                redPixels++;
            } else if (qBlue(result.pixel(x, y)) == 255) {
                bluePixels++;
            }
        }
    }

    // 96x96 big icon will have 32x32 big overlay (=1024 blue pixels)
    QCOMPARE(bluePixels, 1024);
    QCOMPARE(redPixels, 8192);

    // Try paint method
    icon = QIcon(rectanglePixmap);
    iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::TopLeftCorner);

    QPixmap a(96, 96);
    QPainter p(&a);
    iconWithOverlay.paint(&p, a.rect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);

    result = a.toImage();

    bluePixels = 0;
    redPixels = 0;

    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            if (qRed(result.pixel(x, y)) == 255) {
                redPixels++;
            } else if (qBlue(result.pixel(x, y)) == 255) {
                bluePixels++;
            }
        }
    }

    // 96x96 big icon will have 32x32 big overlay (=1024 blue pixels)
    QCOMPARE(bluePixels, 1024);
    QCOMPARE(redPixels, 8192);
}

void KIconUtilsTest::addOverlaysTest()
{
    QPixmap rectanglePixmap(32, 32);
    rectanglePixmap.fill(Qt::red);

    QIcon icon(rectanglePixmap);

    QPixmap overlay(32, 32);
    overlay.fill(Qt::blue);

    QIcon overlayIcon(overlay);

    QHash<Qt::Corner, QIcon> overlays;
    overlays.insert(Qt::BottomRightCorner, overlayIcon);
    overlays.insert(Qt::TopLeftCorner, overlayIcon);

    QIcon iconWithOverlay = KIconUtils::addOverlays(icon, overlays);
    QImage result = iconWithOverlay.pixmap(32, 32).toImage();

    int bluePixels = 0;
    int redPixels = 0;

    // Go over the image and count red and blue pixels
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            if (qRed(result.pixel(x, y)) == 255) {
                redPixels++;
            } else if (qBlue(result.pixel(x, y)) == 255) {
                bluePixels++;
            }
        }
    }

    // Two blue overlays in icon size 32x32 would intersect with 16 pixels,
    // so the amount of blue pixels should be 2x256-16 = 496
    QCOMPARE(bluePixels, 496);
    QCOMPARE(redPixels, 528);

    // Try different size

    rectanglePixmap = rectanglePixmap.scaled(96, 96);
    icon = QIcon(rectanglePixmap);
    overlay = overlay.scaled(96, 96);
    overlayIcon = QIcon(overlay);

    // Clear the old sizes first
    overlays.clear();
    overlays.insert(Qt::BottomRightCorner, overlayIcon);
    overlays.insert(Qt::TopRightCorner, overlayIcon);
    overlays.insert(Qt::TopLeftCorner, overlayIcon);

    // Now it will have 3 overlays
    iconWithOverlay = KIconUtils::addOverlays(icon, overlays);

    QPixmap a(96, 96);
    QPainter p(&a);
    iconWithOverlay.paint(&p, a.rect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);

    result = a.toImage();

    bluePixels = 0;
    redPixels = 0;

    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            if (qRed(result.pixel(x, y)) == 255) {
                redPixels++;
            } else if (qBlue(result.pixel(x, y)) == 255) {
                bluePixels++;
            }
        }
    }

    // 96x96 big icon will have 32x32 big overlays (=3072 blue pixels)
    QCOMPARE(bluePixels, 3072);
    QCOMPARE(redPixels, 6144);
}

#include "moc_kiconutilstest.cpp"
