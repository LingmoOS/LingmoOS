// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INIPARSER_H
#define INIPARSER_H

#include <QTextStream>
#include <QString>
#include <QMap>
#include <QRegularExpression>

enum class ParserError {
    NoError,
    NotFound,
    MismatchedFile,
    InvalidLocation,
    InvalidFormat,
    OpenFailed,
    MissingInfo,
    Parsed,
    InternalError
};

template <typename Value>
class Parser
{
public:
    explicit Parser(QTextStream &stream)
        : m_stream(stream){};
    virtual ~Parser() = default;
    using Groups = QMap<QString, QMap<QString, Value>>;

    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) = delete;

    virtual ParserError parse(Groups &groups) noexcept = 0;
    virtual ParserError addGroup(Groups &groups) noexcept = 0;
    virtual ParserError addEntry(typename Groups::iterator &group) noexcept = 0;
    void skip() noexcept
    {
        while (!m_stream.atEnd() and (m_line.startsWith('#') or m_line.isEmpty())) {
            m_line = m_stream.readLine().trimmed();
        }
    };

protected:
    QTextStream &m_stream;
    QString m_line;
};

inline bool hasNonAsciiAndControlCharacters(const QString &str) noexcept
{
    static const QRegularExpression _matchControlChars = []() {
        QRegularExpression tmp{R"(\p{Cc})"};
        tmp.optimize();
        return tmp;
    }();
    thread_local const auto matchControlChars = _matchControlChars;
    static const QRegularExpression _matchNonAsciiChars = []() {
        QRegularExpression tmp{R"([^\x00-\x7f])"};
        tmp.optimize();
        return tmp;
    }();
    thread_local const auto matchNonAsciiChars = _matchNonAsciiChars;
    return str.contains(matchControlChars) and str.contains(matchNonAsciiChars);
}

inline QDebug operator<<(QDebug debug, const ParserError &v)
{
    QDebugStateSaver saver{debug};
    QString errMsg;
    switch (v) {
    case ParserError::NoError: {
        errMsg = "no error.";
    } break;
    case ParserError::NotFound: {
        errMsg = "file not found.";
    } break;
    case ParserError::MismatchedFile: {
        errMsg = "file type is mismatched.";
    } break;
    case ParserError::InvalidLocation: {
        errMsg = "file location is invalid, please check $XDG_DATA_DIRS.";
    } break;
    case ParserError::OpenFailed: {
        errMsg = "couldn't open the file.";
    } break;
    case ParserError::InvalidFormat: {
        errMsg = "the format of desktopEntry file is invalid.";
    } break;
    case ParserError::MissingInfo: {
        errMsg = "missing required infomation.";
    } break;
    case ParserError::Parsed: {
        errMsg = "this desktop entry is parsed.";
    } break;
    case ParserError::InternalError: {
        errMsg = "internal error of parser.";
    } break;
    }
    debug << errMsg;
    return debug;
}

#endif
