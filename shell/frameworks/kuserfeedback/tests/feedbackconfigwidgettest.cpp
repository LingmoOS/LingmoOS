/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <feedbackconfigdialog.h>

#include <applicationversionsource.h>
#include <platforminfosource.h>
#include <provider.h>
#include <qtversionsource.h>
#include <screeninfosource.h>

#include <QApplication>
#include <QStandardPaths>

using namespace KUserFeedback;

int main(int argc, char **argv)
{
    QStandardPaths::setTestModeEnabled(true);

    QCoreApplication::setApplicationName(QStringLiteral("feedbackconfigwidgettest"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1984"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("FeedbackConfigWidgetTest"));

    QApplication app(argc, argv);

    Provider provider;
    provider.addDataSource(new ApplicationVersionSource);
    provider.addDataSource(new PlatformInfoSource);
    provider.addDataSource(new ScreenInfoSource);
    provider.addDataSource(new QtVersionSource);

    FeedbackConfigDialog w;
    w.setFeedbackProvider(&provider);
    w.show();
    return app.exec();
}
