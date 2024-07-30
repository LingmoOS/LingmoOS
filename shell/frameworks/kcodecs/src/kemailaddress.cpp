/*
    SPDX-FileCopyrightText: 2004 Matt Douhan <matt@fruitsalad.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kemailaddress.h"
#include "kcodecs.h"
#include "kcodecs_debug.h"

#include <QRegularExpression>

using namespace KEmailAddress;

//-----------------------------------------------------------------------------
QStringList KEmailAddress::splitAddressList(const QString &aStr)
{
    // Features:
    // - always ignores quoted characters
    // - ignores everything (including parentheses and commas)
    //   inside quoted strings
    // - supports nested comments
    // - ignores everything (including double quotes and commas)
    //   inside comments

    QStringList list;

    if (aStr.isEmpty()) {
        return list;
    }

    QString addr;
    uint addrstart = 0;
    int commentlevel = 0;
    bool insidequote = false;

    for (int index = 0; index < aStr.length(); index++) {
        // the following conversion to latin1 is o.k. because
        // we can safely ignore all non-latin1 characters
        switch (aStr[index].toLatin1()) {
        case '"': // start or end of quoted string
            if (commentlevel == 0) {
                insidequote = !insidequote;
            }
            break;
        case '(': // start of comment
            if (!insidequote) {
                ++commentlevel;
            }
            break;
        case ')': // end of comment
            if (!insidequote) {
                if (commentlevel > 0) {
                    --commentlevel;
                } else {
                    return list;
                }
            }
            break;
        case '\\': // quoted character
            index++; // ignore the quoted character
            break;
        case ',':
        case ';':
            if (!insidequote && (commentlevel == 0)) {
                addr = aStr.mid(addrstart, index - addrstart);
                if (!addr.isEmpty()) {
                    list += addr.trimmed();
                }
                addrstart = index + 1;
            }
            break;
        }
    }
    // append the last address to the list
    if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, aStr.length() - addrstart);
        if (!addr.isEmpty()) {
            list += addr.trimmed();
        }
    }

    return list;
}

//-----------------------------------------------------------------------------
// Used by KEmailAddress::splitAddress(...) and KEmailAddress::firstEmailAddress(...).
KEmailAddress::EmailParseResult
splitAddressInternal(const QByteArray &address, QByteArray &displayName, QByteArray &addrSpec, QByteArray &comment, bool allowMultipleAddresses)
{
    //  qCDebug(KCODECS_LOG) << "address";
    displayName = "";
    addrSpec = "";
    comment = "";

    if (address.isEmpty()) {
        return AddressEmpty;
    }

    // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
    // The purpose is to extract a displayable string from the mailboxes.
    // Comments in the addr-spec are not handled. No error checking is done.

    enum {
        TopLevel,
        InComment,
        InAngleAddress,
    } context = TopLevel;
    bool inQuotedString = false;
    int commentLevel = 0;
    bool stop = false;

    for (const char *p = address.data(); *p && !stop; ++p) {
        switch (context) {
        case TopLevel: {
            switch (*p) {
            case '"':
                inQuotedString = !inQuotedString;
                displayName += *p;
                break;
            case '(':
                if (!inQuotedString) {
                    context = InComment;
                    commentLevel = 1;
                } else {
                    displayName += *p;
                }
                break;
            case '<':
                if (!inQuotedString) {
                    context = InAngleAddress;
                } else {
                    displayName += *p;
                }
                break;
            case '\\': // quoted character
                displayName += *p;
                ++p; // skip the '\'
                if (*p) {
                    displayName += *p;
                } else {
                    return UnexpectedEnd;
                }
                break;
            case ',':
                if (!inQuotedString) {
                    if (allowMultipleAddresses) {
                        stop = true;
                    } else {
                        return UnexpectedComma;
                    }
                } else {
                    displayName += *p;
                }
                break;
            default:
                displayName += *p;
            }
            break;
        }
        case InComment: {
            switch (*p) {
            case '(':
                ++commentLevel;
                comment += *p;
                break;
            case ')':
                --commentLevel;
                if (commentLevel == 0) {
                    context = TopLevel;
                    comment += ' '; // separate the text of several comments
                } else {
                    comment += *p;
                }
                break;
            case '\\': // quoted character
                comment += *p;
                ++p; // skip the '\'
                if (*p) {
                    comment += *p;
                } else {
                    return UnexpectedEnd;
                }
                break;
            default:
                comment += *p;
            }
            break;
        }
        case InAngleAddress: {
            switch (*p) {
            case '"':
                inQuotedString = !inQuotedString;
                addrSpec += *p;
                break;
            case '>':
                if (!inQuotedString) {
                    context = TopLevel;
                } else {
                    addrSpec += *p;
                }
                break;
            case '\\': // quoted character
                addrSpec += *p;
                ++p; // skip the '\'
                if (*p) {
                    addrSpec += *p;
                } else {
                    return UnexpectedEnd;
                }
                break;
            default:
                addrSpec += *p;
            }
            break;
        }
        } // switch ( context )
    }
    // check for errors
    if (inQuotedString) {
        return UnbalancedQuote;
    }
    if (context == InComment) {
        return UnbalancedParens;
    }
    if (context == InAngleAddress) {
        return UnclosedAngleAddr;
    }

    displayName = displayName.trimmed();
    comment = comment.trimmed();
    addrSpec = addrSpec.trimmed();

    if (addrSpec.isEmpty()) {
        if (displayName.isEmpty()) {
            return NoAddressSpec;
        } else {
            addrSpec = displayName;
            displayName.truncate(0);
        }
    }
    /*
      qCDebug(KCODECS_LOG) << "display-name : \"" << displayName << "\"";
      qCDebug(KCODECS_LOG) << "comment      : \"" << comment << "\"";
      qCDebug(KCODECS_LOG) << "addr-spec    : \"" << addrSpec << "\"";
    */
    return AddressOk;
}

