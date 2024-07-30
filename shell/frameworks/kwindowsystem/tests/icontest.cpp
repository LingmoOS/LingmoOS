/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "../src/platforms/xcb/netwm.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <private/qtx11extras_p.h>

#include <kwindowsystem.h>
#include <kx11extras.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("WId"), QStringLiteral("window id for the window to take the icon from"), QStringLiteral("[WId]"));
    parser.addHelpOption();
    parser.process(app);

    QWidget window;
    QVBoxLayout *vbox = new QVBoxLayout(&window);

    bool ok = false;
    qulonglong id = parser.positionalArguments().first().toULongLong(&ok);
    if (!ok) {
        // try hex
        id = parser.positionalArguments().first().toULongLong(&ok, 16);
    }
    if (!ok) {
        return 1;
    }
    NETWinInfo info(QX11Info::connection(), id, QX11Info::appRootWindow(), NET::WMIcon, NET::WM2WindowClass | NET::WM2IconPixmap);
    auto addIcons = [&window, vbox, &id, &info](const QString &name, int flag) {
        QLabel *title = new QLabel(name, &window);
        vbox->addWidget(title);
        QIcon icons;
        if (flag & KX11Extras::NETWM) {
            const int *iconSizes = info.iconSizes();
            int index = 0;
            while (iconSizes[index] != 0 && iconSizes[index + 1] != 0) {
                const int width = iconSizes[index++];
                const int height = iconSizes[index++];
                NETIcon ni = info.icon(width, height);
                if (ni.data) {
                    QImage img((uchar *)ni.data, (int)ni.size.width, (int)ni.size.height, QImage::Format_ARGB32);
                    if (!img.isNull()) {
                        icons.addPixmap(QPixmap::fromImage(img));
                    }
                }
            }
        }

        if (flag & KX11Extras::WMHints) {
            icons.addPixmap(KX11Extras::icon(id, 0, 0, false, KX11Extras::WMHints, &info));
        }

        if (flag & KX11Extras::ClassHint) {
            icons = QIcon::fromTheme(QString::fromUtf8(info.windowClassClass()).toLower());
        }
        if (flag & KX11Extras::XApp) {
            icons = QIcon::fromTheme(QLatin1String("xorg"));
        }
        if (icons.isNull()) {
            return;
        }
        QHBoxLayout *layout = new QHBoxLayout();
        const auto sizes = icons.availableSizes();
        for (auto it = sizes.begin(); it != sizes.end(); ++it) {
            const QSize &s = *it;
            QVBoxLayout *v = new QVBoxLayout();
            QLabel *l = new QLabel(QStringLiteral("%1/%2").arg(s.width()).arg(s.height()), &window);
            v->addWidget(l);
            QLabel *p = new QLabel(&window);
            p->setPixmap(icons.pixmap(s));
            v->addWidget(p);
            layout->addLayout(v);
        }
        vbox->addLayout(layout);
    };
    addIcons(QStringLiteral("NetWM"), KX11Extras::NETWM);
    addIcons(QStringLiteral("WMHints"), KX11Extras::WMHints);
    addIcons(QStringLiteral("ClassHint"), KX11Extras::ClassHint);
    addIcons(QStringLiteral("XApp"), KX11Extras::XApp);

    window.show();

    return app.exec();
}
