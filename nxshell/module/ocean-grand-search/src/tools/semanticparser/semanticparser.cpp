// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticparser_p.h"

#include <QDebug>

using namespace GrandSearch;

SemanticParserPrivate::SemanticParserPrivate(SemanticParser *parent) : q(parent)
{

}

QString SemanticParser::vectorSearch(const QString &context)
{
    QString ret;
    if (!d->m_vector)
        return ret;
    ret = d->m_vector->Search("ocean-grand-search", context, 50);
    return ret;
}

QString SemanticParser::query(const QString &text)
{
    Q_ASSERT(d->m_querylang);

    QString ret;
    auto reply = d->m_querylang->Query(text);
    ret = reply.value();
    if (reply.isError()) {
        qWarning() << "the parser server return error" << reply.error().message();
        ret.clear();
    }
    return ret;
}

bool SemanticParser::isVectorSupported()
{
    if (!d->m_vector)
        return false;

    return d->m_vector->Enable();
}

bool SemanticParser::isQueryLangSupported()
{
    if (!d->m_querylang)
        return false;

    return d->m_querylang->Enable();
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

bool SemanticParser::connectToVector(const QString &service)
{
    if (d->m_vector)
        return false;

    d->m_vector = new VectorIndex(service, "/org/lingmo/ai/daemon/VectorIndex",
                                    QDBusConnection::sessionBus(), this);
    d->m_vector->setTimeout(60 * 1000);
    return d->m_vector->isValid();
}

bool SemanticParser::connectToQueryLang(const QString &service)
{
    if (d->m_querylang)
        return false;

    d->m_querylang = new QueryLang(service, "/org/lingmo/ai/daemon/QueryLang",
                                    QDBusConnection::sessionBus(), this);
    d->m_querylang->setTimeout(60 * 1000);
    return d->m_querylang->isValid();
}