//-----------------------------------------------------------------------------
EmailParseResult KEmailAddress::splitAddress(const QByteArray &address, QByteArray &displayName, QByteArray &addrSpec, QByteArray &comment)
{
    return splitAddressInternal(address, displayName, addrSpec, comment, false /* don't allow multiple addresses */);
}

//-----------------------------------------------------------------------------
EmailParseResult KEmailAddress::splitAddress(const QString &address, QString &displayName, QString &addrSpec, QString &comment)
{
    QByteArray d;
    QByteArray a;
    QByteArray c;
    // FIXME: toUtf8() is probably not safe here, what if the second byte of a multi-byte character
    //        has the same code as one of the ASCII characters that splitAddress uses as delimiters?
    EmailParseResult result = splitAddress(address.toUtf8(), d, a, c);

    if (result == AddressOk) {
        displayName = QString::fromUtf8(d);
        addrSpec = QString::fromUtf8(a);
        comment = QString::fromUtf8(c);
    }
    return result;
}

//-----------------------------------------------------------------------------
EmailParseResult KEmailAddress::isValidAddress(const QString &aStr)
{
    // If we are passed an empty string bail right away no need to process
    // further and waste resources
    if (aStr.isEmpty()) {
        return AddressEmpty;
    }

    // count how many @'s are in the string that is passed to us
    // if 0 or > 1 take action
    // at this point to many @'s cannot bail out right away since
    // @ is allowed in quotes, so we use a bool to keep track
    // and then make a judgment further down in the parser

    bool tooManyAtsFlag = false;

    int atCount = aStr.count(QLatin1Char('@'));
    if (atCount > 1) {
        tooManyAtsFlag = true;
    } else if (atCount == 0) {
        return TooFewAts;
    }

    int dotCount = aStr.count(QLatin1Char('.'));

    // The main parser, try and catch all weird and wonderful
    // mistakes users and/or machines can create

    enum {
        TopLevel,
        InComment,
        InAngleAddress,
    } context = TopLevel;
    bool inQuotedString = false;
    int commentLevel = 0;

    unsigned int strlen = aStr.length();

    for (unsigned int index = 0; index < strlen; index++) {
        switch (context) {
        case TopLevel: {
            switch (aStr[index].toLatin1()) {
            case '"':
                inQuotedString = !inQuotedString;
                break;
            case '(':
                if (!inQuotedString) {
                    context = InComment;
                    commentLevel = 1;
                }
                break;
            case '[':
                if (!inQuotedString) {
                    return InvalidDisplayName;
                }
                break;
            case ']':
                if (!inQuotedString) {
                    return InvalidDisplayName;
                }
                break;
            case ':':
                if (!inQuotedString) {
                    return DisallowedChar;
                }
                break;
            case '<':
                if (!inQuotedString) {
                    context = InAngleAddress;
                }
                break;
            case '\\': // quoted character
                ++index; // skip the '\'
                if ((index + 1) > strlen) {
                    return UnexpectedEnd;
                }
                break;
            case ',':
                if (!inQuotedString) {
                    return UnexpectedComma;
                }
                break;
            case ')':
                if (!inQuotedString) {
                    return UnbalancedParens;
                }
                break;
            case '>':
                if (!inQuotedString) {
                    return UnopenedAngleAddr;
                }
                break;
            case '@':
                if (!inQuotedString) {
                    if (index == 0) { // Missing local part
                        return MissingLocalPart;
                    } else if (index == strlen - 1) {
                        return MissingDomainPart;
                    }
                } else {
                    --atCount;
                    if (atCount == 1) {
                        tooManyAtsFlag = false;
                    }
                }
                break;
            case '.':
                if (inQuotedString) {
                    --dotCount;
                }
                break;
            }
            break;
        }
        case InComment: {
            switch (aStr[index].toLatin1()) {
            case '(':
                ++commentLevel;
                break;
            case ')':
                --commentLevel;
                if (commentLevel == 0) {
                    context = TopLevel;
                }
                break;
            case '\\': // quoted character
                ++index; // skip the '\'
                if ((index + 1) > strlen) {
                    return UnexpectedEnd;
                }
                break;
            }
            break;
        }

        case InAngleAddress: {
            switch (aStr[index].toLatin1()) {
            case ',':
                if (!inQuotedString) {
                    return UnexpectedComma;
                }
                break;
            case '"':
                inQuotedString = !inQuotedString;
                break;
            case '@':
                if (inQuotedString) {
                    --atCount;
                }
                if (atCount == 1) {
                    tooManyAtsFlag = false;
                }
                break;
            case '.':
                if (inQuotedString) {
                    --dotCount;
                }
                break;
            case '>':
                if (!inQuotedString) {
                    context = TopLevel;
                    break;
                }
                break;
            case '\\': // quoted character
                ++index; // skip the '\'
                if ((index + 1) > strlen) {
                    return UnexpectedEnd;
                }
                break;
            }
            break;
        }
        }
    }

    if (dotCount == 0 && !inQuotedString) {
        return TooFewDots;
    }

    if (atCount == 0 && !inQuotedString) {
        return TooFewAts;
    }

    if (inQuotedString) {
        return UnbalancedQuote;
    }

    if (context == InComment) {
        return UnbalancedParens;
    }

    if (context == InAngleAddress) {
        return UnclosedAngleAddr;
    }

    if (tooManyAtsFlag) {
        return TooManyAts;
    }

    return AddressOk;
}

