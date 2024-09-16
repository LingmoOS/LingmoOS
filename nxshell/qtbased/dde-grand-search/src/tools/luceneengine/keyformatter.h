// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYFORMATTER_H
#define KEYFORMATTER_H

#include <Formatter.h>

#include <QSet>

namespace GrandSearch {

class KeyFormatter : public Lucene::Formatter, public Lucene::LuceneObject {
public:
    explicit KeyFormatter();
    ~KeyFormatter() override;
public:
    inline QSet<QString> keys() const {
        return tokens;
    }

    void clear();
    Lucene::String highlightTerm(const Lucene::String& originalText,
                                 const Lucene::TokenGroupPtr& tokenGroup) override;
protected:
    QString token;
    QSet<QString> tokens;
};
}

#endif // KEYFORMATTER_H
