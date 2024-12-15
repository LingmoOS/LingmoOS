// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FEATURELIBENGINE_P_H
#define FEATURELIBENGINE_P_H

#include "featurelibengine.h"
#include "keyformatter.h"

#include <lucene++/LuceneHeaders.h>
#include <SimpleFragmenter.h>
#include <QueryScorer.h>
#include <Highlighter.h>

namespace GrandSearch {
class FeatureLibEnginePrivate
{
public:
    explicit FeatureLibEnginePrivate(FeatureLibEngine *qq);
    int conditonsToString(const FeatureLibEngine::QueryConditons &cond, QString &out) const;
    QString packageString(const QString &key, const QStringList &value) const;
    QString packageTime(const QString &key, const QList<QPair<qint64, qint64>> &value) const;
    Lucene::IndexReaderPtr createReader(const QString &cache);
public:
    Lucene::IndexReaderPtr m_reader;
private:
   FeatureLibEngine *q;
};
}

#endif // FEATURELIBENGINE_P_H