//-----------------------------------------------------------------------------
KEmailAddress::EmailParseResult KEmailAddress::isValidAddressList(const QString &aStr, QString &badAddr)
{
    if (aStr.isEmpty()) {
        return AddressEmpty;
    }

    const QStringList list = splitAddressList(aStr);
    EmailParseResult errorCode = AddressOk;
    auto it = std::find_if(list.cbegin(), list.cend(), [&errorCode](const QString &addr) {
        qCDebug(KCODECS_LOG) << " address" << addr;
        errorCode = isValidAddress(addr);
        return errorCode != AddressOk;
    });
    if (it != list.cend()) {
        badAddr = *it;
    }
    return errorCode;
}

//-----------------------------------------------------------------------------
QString KEmailAddress::emailParseResultToString(EmailParseResult errorCode)
{
    switch (errorCode) {
    case TooManyAts:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "contains more than one @.\n"
            "You will not create valid messages if you do not "
            "change your address.");
    case TooFewAts:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "does not contain a @.\n"
            "You will not create valid messages if you do not "
            "change your address.");
    case AddressEmpty:
        return QObject::tr("You have to enter something in the email address field.");
    case MissingLocalPart:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "does not contain a local part.");
    case MissingDomainPart:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "does not contain a domain part.");
    case UnbalancedParens:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "contains unclosed comments/brackets.");
    case AddressOk:
        return QObject::tr("The email address you entered is valid.");
    case UnclosedAngleAddr:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "contains an unclosed angle bracket.");
    case UnopenedAngleAddr:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "contains too many closing angle brackets.");
    case UnexpectedComma:
        return QObject::tr(
            "The email address you have entered is not valid because it "
            "contains an unexpected comma.");
    case UnexpectedEnd:
        return QObject::tr(
            "The email address you entered is not valid because it ended "
            "unexpectedly.\nThis probably means you have used an escaping "
            "type character like a '\\' as the last character in your "
            "email address.");
    case UnbalancedQuote:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "contains quoted text which does not end.");
    case NoAddressSpec:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "does not seem to contain an actual email address, i.e. "
            "something of the form joe@example.org.");
    case DisallowedChar:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "contains an illegal character.");
    case InvalidDisplayName:
        return QObject::tr(
            "The email address you have entered is not valid because it "
            "contains an invalid display name.");
    case TooFewDots:
        return QObject::tr(
            "The email address you entered is not valid because it "
            "does not contain a \'.\'.\n"
            "You will not create valid messages if you do not "
            "change your address.");
    }
    return QObject::tr("Unknown problem with email address");
}

