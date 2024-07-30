/*
    Gettext translation file analyzer

    SPDX-FileCopyrightText: 2007 Montel Laurent <montel@kde.org>
    SPDX-FileCopyrightText: 2009 Jos van den Oever <jos@vandenoever.info>
    SPDX-FileCopyrightText: 2014 Nick Shaforostoff <shaforostoff@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "poextractor.h"
#include <QFile>
#include <fstream>

using namespace KFileMetaData;

POExtractor::POExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

const QStringList supportedMimeTypes = {
    QStringLiteral("text/x-gettext-translation"),
};

QStringList POExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void POExtractor::endMessage()
{
    messages++;
    fuzzy+=isFuzzy;
    untranslated+=(!isTranslated);

    isFuzzy = false;
    isTranslated = false;
    state = WHITESPACE;
}

void POExtractor::handleComment(const char* data, quint32 length)
{
    state = COMMENT;
    if (length >= 8 && strncmp(data, "#, fuzzy", 8) == 0) { // could be better
        isFuzzy = true;
    }
}

void POExtractor::handleLine(const char* data, quint32 length)
{
    if (state == ERROR) {
        return;
    }
    if (state == WHITESPACE) {
        if (length == 0) {
            return;
        }
        if (data[0] != '#') {
            state = COMMENT; //this allows PO files w/o comments
        } else {
            handleComment(data, length);
            return;
        }
    }
    if (state == COMMENT) {
        if (length == 0) {
            state = WHITESPACE;
        } else if (data[0] == '#') {
            handleComment(data, length);
        } else if (length > 7 && strncmp("msgctxt", data, 7) == 0) {
            state = MSGCTXT;
        } else if (length > 7 && strncmp("msgid \"", data, 7) == 0) {
            state = MSGID;
        } else {
            state = ERROR;
        }
        return;
    } else if (length > 1 && data[0] == '"' && data[length-1] == '"'
            && (state == MSGCTXT || state == MSGID || state == MSGSTR
                || state == MSGID_PLURAL)) {
        // continued text field
        isTranslated = state == MSGSTR && length > 2;
    } else if (state == MSGCTXT
            && length > 7 && strncmp("msgid \"", data, 7) == 0) {
        state = MSGID;
    } else if (state == MSGID
            && length > 14 && strncmp("msgid_plural \"", data, 14) == 0) {
        state = MSGID_PLURAL;
    } else if ((state == MSGID || state == MSGID_PLURAL || state == MSGSTR)
            && length > 8 && strncmp("msgstr", data, 6) == 0) {
        state = MSGSTR;
        isTranslated = strncmp(data+length-3, " \"\"", 3) != 0;
    } else if (state == MSGSTR) {
        if (length == 0) {
            endMessage();
        } else if (data[0]=='#' || data[0]=='m') { //allow PO without empty line between entries
            endMessage();
            state = COMMENT;
            handleLine(data, length);
        } else {
            state = ERROR;
        }
    } else {
        state = ERROR;
    }
#if 0
    if (messages > 1 || state != MSGSTR) return;

    // handle special values in the first message
    // assumption is that value takes up only one line
    if (strncmp("\"POT-Creation-Date: ", data, 20) == 0) {
        result->add(Property::TranslationTemplateDate, QByteArray(data + 20, length - 21));
    } else if (strncmp("\"PO-Revision-Date: ", data, 19) == 0) {
        result->add(Property::TranslationLastUpDate, QByteArray(data + 19, length - 20));
    } else if (strncmp("\"Last-Translator: ", data, 18) == 0) {
        result->add(Property::TranslationLastAuthor, QByteArray(data + 18, length - 19));
    }
#endif
}

void POExtractor::extract(ExtractionResult* result)
{
    std::ifstream fstream(QFile::encodeName(result->inputUrl()).constData());
    if (!fstream.is_open()) {
        return;
    }

    result->addType(Type::Text);
    if (!(result->inputFlags() & ExtractionResult::ExtractPlainText)) {
        return;
    }

    state = WHITESPACE;
    messages = 0;
    untranslated = 0;
    fuzzy = 0;
    isFuzzy = false;
    isTranslated = false;

    std::string line;
    int lines = 0;
    while (std::getline(fstream, line)) {
        //TODO add a parsed text of translation units
        //QByteArray arr = QByteArray::fromRawData(line.c_str(), line.size());
        //result->append(QString::fromUtf8(arr));

        handleLine(line.c_str(), line.size());
        lines++;
        
        
        if (messages <= 1 && state == MSGSTR)
        {
            // handle special values in the first message
            // assumption is that value takes up only one line
            if (strncmp("\"POT-Creation-Date: ", line.c_str(), 20) == 0) {
                result->add(Property::TranslationTemplateDate, QByteArray(line.c_str() + 20, line.size() - 21));
            } else if (strncmp("\"PO-Revision-Date: ", line.c_str(), 19) == 0) {
                result->add(Property::TranslationLastUpDate, QByteArray(line.c_str() + 19, line.size() - 20));
            } else if (strncmp("\"Last-Translator: ", line.c_str(), 18) == 0) {
                result->add(Property::TranslationLastAuthor, QString::fromUtf8(QByteArray::fromRawData(line.c_str() + 18, line.size() - 19)));
            }
        }
    }
    handleLine("", 0); //for files with non-empty last line
    messages--;//cause header does not count

    result->add(Property::TranslationUnitsTotal, messages);
    result->add(Property::TranslationUnitsWithTranslation, messages-untranslated);
    result->add(Property::TranslationUnitsWithDraftTranslation, fuzzy);
    result->add(Property::LineCount, lines);
    //TODO WordCount
}

#include "moc_poextractor.cpp"
