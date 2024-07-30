/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDebug>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>

#include "../systemclipboard/ksystemclipboard.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    auto clip = KSystemClipboard::instance();
    QObject::connect(clip, &KSystemClipboard::changed, &app, [clip](QClipboard::Mode mode) {
        if (mode != QClipboard::Clipboard) {
            return;
        }
        auto dbg = qDebug();
        dbg << "New clipboard content: ";

        const QMimeData *mime = clip->mimeData(QClipboard::Clipboard);

        if (mime) {
            if (mime->hasText()) {
                dbg << "text data:" << mime->text();
            } else if (mime->hasImage()) {
                const QImage image = qvariant_cast<QImage>(mime->imageData());
                dbg << "image data: " << image.size();
            } else {
                dbg << "data: " << mime->formats();
            }
        } else {
            dbg << "[empty]";
        }
    });

    qDebug() << "Watching for new clipboard content...";

    return app.exec();
}