//-----------------------------------------------------------------------------
bool KEmailAddress::isValidSimpleAddress(const QString &aStr)
{
    // If we are passed an empty string bail right away no need to process further
    // and waste resources
    if (aStr.isEmpty()) {
        return false;
    }

    int atChar = aStr.lastIndexOf(QLatin1Char('@'));
    QString domainPart = aStr.mid(atChar + 1);
    QString localPart = aStr.left(atChar);

    // Both of these parts must be non empty
    // after all we cannot have emails like:
    // @kde.org, or  foo@
    if (localPart.isEmpty() || domainPart.isEmpty()) {
        return false;
    }

    bool inQuotedString = false;
    int atCount = localPart.count(QLatin1Char('@'));

    unsigned int strlen = localPart.length();
    for (unsigned int index = 0; index < strlen; index++) {
        switch (localPart[index].toLatin1()) {
        case '"':
            inQuotedString = !inQuotedString;
            break;
        case '@':
            if (inQuotedString) {
                --atCount;
            }
            break;
        }
    }

    QString addrRx;

    if (localPart[0] == QLatin1Char('\"') || localPart[localPart.length() - 1] == QLatin1Char('\"')) {
        addrRx = QStringLiteral("\"[a-zA-Z@]*[\\w.@-]*[a-zA-Z0-9@]\"@");
    } else {
        addrRx = QStringLiteral("[a-zA-Z]*[~|{}`\\^?=/+*'&%$#!_\\w.-]*[~|{}`\\^?=/+*'&%$#!_a-zA-Z0-9-]@");
    }
    if (domainPart[0] == QLatin1Char('[') || domainPart[domainPart.length() - 1] == QLatin1Char(']')) {
        addrRx += QStringLiteral("\\[[0-9]{1,3}(\\.[0-9]{1,3}){3}\\]");
    } else {
        addrRx += QStringLiteral("[\\w#-]+(\\.[\\w#-]+)*");
    }

    const QRegularExpression rx(QRegularExpression::anchoredPattern(addrRx), QRegularExpression::UseUnicodePropertiesOption);
    return rx.match(aStr).hasMatch();
}

