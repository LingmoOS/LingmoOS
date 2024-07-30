/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcontacts_debug.h"
#include "vcardparser_p.h"
#include <KCodecs>
#include <QStringDecoder>
#include <QStringEncoder>
#include <functional>

// This cache for QString::fromLatin1() isn't about improving speed, but about reducing memory usage by sharing common strings
class StringCache
{
public:
    QString fromLatin1(const QByteArray &value)
    {
        if (value.isEmpty()) {
            return QString();
        }

        auto it = m_values.constFind(value);
        if (it != m_values.constEnd()) {
            return it.value();
        }

        QString string = QString::fromLatin1(value);
        m_values.insert(value, string);
        return string;
    }

private:
    QHash<QByteArray, QString> m_values;
};

using namespace KContacts;

static void addEscapes(QByteArray &str, bool excludeEscapedComma)
{
    str.replace('\\', "\\\\");
    if (!excludeEscapedComma) {
        str.replace(',', "\\,");
    }
    str.replace('\r', "\\r");
    str.replace('\n', "\\n");
}

static void removeEscapes(QByteArray &str)
{
    // It's more likely that no escape is present, so add fast path
    if (!str.contains('\\')) {
        return;
    }
    str.replace("\\n", "\n");
    str.replace("\\N", "\n");
    str.replace("\\r", "\r");
    str.replace("\\,", ",");
    str.replace("\\\\", "\\");
}

class VCardLineParser
{
public:
    VCardLineParser(StringCache &cache, std::function<QByteArray()> fetchAnotherLine)
        : m_cache(cache)
        , m_fetchAnotherLine(fetchAnotherLine)
    {
    }

    void parseLine(const QByteArray &currentLine, VCardLine *vCardLine);

private:
    void addParameter(const QByteArray &paramKey, const QByteArray &paramValue);

private:
    StringCache &m_cache;
    std::function<QByteArray()> m_fetchAnotherLine;

    VCardLine *m_vCardLine = nullptr;
    QByteArray m_encoding;
    QByteArray m_charset;
};

void VCardLineParser::addParameter(const QByteArray &paramKey, const QByteArray &paramValue)
{
    if (paramKey == "encoding") {
        m_encoding = paramValue.toLower();
    } else if (paramKey == "charset") {
        m_charset = paramValue.toLower();
    }
    // qDebug() << "  add parameter" << paramKey << "    =    " << paramValue;
    m_vCardLine->addParameter(m_cache.fromLatin1(paramKey), m_cache.fromLatin1(paramValue));
}

void VCardLineParser::parseLine(const QByteArray &currentLine, KContacts::VCardLine *vCardLine)
{
    // qDebug() << currentLine;
    m_vCardLine = vCardLine;
    // The syntax is key:value, but the key can contain semicolon-separated parameters, which can contain a ':', so indexOf(':') is wrong.
    // EXAMPLE: ADR;GEO="geo:22.500000,45.099998";LABEL="My Label";TYPE=home:P.O. Box 101;;;Any Town;CA;91921-1234;
    // Therefore we need a small state machine, just the way I like it.
    enum State {
        StateInitial,
        StateParamKey,
        StateParamValue,
        StateQuotedValue,
        StateAfterParamValue,
        StateValue,
    };
    State state = StateInitial;
    const int lineLength = currentLine.length();
    const char *lineData = currentLine.constData(); // to skip length checks from at() in debug mode
    QByteArray paramKey;
    QByteArray paramValue;
    int start = 0;
    int pos = 0;
    for (; pos < lineLength; ++pos) {
        const char ch = lineData[pos];
        const bool colonOrSemicolon = (ch == ';' || ch == ':');
        switch (state) {
        case StateInitial:
            if (colonOrSemicolon) {
                const QByteArray identifier = currentLine.mid(start, pos - start);
                // qDebug() << " identifier" << identifier;
                vCardLine->setIdentifier(m_cache.fromLatin1(identifier));
                start = pos + 1;
            }
            if (ch == ';') {
                state = StateParamKey;
            } else if (ch == ':') {
                state = StateValue;
            } else if (ch == '.') {
                vCardLine->setGroup(m_cache.fromLatin1(currentLine.mid(start, pos - start)));
                start = pos + 1;
            }
            break;
        case StateParamKey:
            if (colonOrSemicolon || ch == '=') {
                paramKey = currentLine.mid(start, pos - start);
                start = pos + 1;
            }
            if (colonOrSemicolon) {
                // correct the so-called 2.1 'standard'
                paramValue = paramKey;
                const QByteArray lowerKey = paramKey.toLower();
                if (lowerKey == "quoted-printable" || lowerKey == "base64") {
                    paramKey = "encoding";
                } else {
                    paramKey = "type";
                }
                addParameter(paramKey, paramValue);
            }
            if (ch == ';') {
                state = StateParamKey;
            } else if (ch == ':') {
                state = StateValue;
            } else if (ch == '=') {
                state = StateParamValue;
            }
            break;
        case StateQuotedValue:
            if (ch == '"' || (ch == ',' && paramKey.toLower() == "type")) {
                // TODO the hack above is for TEL;TYPE=\"voice,home\":... without breaking GEO.... TODO: check spec
                paramValue = currentLine.mid(start, pos - start);
                addParameter(paramKey.toLower(), paramValue);
                start = pos + 1;
                if (ch == '"') {
                    state = StateAfterParamValue; // to avoid duplicating code with StateParamValue, we use this intermediate state for one char
                }
            }
            break;
        case StateParamValue:
            if (colonOrSemicolon || ch == ',') {
                paramValue = currentLine.mid(start, pos - start);
                addParameter(paramKey.toLower(), paramValue);
                start = pos + 1;
            }
            // fall-through intended
            Q_FALLTHROUGH();
        case StateAfterParamValue:
            if (ch == ';') {
                state = StateParamKey;
                start = pos + 1;
            } else if (ch == ':') {
                state = StateValue;
            } else if (pos == start && ch == '"') { // don't treat foo"bar" as quoted - TODO check the vCard 3.0 spec.
                state = StateQuotedValue;
                start = pos + 1;
            }
            break;
        case StateValue:
            Q_UNREACHABLE();
            break;
        }

        if (state == StateValue) {
            break;
        }
    }

    if (state != StateValue) { // invalid line, no ':'
        return;
    }

    QByteArray value = currentLine.mid(pos + 1);
    removeEscapes(value);

    QByteArray output;
    bool wasBase64Encoded = false;

    if (!m_encoding.isEmpty()) {
        // have to decode the data
        if (m_encoding == "b" || m_encoding == "base64") {
            output = QByteArray::fromBase64(value);
            wasBase64Encoded = true;
        } else if (m_encoding == "quoted-printable") {
            // join any qp-folded lines
            while (value.endsWith('=')) {
                value.chop(1); // remove the '='
                value.append(m_fetchAnotherLine());
            }
            KCodecs::quotedPrintableDecode(value, output);
        } else if (m_encoding == "8bit") {
            output = value;
        } else {
            qDebug("Unknown vcard encoding type!");
        }
    } else {
        output = value;
    }

    if (!m_charset.isEmpty()) {
        // have to convert the data
        auto codec = QStringDecoder(m_charset.constData());
        if (codec.isValid()) {
            vCardLine->setValue(QVariant::fromValue<QString>(codec.decode(output)));
        } else {
            vCardLine->setValue(QString::fromUtf8(output));
        }
    } else if (wasBase64Encoded) {
        vCardLine->setValue(output);
    } else {
        vCardLine->setValue(QString::fromUtf8(output));
    }
}

