/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAENTRYTEMPLATES_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAENTRYTEMPLATES_H

#include <QVector>

namespace KUserFeedback {
namespace Console {

class Product;

namespace SchemaEntryTemplates
{
    QVector<Product> availableTemplates();
}

}
}

#endif // KUSERFEEDBACK_CONSOLE_SCHEMAENTRYTEMPLATES_H