//-----------------------------------------------------------------------------
QString KEmailAddress::simpleEmailAddressErrorMsg()
{
    return QObject::tr(
        "The email address you entered is not valid.\nIt "
        "does not seem to contain an actual email address, i.e. "
        "something of the form joe@example.org.");
}

//-----------------------------------------------------------------------------
QByteArray KEmailAddress::extractEmailAddress(const QByteArray &address)
{
    QString errorMessage;
    return extractEmailAddress(address, errorMessage);
}

QByteArray KEmailAddress::extractEmailAddress(const QByteArray &address, QString &errorMessage)
{
    QByteArray dummy1;
    QByteArray dummy2;
    QByteArray addrSpec;
    const EmailParseResult result = splitAddressInternal(address, dummy1, addrSpec, dummy2, false /* don't allow multiple addresses */);
    if (result != AddressOk) {
        addrSpec = QByteArray();
        if (result != AddressEmpty) {
            errorMessage = emailParseResultToString(result);
            qCDebug(KCODECS_LOG) << "Input:" << address << "\nError:" << errorMessage;
        }
    } else {
        errorMessage.clear();
    }

    return addrSpec;
}

//-----------------------------------------------------------------------------
QString KEmailAddress::extractEmailAddress(const QString &address)
{
    QString errorMessage;
    return extractEmailAddress(address, errorMessage);
}

QString KEmailAddress::extractEmailAddress(const QString &address, QString &errorMessage)
{
    return QString::fromUtf8(extractEmailAddress(address.toUtf8(), errorMessage));
}

//-----------------------------------------------------------------------------
QByteArray KEmailAddress::firstEmailAddress(const QByteArray &addresses)
{
    QString errorMessage;
    return firstEmailAddress(addresses, errorMessage);
}

QByteArray KEmailAddress::firstEmailAddress(const QByteArray &addresses, QString &errorMessage)
{
    QByteArray dummy1;
    QByteArray dummy2;
    QByteArray addrSpec;
    const EmailParseResult result = splitAddressInternal(addresses, dummy1, addrSpec, dummy2, true /* allow multiple addresses */);
    if (result != AddressOk) {
        addrSpec = QByteArray();
        if (result != AddressEmpty) {
            errorMessage = emailParseResultToString(result);
            qCDebug(KCODECS_LOG) << "Input: aStr\nError:" << errorMessage;
        }
    } else {
        errorMessage.clear();
    }

    return addrSpec;
}

//-----------------------------------------------------------------------------
QString KEmailAddress::firstEmailAddress(const QString &addresses)
{
    QString errorMessage;
    return firstEmailAddress(addresses, errorMessage);
}

QString KEmailAddress::firstEmailAddress(const QString &addresses, QString &errorMessage)
{
    return QString::fromUtf8(firstEmailAddress(addresses.toUtf8(), errorMessage));
}