////

VCardParser::VCardParser()
{
}

VCardParser::~VCardParser()
{
}

VCard::List VCardParser::parseVCards(const QByteArray &text)
{
    VCard currentVCard;
    VCard::List vCardList;
    QByteArray currentLine;

    int lineStart = 0;
    int lineEnd = text.indexOf('\n');

    bool inVCard = false;

    StringCache cache;
    for (; lineStart != text.size() + 1;
         lineStart = lineEnd + 1, lineEnd = (text.indexOf('\n', lineStart) == -1) ? text.size() : text.indexOf('\n', lineStart)) {
        QByteArray cur = text.mid(lineStart, lineEnd - lineStart);
        // remove the trailing \r, left from \r\n
        if (cur.endsWith('\r')) {
            cur.chop(1);
        }

        if (cur.startsWith(' ') //
            || cur.startsWith('\t')) { // folded line => append to previous
            currentLine.append(cur.mid(1));
            continue;
        } else {
            if (cur.trimmed().isEmpty()) { // empty line
                continue;
            }
            if (inVCard && !currentLine.isEmpty()) { // now parse the line
                VCardLine vCardLine;

                // Provide a way for the parseVCardLine function to read more lines (for quoted-printable support)
                auto fetchAnotherLine = [&text, &lineStart, &lineEnd, &cur]() -> QByteArray {
                    const QByteArray ret = cur;
                    lineStart = lineEnd + 1;
                    lineEnd = text.indexOf('\n', lineStart);
                    if (lineEnd != -1) {
                        cur = text.mid(lineStart, lineEnd - lineStart);
                        // remove the trailing \r, left from \r\n
                        if (cur.endsWith('\r')) {
                            cur.chop(1);
                        }
                    }
                    return ret;
                };

                VCardLineParser lineParser(cache, fetchAnotherLine);

                lineParser.parseLine(currentLine, &vCardLine);

                currentVCard.addLine(vCardLine);
            }

            // we do not save the start and end tag as vcardline
            if (qstrnicmp(cur.constData(), "begin:vcard", 11) == 0) {
                inVCard = true;
                currentLine.clear();
                currentVCard.clear(); // flush vcard
                continue;
            }

            if (qstrnicmp(cur.constData(), "end:vcard", 9) == 0) {
                inVCard = false;
                vCardList.append(currentVCard);
                currentLine.clear();
                currentVCard.clear(); // flush vcard
                continue;
            }

            currentLine = cur;
        }
    }

    return vCardList;
}

static const int FOLD_WIDTH = 75;

