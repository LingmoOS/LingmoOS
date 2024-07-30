/*
    SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// Undefine this because we don't want our i18n*() method names to be turned into i18nd*()
#undef TRANSLATION_DOMAIN

#include "translationcontext.h"

#include <QDebug>

#include <KLocalizedString>

TranslationContext::TranslationContext(QObject *parent)
    : QObject(parent)
{
}

TranslationContext::~TranslationContext()
{
}

QString TranslationContext::domain() const
{
    return m_domain;
}

void TranslationContext::setDomain(const QString &domain)
{
    if (m_domain == domain) {
        return;
    }

    m_domain = domain;
    Q_EMIT domainChanged(domain);
}

QString TranslationContext::i18n(const QString &message,
                                 const QString &param1,
                                 const QString &param2,
                                 const QString &param3,
                                 const QString &param4,
                                 const QString &param5,
                                 const QString &param6,
                                 const QString &param7,
                                 const QString &param8,
                                 const QString &param9,
                                 const QString &param10) const
{
    if (message.isNull()) {
        qWarning() << "i18n() needs at least one parameter";
        return QString();
    }

    KLocalizedString trMessage = ki18nd(m_domain.toUtf8().constData(), message.toUtf8().constData());

    if (!param1.isNull()) {
        trMessage = trMessage.subs(param1);
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

QString TranslationContext::i18nc(const QString &context,
                                  const QString &message,
                                  const QString &param1,
                                  const QString &param2,
                                  const QString &param3,
                                  const QString &param4,
                                  const QString &param5,
                                  const QString &param6,
                                  const QString &param7,
                                  const QString &param8,
                                  const QString &param9,
                                  const QString &param10) const
{
    if (context.isNull() || message.isNull()) {
        qWarning() << "i18nc() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ndc(m_domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    if (!param1.isNull()) {
        trMessage = trMessage.subs(param1);
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

QString TranslationContext::i18np(const QString &singular,
                                  const QString &plural,
                                  const QString &param1,
                                  const QString &param2,
                                  const QString &param3,
                                  const QString &param4,
                                  const QString &param5,
                                  const QString &param6,
                                  const QString &param7,
                                  const QString &param8,
                                  const QString &param9,
                                  const QString &param10) const
{
    if (singular.isNull() || plural.isNull()) {
        qWarning() << "i18np() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ndp(m_domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    if (!param1.isNull()) {
        bool ok;
        int num = param1.toInt(&ok);
        if (ok) {
            trMessage = trMessage.subs(num);
        } else {
            trMessage = trMessage.subs(param1);
        }
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

QString TranslationContext::i18ncp(const QString &context,
                                   const QString &singular,
                                   const QString &plural,
                                   const QString &param1,
                                   const QString &param2,
                                   const QString &param3,
                                   const QString &param4,
                                   const QString &param5,
                                   const QString &param6,
                                   const QString &param7,
                                   const QString &param8,
                                   const QString &param9,
                                   const QString &param10) const
{
    if (context.isNull() || singular.isNull() || plural.isNull()) {
        qWarning() << "i18ncp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage =
        ki18ndcp(m_domain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    if (!param1.isNull()) {
        bool ok;
        int num = param1.toInt(&ok);
        if (ok) {
            trMessage = trMessage.subs(num);
        } else {
            trMessage = trMessage.subs(param1);
        }
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

#include "moc_translationcontext.cpp"
