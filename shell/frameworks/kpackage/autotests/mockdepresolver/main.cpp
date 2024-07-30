/*
    SPDX-FileCopyrightText: 2016 Bhushan Shah <bshah@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QDebug>
#include <QUrl>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Q_ASSERT(app.arguments().count() == 2);

    const QUrl url(app.arguments().constLast());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("mock"));

    // This is very basic dep resolver used for mocking in tests
    // if asked to install invalidapp, will fail
    // if asked to install validdep, will pass
    const QString componentName = url.host();
    if (componentName.isEmpty()) {
        qWarning() << "wrongly formatted URI" << url;
        return 1;
    }

    if (componentName == QStringLiteral("invaliddep")) {
        qWarning() << "package asked to install invalid dep, bailing out";
        return 1;
    }

    if (componentName.startsWith(QLatin1String("validdep"))) {
        qWarning() << "asked to install valid dep, success!";
        return 0;
    }

    qWarning() << "Assuming provided package is not available";
    return 1;
}