//-----------------------------------------------------------------------------
bool KEmailAddress::extractEmailAddressAndName(const QString &aStr, QString &mail, QString &name)
{
    name.clear();
    mail.clear();

    const int len = aStr.length();
    const char cQuotes = '"';

    bool bInComment = false;
    bool bInQuotesOutsideOfEmail = false;
    int i = 0;
    int iAd = 0;
    int iMailStart = 0;
    int iMailEnd = 0;
    QChar c;
    unsigned int commentstack = 0;

    // Find the '@' of the email address
    // skipping all '@' inside "(...)" comments:
    while (i < len) {
        c = aStr[i];
        if (QLatin1Char('(') == c) {
            ++commentstack;
        }
        if (QLatin1Char(')') == c) {
            --commentstack;
        }
        bInComment = commentstack != 0;
        if (QLatin1Char('"') == c && !bInComment) {
            bInQuotesOutsideOfEmail = !bInQuotesOutsideOfEmail;
        }

        if (!bInComment && !bInQuotesOutsideOfEmail) {
            if (QLatin1Char('@') == c) {
                iAd = i;
                break; // found it
            }
        }
        ++i;
    }

    if (!iAd) {
        // We suppose the user is typing the string manually and just
        // has not finished typing the mail address part.
        // So we take everything that's left of the '<' as name and the rest as mail
        for (i = 0; len > i; ++i) {
            c = aStr[i];
            if (QLatin1Char('<') != c) {
                name.append(c);
            } else {
                break;
            }
        }
        mail = aStr.mid(i + 1);
        if (mail.endsWith(QLatin1Char('>'))) {
            mail.truncate(mail.length() - 1);
        }

    } else {
        // Loop backwards until we find the start of the string
        // or a ',' that is outside of a comment
        //          and outside of quoted text before the leading '<'.
        bInComment = false;
        bInQuotesOutsideOfEmail = false;
        for (i = iAd - 1; 0 <= i; --i) {
            c = aStr[i];
            if (bInComment) {
                if (QLatin1Char('(') == c) {
                    if (!name.isEmpty()) {
                        name.prepend(QLatin1Char(' '));
                    }
                    bInComment = false;
                } else {
                    name.prepend(c); // all comment stuff is part of the name
                }
            } else if (bInQuotesOutsideOfEmail) {
                if (QLatin1Char(cQuotes) == c) {
                    bInQuotesOutsideOfEmail = false;
                } else if (c != QLatin1Char('\\')) {
                    name.prepend(c);
                }
            } else {
                // found the start of this addressee ?
                if (QLatin1Char(',') == c) {
                    break;
                }
                // stuff is before the leading '<' ?
                if (iMailStart) {
                    if (QLatin1Char(cQuotes) == c) {
                        bInQuotesOutsideOfEmail = true; // end of quoted text found
                    } else {
                        name.prepend(c);
                    }
                } else {
                    switch (c.toLatin1()) {
                    case '<':
                        iMailStart = i;
                        break;
                    case ')':
                        if (!name.isEmpty()) {
                            name.prepend(QLatin1Char(' '));
                        }
                        bInComment = true;
                        break;
                    default:
                        if (QLatin1Char(' ') != c) {
                            mail.prepend(c);
                        }
                    }
                }
            }
        }

        name = name.simplified();
        mail = mail.simplified();

        if (mail.isEmpty()) {
            return false;
        }

        mail.append(QLatin1Char('@'));

        // Loop forward until we find the end of the string
        // or a ',' that is outside of a comment
        //          and outside of quoted text behind the trailing '>'.
        bInComment = false;
        bInQuotesOutsideOfEmail = false;
        int parenthesesNesting = 0;
        for (i = iAd + 1; len > i; ++i) {
            c = aStr[i];
            if (bInComment) {
                if (QLatin1Char(')') == c) {
                    if (--parenthesesNesting == 0) {
                        bInComment = false;
                        if (!name.isEmpty()) {
                            name.append(QLatin1Char(' '));
                        }
                    } else {
                        // nested ")", add it
                        name.append(QLatin1Char(')')); // name can't be empty here
                    }
                } else {
                    if (QLatin1Char('(') == c) {
                        // nested "("
                        ++parenthesesNesting;
                    }
                    name.append(c); // all comment stuff is part of the name
                }
            } else if (bInQuotesOutsideOfEmail) {
                if (QLatin1Char(cQuotes) == c) {
                    bInQuotesOutsideOfEmail = false;
                } else if (c != QLatin1Char('\\')) {
                    name.append(c);
                }
            } else {
                // found the end of this addressee ?
                if (QLatin1Char(',') == c) {
                    break;
                }
                // stuff is behind the trailing '>' ?
                if (iMailEnd) {
                    if (QLatin1Char(cQuotes) == c) {
                        bInQuotesOutsideOfEmail = true; // start of quoted text found
                    } else {
                        name.append(c);
                    }
                } else {
                    switch (c.toLatin1()) {
                    case '>':
                        iMailEnd = i;
                        break;
                    case '(':
                        if (!name.isEmpty()) {
                            name.append(QLatin1Char(' '));
                        }
                        if (++parenthesesNesting > 0) {
                            bInComment = true;
                        }
                        break;
                    default:
                        if (QLatin1Char(' ') != c) {
                            mail.append(c);
                        }
                    }
                }
            }
        }
    }

    name = name.simplified();
    mail = mail.simplified();

    return !(name.isEmpty() || mail.isEmpty());
}

