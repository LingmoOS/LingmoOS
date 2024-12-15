// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTENGINE_P_H
#define FULLTEXTENGINE_P_H

#include "fulltextengine.h"
#include "keyformatter.h"

#include <lucene++/LuceneHeaders.h>
#include <SimpleFragmenter.h>
#include <QueryScorer.h>
#include <Highlighter.h>

#include <QSet>

namespace GrandSearch {

class FullTextEnginePrivate
{
public:
    struct KeyContext
    {
        Lucene::AnalyzerPtr analyzer;
        Lucene::FormatterPtr format;
        Lucene::HighlighterPtr lighter;
        Lucene::DocumentPtr doc;
        KeyFormatter *keyFormatter() const {
            return dynamic_cast<KeyFormatter *>(format.get());
        }
    };
public:
    explicit FullTextEnginePrivate(FullTextEngine *qq);
    Lucene::IndexReaderPtr createReader(const QString &cache);
public:
    Lucene::IndexReaderPtr m_reader;
private:
   FullTextEngine *q;
};
}

#endif // FULLTEXTENGINE_P_H
