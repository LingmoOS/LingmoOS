/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2024 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only
*/

namespace {

class LcIdentifierName
{
    // A proxy class for fast case-insensitive lookup of
    // keys in a QHash
    //
    // Pre-condition: Uppercase letters (QChar::isUpper) must
    // trivially map to their lowercase counterpart. This
    // is the case for all ASCII codepoints.
public:
    LcIdentifierName(const QStringView _name) : name(_name) {};
    QStringView name;
};

constexpr QChar trivialToLower(const QChar &c) {
    if (c.isUpper()) {
        return QChar::fromLatin1(c.toLatin1() ^ ('a' ^ 'A'));
    }
    return c;
}

inline bool operator==(const LcIdentifierName &a, const LcIdentifierName &b)
{
    if (a.name.size() != b.name.size()) {
        return false;
    }
    for (int i = 0; i < a.name.size(); i++) {
        if ((a.name[i] != b.name[i]) && (trivialToLower(a.name[i]) != trivialToLower(b.name[i]))) {
            return false;
        }
    }
    return true;
}

inline size_t qHash(const LcIdentifierName &key, size_t seed = 0)
{
    size_t val = seed;
    for (const auto& c : key.name) {
        val ^= qHash(trivialToLower(c));
    }
    return val;
}

}