//-----------------------------------------------------------------------------
bool KEmailAddress::compareEmail(const QString &email1, const QString &email2, bool matchName)
{
    QString e1Name;
    QString e1Email;
    QString e2Name;
    QString e2Email;

    extractEmailAddressAndName(email1, e1Email, e1Name);
    extractEmailAddressAndName(email2, e2Email, e2Name);

    return e1Email == e2Email && (!matchName || (e1Name == e2Name));
}

//-----------------------------------------------------------------------------
// Used internally by normalizedAddress()
QString removeBidiControlChars(const QString &input)
{
    constexpr QChar LRO(0x202D);
    constexpr QChar RLO(0x202E);
    constexpr QChar LRE(0x202A);
    constexpr QChar RLE(0x202B);
    QString result = input;
    result.remove(LRO);
    result.remove(RLO);
    result.remove(LRE);
    result.remove(RLE);
    return result;
}

QString KEmailAddress::normalizedAddress(const QString &displayName, const QString &addrSpec, const QString &comment)
{
    const QString realDisplayName = removeBidiControlChars(displayName);
    if (realDisplayName.isEmpty() && comment.isEmpty()) {
        return addrSpec;
    } else if (comment.isEmpty()) {
        if (!realDisplayName.startsWith(QLatin1Char('\"'))) {
            return quoteNameIfNecessary(realDisplayName) + QLatin1String(" <") + addrSpec + QLatin1Char('>');
        } else {
            return realDisplayName + QLatin1String(" <") + addrSpec + QLatin1Char('>');
        }
    } else if (realDisplayName.isEmpty()) {
        return quoteNameIfNecessary(comment) + QLatin1String(" <") + addrSpec + QLatin1Char('>');
    } else {
        return realDisplayName + QLatin1String(" (") + comment + QLatin1String(") <") + addrSpec + QLatin1Char('>');
    }
}

//-----------------------------------------------------------------------------
QString KEmailAddress::fromIdn(const QString &addrSpec)
{
    const int atPos = addrSpec.lastIndexOf(QLatin1Char('@'));
    if (atPos == -1) {
        return addrSpec;
    }

    QString idn = QUrl::fromAce(addrSpec.mid(atPos + 1).toLatin1());
    if (idn.isEmpty()) {
        return QString();
    }

    return addrSpec.left(atPos + 1) + idn;
}

//-----------------------------------------------------------------------------
QString KEmailAddress::toIdn(const QString &addrSpec)
{
    const int atPos = addrSpec.lastIndexOf(QLatin1Char('@'));
    if (atPos == -1) {
        return addrSpec;
    }

    QString idn = QLatin1String(QUrl::toAce(addrSpec.mid(atPos + 1)));
    if (idn.isEmpty()) {
        return addrSpec;
    }

    return addrSpec.left(atPos + 1) + idn;
}

//-----------------------------------------------------------------------------
QString KEmailAddress::normalizeAddressesAndDecodeIdn(const QString &str)
{
    //  qCDebug(KCODECS_LOG) << str;
    if (str.isEmpty()) {
        return str;
    }

    const QStringList addressList = splitAddressList(str);
    QStringList normalizedAddressList;

    QByteArray displayName;
    QByteArray addrSpec;
    QByteArray comment;

    for (const auto &addr : addressList) {
        if (!addr.isEmpty()) {
            if (splitAddress(addr.toUtf8(), displayName, addrSpec, comment) == AddressOk) {
                QByteArray cs;
                displayName = KCodecs::decodeRFC2047String(displayName, &cs).toUtf8();
                comment = KCodecs::decodeRFC2047String(comment, &cs).toUtf8();

                normalizedAddressList << normalizedAddress(QString::fromUtf8(displayName), fromIdn(QString::fromUtf8(addrSpec)), QString::fromUtf8(comment));
            }
        }
    }
    /*
      qCDebug(KCODECS_LOG) << "normalizedAddressList: \""
               << normalizedAddressList.join( ", " )
               << "\"";
    */
    return normalizedAddressList.join(QStringLiteral(", "));
}

