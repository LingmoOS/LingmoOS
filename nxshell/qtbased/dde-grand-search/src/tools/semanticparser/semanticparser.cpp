// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticparser_p.h"

#include <QDebug>

using namespace GrandSearch;

SemanticParserPrivate::SemanticParserPrivate(SemanticParser *parent) : q(parent)
{

}

QString SemanticParser::analyze(const QString &text)
{
    Q_ASSERT(d->m_analyze);

    QString ret;
    auto reply = d->m_analyze->Analyze(text);
    ret = reply.value();
    if (reply.isError()) {
        qWarning() << "the parser server return error" << reply.error().message();
        ret.clear();
    }
    return ret;
}

QString SemanticParser::vectorSearch(const QString &context)
{
    QString ret;
    if (!d->m_vector)
        return ret;
    ret = d->m_vector->Search("dde-grand-search", context, 50);
    return ret;
}

bool SemanticParser::isAnalayzeSupported()
{
    if (!d->m_analyze)
        return false;

    return true;
}

bool SemanticParser::isVectorSupported()
{
    if (!d->m_vector)
        return false;

    return d->m_vector->Enable();
}

SemanticParser::SemanticParser(QObject *parent)
    : QObject(parent)
    , d(new SemanticParserPrivate(this))
{

}

SemanticParser::~SemanticParser()
{
    delete d;
    d = nullptr;
}

bool SemanticParser::connectToAnalyze(const QString &service)
{
    if (d->m_analyze)
        return false;

    d->m_analyze = new AnalyzeServer(service, "/org/deepin/ai/daemon/AnalyzeServer",
                                    QDBusConnection::sessionBus(), this);
    d->m_analyze->setTimeout(60 * 1000);
    return d->m_analyze->isValid();
}

bool SemanticParser::connectToVector(const QString &service)
{
    if (d->m_vector)
        return false;

    d->m_vector = new VectorIndex(service, "/org/deepin/ai/daemon/VectorIndex",
                                    QDBusConnection::sessionBus(), this);
    d->m_vector->setTimeout(60 * 1000);
    return d->m_vector->isValid();
}
