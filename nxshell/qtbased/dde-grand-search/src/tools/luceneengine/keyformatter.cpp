// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyformatter.h"

#include <LuceneHeaders.h>
#include <TokenGroup.h>

using namespace GrandSearch;
using namespace Lucene;

KeyFormatter::KeyFormatter() : Formatter(), LuceneObject() {

}

KeyFormatter::~KeyFormatter()
{

}

void KeyFormatter::clear()
{
    token.clear();
    tokens.clear();
}

String KeyFormatter::highlightTerm(const String &originalText, const TokenGroupPtr &tokenGroup)
{
    if (tokenGroup->getTotalScore() == 0) {
        if (!token.isEmpty()) {
            tokens.insert(token);
            token.clear();
        }
    } else {
        token.append(StringUtils::toUTF8(originalText).c_str());
    }

    return originalText;
}
