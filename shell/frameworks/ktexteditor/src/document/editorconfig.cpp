/*
    SPDX-FileCopyrightText: 2017 Grzegorz Szymaszek <gszymaszek@short.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "editorconfig.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "katepartdebug.h"

/**
 * @return whether a string value could be converted to a bool value as
 * supported. The value is put in *result.
 */
static bool checkBoolValue(QString val, bool *result)
{
    val = val.trimmed().toLower();
    static const QStringList trueValues{QStringLiteral("1"), QStringLiteral("on"), QStringLiteral("true")};
    if (trueValues.contains(val)) {
        *result = true;
        return true;
    }

    static const QStringList falseValues{QStringLiteral("0"), QStringLiteral("off"), QStringLiteral("false")};
    if (falseValues.contains(val)) {
        *result = false;
        return true;
    }
    return false;
}

/**
 * @return whether a string value could be converted to a integer value. The
 * value is put in *result.
 */
static bool checkIntValue(const QString &val, int *result)
{
    bool ret(false);
    *result = val.toInt(&ret);
    return ret;
}

EditorConfig::EditorConfig(KTextEditor::DocumentPrivate *document)
    : m_document(document)
    , m_handle(editorconfig_handle_init())
{
}

EditorConfig::~EditorConfig()
{
    editorconfig_handle_destroy(m_handle);
}

int EditorConfig::parse()
{
    const int code = editorconfig_parse(m_document->url().toLocalFile().toStdString().c_str(), m_handle);

    if (code != 0) {
        if (code == EDITORCONFIG_PARSE_MEMORY_ERROR) {
            qCDebug(LOG_KTE) << "Failed to parse .editorconfig, memory error occurred";
        } else if (code > 0) {
            qCDebug(LOG_KTE) << "Failed to parse .editorconfig, error in line" << code;
        } else {
            qCDebug(LOG_KTE) << "Failed to parse .editorconfig, unknown error";
        }

        return code;
    }

    // count of found key-value pairs
    const unsigned int count = editorconfig_handle_get_name_value_count(m_handle);

    // if indent_size=tab
    bool setIndentSizeAsTabWidth = false;

    // whether corresponding fields were found in .editorconfig
    bool indentSizeSet = false;
    bool tabWidthSet = false;

    // the following only applies if indent_size=tab and there isn’t tab_width
    int tabWidth = m_document->config()->tabWidth();

    for (unsigned int i = 0; i < count; ++i) {
        // raw values from EditorConfig library
        const char *rawKey = nullptr;
        const char *rawValue = nullptr;

        // buffers for integer/boolean values
        int intValue;
        bool boolValue;

        // fetch next key-value pair
        editorconfig_handle_get_name_value(m_handle, i, &rawKey, &rawValue);

        // and convert to Qt strings
        const QLatin1String key = QLatin1String(rawKey);
        const QLatin1String value = QLatin1String(rawValue);

        if (QLatin1String("charset") == key) {
            m_document->setEncoding(value);
        } else if (QLatin1String("end_of_line") == key) {
            QStringList eols;

            // NOTE: EOLs are declared in Kate::TextBuffer::EndOfLineMode
            eols << QStringLiteral("lf") << QStringLiteral("crlf") << QStringLiteral("cr");

            if ((intValue = eols.indexOf(value)) != -1) {
                m_document->config()->setEol(intValue);
                m_document->config()->setAllowEolDetection(false);
            } else {
                qCDebug(LOG_KTE) << "End of line in .editorconfig other than unix/dos/mac";
            }
        } else if (QLatin1String("indent_size") == key) {
            if (QLatin1String("tab") == value) {
                setIndentSizeAsTabWidth = true;
            } else if (checkIntValue(value, &intValue)) {
                m_document->config()->setIndentationWidth(intValue);
                indentSizeSet = true;
            } else {
                qCDebug(LOG_KTE) << "Indent size in .editorconfig not a number, nor tab";
            }
        } else if (QLatin1String("indent_style") == key) {
            if (QLatin1String("tab") == value) {
                m_document->config()->setReplaceTabsDyn(false);
            } else if (QLatin1String("space") == value) {
                m_document->config()->setReplaceTabsDyn(true);
            } else {
                qCDebug(LOG_KTE) << "Indent style in .editorconfig other than tab or space";
            }
        } else if (QLatin1String("insert_final_newline") == key && checkBoolValue(value, &boolValue)) {
            m_document->config()->setNewLineAtEof(boolValue);
        } else if (QLatin1String("max_line_length") == key && checkIntValue(value, &intValue)) {
            m_document->config()->setWordWrapAt(intValue);
        } else if (QLatin1String("tab_width") == key && checkIntValue(value, &intValue)) {
            m_document->config()->setTabWidth(intValue);
            tabWidth = intValue;
            tabWidthSet = true;
        } else if (QLatin1String("trim_trailing_whitespace") == key && checkBoolValue(value, &boolValue)) {
            if (boolValue) {
                m_document->config()->setRemoveSpaces(1 /* remove trailing spaces of modified lines (line modification system) */);
            } else {
                m_document->config()->setRemoveSpaces(0);
            }
        }
    }

    if (setIndentSizeAsTabWidth) {
        m_document->config()->setIndentationWidth(tabWidth);
    } else if (!tabWidthSet && indentSizeSet) {
        m_document->config()->setTabWidth(m_document->config()->indentationWidth());
    }

    return 0;
}
