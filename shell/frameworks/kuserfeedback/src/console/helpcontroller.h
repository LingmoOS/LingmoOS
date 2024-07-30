/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_HELPCONTROLLER_H
#define KUSERFEEDBACK_CONSOLE_HELPCONTROLLER_H

class QString;

namespace KUserFeedback {
namespace Console {

/*! Controls the Assistant-based help browser. */
namespace HelpController
{

/*! Returns @c true if Assistant and our help collection are found. */
bool isAvailable();

/*! Open start page of the help collection. */
void openContents();

/*! Opens the specified page of the help collection. */
void openPage(const QString &page);

}

}}

#endif // KUSERFEEDBACK_CONSOLE_HELPCONTROLLER_H