QByteArray VCardParser::createVCards(const VCard::List &list)
{
    QByteArray text;
    QByteArray textLine;
    QString encodingType;
    QStringList params;
    QStringList values;

    VCardLine::List lines;

    bool hasEncoding;

    text.reserve(list.size() * 300); // reserve memory to be more efficient

    // iterate over the cards
    for (const VCard &card : list) {
        text.append("BEGIN:VCARD\r\n");

        QStringList idents = card.identifiers();
        // VERSION must be first
        if (idents.contains(QLatin1String("VERSION"))) {
            const QString str = idents.takeAt(idents.indexOf(QLatin1String("VERSION")));
            idents.prepend(str);
        }

        for (const auto &id : std::as_const(idents)) {
            lines = card.lines(id);

            // iterate over the lines
            for (const VCardLine &vline : std::as_const(lines)) {
                QVariant val = vline.value();
                if (val.isValid()) {
                    if (vline.hasGroup()) {
                        textLine = vline.group().toLatin1() + '.' + vline.identifier().toLatin1();
                    } else {
                        textLine = vline.identifier().toLatin1();
                    }

                    params = vline.parameterList();
                    hasEncoding = false;
                    if (!params.isEmpty()) { // we have parameters
                        for (const QString &param : std::as_const(params)) {
                            if (param == QLatin1String("encoding")) {
                                hasEncoding = true;
                                encodingType = vline.parameter(QStringLiteral("encoding")).toLower();
                            }

                            values = vline.parameters(param);
                            for (const QString &str : std::as_const(values)) {
                                textLine.append(';' + param.toLatin1().toUpper());
                                if (!str.isEmpty()) {
                                    textLine.append('=' + str.toLatin1());
                                }
                            }
                        }
                    }

                    QByteArray input;
                    QByteArray output;
                    bool checkMultibyte = false; // avoid splitting a multibyte character

                    // handle charset
                    const QString charset = vline.parameter(QStringLiteral("charset"));
                    if (!charset.isEmpty()) {
                        // have to convert the data
                        const QString value = vline.value().toString();
                        auto codec = QStringEncoder(charset.toLatin1().constData());
                        if (codec.isValid()) {
                            input = codec.encode(value);
                        } else {
                            checkMultibyte = true;
                            input = value.toUtf8();
                        }
                    } else if (vline.value().userType() == QMetaType::QByteArray) {
                        input = vline.value().toByteArray();
                    } else {
                        checkMultibyte = true;
                        input = vline.value().toString().toUtf8();
                    }

                    // handle encoding
                    if (hasEncoding) { // have to encode the data
                        if (encodingType == QLatin1Char('b')) {
                            checkMultibyte = false;
                            output = input.toBase64();
                        } else if (encodingType == QLatin1String("quoted-printable")) {
                            checkMultibyte = false;
                            KCodecs::quotedPrintableEncode(input, output, false);
                        }
                    } else {
                        output = input;
                    }
                    addEscapes(output, (vline.identifier() == QLatin1String("CATEGORIES") || vline.identifier() == QLatin1String("GEO")));

                    if (!output.isEmpty()) {
                        textLine.append(':' + output);

                        if (textLine.length() > FOLD_WIDTH) { // we have to fold the line
                            if (checkMultibyte) {
                                // RFC 6350: Multi-octet characters MUST remain contiguous.
                                // we know that textLine contains UTF-8 encoded characters
                                int lineLength = 0;
                                for (int i = 0; i < textLine.length(); ++i) {
                                    if ((textLine[i] & 0xC0) == 0xC0) { // a multibyte sequence follows
                                        int sequenceLength = 2;
                                        if ((textLine[i] & 0xE0) == 0xE0) {
                                            sequenceLength = 3;
                                        } else if ((textLine[i] & 0xF0) == 0xF0) {
                                            sequenceLength = 4;
                                        }
                                        if ((lineLength + sequenceLength) > FOLD_WIDTH) {
                                            // the current line would be too long. fold it
                                            text += "\r\n " + textLine.mid(i, sequenceLength);
                                            lineLength = 1 + sequenceLength; // incl. leading space
                                        } else {
                                            text += textLine.mid(i, sequenceLength);
                                            lineLength += sequenceLength;
                                        }
                                        i += sequenceLength - 1;
                                    } else {
                                        text += textLine[i];
                                        ++lineLength;
                                    }
                                    if ((lineLength == FOLD_WIDTH) && (i < (textLine.length() - 1))) {
                                        text += "\r\n ";
                                        lineLength = 1; // leading space
                                    }
                                }
                                text += "\r\n";
                            } else {
                                for (int i = 0; i <= (textLine.length() / FOLD_WIDTH); ++i) {
                                    text.append((i == 0 ? "" : " ") + textLine.mid(i * FOLD_WIDTH, FOLD_WIDTH) + "\r\n");
                                }
                            }
                        } else {
                            text.append(textLine);
                            text.append("\r\n");
                        }
                    }
                }
            }
        }

        text.append("END:VCARD\r\n");
        text.append("\r\n");
    }

    return text;
}
