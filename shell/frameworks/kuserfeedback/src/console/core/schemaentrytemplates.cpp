/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "schemaentrytemplates.h"

#include <core/product.h>

#include <QDebug>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

#include <algorithm>

using namespace KUserFeedback::Console;

QVector<Product> SchemaEntryTemplates::availableTemplates()
{
    QVector<Product> templates;

    auto dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("org.kde.user-feedback/schematemplates"), QStandardPaths::LocateDirectory);
    dirs += QStringLiteral(":/org.kde.user-feedback/schematemplates");
    foreach (const auto &dir, dirs) {
        QDirIterator it(dir, {QStringLiteral("*.schema")}, QDir::Files | QDir::Readable);
        while (it.hasNext()) {
            const auto fileName = it.next();
            QFile f(fileName);
            if (!f.open(QFile::ReadOnly))
                continue;
            const auto ps = Product::fromJson(f.readAll());
            if (ps.isEmpty())
                qWarning() << "Failed to read template" << fileName;
            templates += ps;
        }
    }

    std::sort(templates.begin(), templates.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.name() < rhs.name();
    });

    return templates;
}