//-----------------------------------------------------------------------------
QString KEmailAddress::normalizeAddressesAndEncodeIdn(const QString &str)
{
    // qCDebug(KCODECS_LOG) << str;
    if (str.isEmpty()) {
        return str;
    }

    const QStringList addressList = splitAddressList(str);
    QStringList normalizedAddressList;

    QByteArray displayName;
    QByteArray addrSpec;
    QByteArray comment;

    for (const auto &addr : addressList) {
        if (!addr.isEmpty()) {
            if (splitAddress(addr.toUtf8(), displayName, addrSpec, comment) == AddressOk) {
                normalizedAddressList << normalizedAddress(QString::fromUtf8(displayName), toIdn(QString::fromUtf8(addrSpec)), QString::fromUtf8(comment));
            }
        }
    }

    /*
      qCDebug(KCODECS_LOG) << "normalizedAddressList: \""
               << normalizedAddressList.join( ", " )
               << "\"";
    */
    return normalizedAddressList.join(QStringLiteral(", "));
}

//-----------------------------------------------------------------------------
// Escapes unescaped doublequotes in str.
static QString escapeQuotes(const QString &str)
{
    if (str.isEmpty()) {
        return QString();
    }

    QString escaped;
    // reserve enough memory for the worst case ( """..."" -> \"\"\"...\"\" )
    escaped.reserve(2 * str.length());
    unsigned int len = 0;
    for (int i = 0, total = str.length(); i < total; ++i, ++len) {
        const QChar &c = str[i];
        if (c == QLatin1Char('"')) { // unescaped doublequote
            escaped.append(QLatin1Char('\\'));
            ++len;
        } else if (c == QLatin1Char('\\')) { // escaped character
            escaped.append(QLatin1Char('\\'));
            ++len;
            ++i;
            if (i >= str.length()) { // handle trailing '\' gracefully
                break;
            }
        }
        // Keep str[i] as we increase i previously
        escaped.append(str[i]);
    }
    escaped.truncate(len);
    return escaped;
}

//-----------------------------------------------------------------------------
QString KEmailAddress::quoteNameIfNecessary(const QString &str)
{
    if (str.isEmpty()) {
        return str;
    }
    QString quoted = str;

    static const QRegularExpression needQuotes(QStringLiteral("[^ 0-9A-Za-z\\x{0080}-\\x{FFFF}]"));
    // avoid double quoting
    if ((quoted[0] == QLatin1Char('"')) && (quoted[quoted.length() - 1] == QLatin1Char('"'))) {
        quoted = QLatin1String("\"") + escapeQuotes(quoted.mid(1, quoted.length() - 2)) + QLatin1String("\"");
    } else if (quoted.indexOf(needQuotes) != -1) {
        quoted = QLatin1String("\"") + escapeQuotes(quoted) + QLatin1String("\"");
    }

    return quoted;
}

QUrl KEmailAddress::encodeMailtoUrl(const QString &mailbox)
{
    const QByteArray encodedPath = KCodecs::encodeRFC2047String(mailbox, "utf-8");
    QUrl mailtoUrl;
    mailtoUrl.setScheme(QStringLiteral("mailto"));
    mailtoUrl.setPath(QLatin1String(encodedPath));
    return mailtoUrl;
}

QString KEmailAddress::decodeMailtoUrl(const QUrl &mailtoUrl)
{
    Q_ASSERT(mailtoUrl.scheme() == QLatin1String("mailto"));
    return KCodecs::decodeRFC2047String(mailtoUrl.path());
}
