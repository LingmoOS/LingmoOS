// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/dexportedinterface.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonValue>

#include <cmath>

//#define ALTERNATE_USAGE

DCORE_USE_NAMESPACE

class CustomInterface : public DUtil::DExportedInterface
{
    QVariant invoke(const QString &action, const QString &parameters) const
    {
        QJsonDocument d = QJsonDocument::fromJson(parameters.toUtf8());
        if (action == "pow") {
            return QVariant(pow(d["a"].toDouble(), d["b"].toDouble()));
        }
        return QVariant();
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QDBusConnection::sessionBus().registerService("com.deepin.ExpIntfTest");

#ifndef ALTERNATE_USAGE
    DUtil::DExportedInterface *ei = new DUtil::DExportedInterface();
    ei->registerAction("quit", "quit the application", [&app](QString)->QVariant {
        app.quit();
        return QVariant();
    });

    ei->registerAction("answer", "answer to the ultimate question of life, the universe, and everything",
                       [](QString)->QVariant {return QVariant(42);});

    ei->registerAction("sum", "returns the sum of two integers", [](QString p)->QVariant {
        QJsonDocument d = QJsonDocument::fromJson(p.toUtf8());
        return QVariant(d["a"].toInt() + d["b"].toInt());
    });
#else
    CustomInterface *cei = new CustomInterface();
    cei->registerAction("pow", "raise a number to a power");
#endif

    return app.exec();
}
