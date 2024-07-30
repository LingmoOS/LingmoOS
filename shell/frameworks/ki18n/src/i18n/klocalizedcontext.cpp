/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Undefine this because we don't want our i18n*() method names to be turned into i18nd*()
#undef TRANSLATION_DOMAIN

#include "klocalizedcontext.h"

#include <klocalizedstring.h>

#include "ki18n_logging.h"

class KLocalizedContextPrivate
{
public:
    QString m_translationDomain;
};

KLocalizedContext::KLocalizedContext(QObject *parent)
    : QObject(parent)
    , d_ptr(new KLocalizedContextPrivate)
{
}

KLocalizedContext::~KLocalizedContext() = default;

QString KLocalizedContext::translationDomain() const
{
    Q_D(const KLocalizedContext);
    return d->m_translationDomain;
}

void KLocalizedContext::setTranslationDomain(const QString &domain)
{
    Q_D(KLocalizedContext);
    if (domain != d->m_translationDomain) {
        d->m_translationDomain = domain;
        Q_EMIT translationDomainChanged(domain);
    }
}

static void subsVariant(KLocalizedString &trMessage, const QVariant &value)
{
    switch (value.userType()) {
    case QMetaType::QString:
        trMessage = trMessage.subs(value.toString());
        break;
    case QMetaType::Int:
        trMessage = trMessage.subs(value.toInt());
        break;
    case QMetaType::Double:
        trMessage = trMessage.subs(value.toDouble());
        break;
    case QMetaType::Char:
        trMessage = trMessage.subs(value.toChar());
        break;
    default:
        if (value.canConvert<QString>()) {
            trMessage = trMessage.subs(value.toString());
        } else {
            trMessage = trMessage.subs(QStringLiteral("???"));
            qCWarning(KI18N) << "couldn't convert" << value << "to translate";
        }
    }
}

static void resolveMessage(KLocalizedString &trMessage,
                           const QVariant &param1,
                           const QVariant &param2,
                           const QVariant &param3,
                           const QVariant &param4,
                           const QVariant &param5,
                           const QVariant &param6,
                           const QVariant &param7,
                           const QVariant &param8,
                           const QVariant &param9,
                           const QVariant &param10 = QVariant())
{
    if (param1.isValid()) {
        subsVariant(trMessage, param1);
    }
    if (param2.isValid()) {
        subsVariant(trMessage, param2);
    }
    if (param3.isValid()) {
        subsVariant(trMessage, param3);
    }
    if (param4.isValid()) {
        subsVariant(trMessage, param4);
    }
    if (param5.isValid()) {
        subsVariant(trMessage, param5);
    }
    if (param6.isValid()) {
        subsVariant(trMessage, param6);
    }
    if (param7.isValid()) {
        subsVariant(trMessage, param7);
    }
    if (param8.isValid()) {
        subsVariant(trMessage, param8);
    }
    if (param9.isValid()) {
        subsVariant(trMessage, param9);
    }
    if (param10.isValid()) {
        subsVariant(trMessage, param10);
    }
}

static void resolvePlural(KLocalizedString &trMessage, const QVariant &param)
{
    trMessage = trMessage.subs(param.toInt());
}

QString KLocalizedContext::i18n(const QString &message,
                                const QVariant &param1,
                                const QVariant &param2,
                                const QVariant &param3,
                                const QVariant &param4,
                                const QVariant &param5,
                                const QVariant &param6,
                                const QVariant &param7,
                                const QVariant &param8,
                                const QVariant &param9,
                                const QVariant &param10) const
{
    if (message.isEmpty()) {
        qCWarning(KI18N) << "i18n() needs at least one parameter";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = ki18nd(d->m_translationDomain.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = ki18n(message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18nc(const QString &context,
                                 const QString &message,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "i18nc() needs at least two arguments";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = ki18ndc(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = ki18nc(context.toUtf8().constData(), message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18np(const QString &singular,
                                 const QString &plural,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18np() needs at least two arguments";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = ki18ndp(d->m_translationDomain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = ki18np(singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ncp(const QString &context,
                                  const QString &singular,
                                  const QString &plural,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18ncp() needs at least three arguments";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage =
            ki18ndcp(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = ki18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18nd(const QString &domain,
                                 const QString &message,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (domain.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "i18nd() needs at least two parameters";
        return QString();
    }

    KLocalizedString trMessage = ki18nd(domain.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ndc(const QString &domain,
                                  const QString &context,
                                  const QString &message,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "i18ndc() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ndc(domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ndp(const QString &domain,
                                  const QString &singular,
                                  const QString &plural,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (domain.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18ndp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ndp(domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ndcp(const QString &domain,
                                   const QString &context,
                                   const QString &singular,
                                   const QString &plural,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18ndcp() needs at least four arguments";
        return QString();
    }

    KLocalizedString trMessage =
        ki18ndcp(domain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

/////////////////////////

QString KLocalizedContext::xi18n(const QString &message,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (message.isEmpty()) {
        qCWarning(KI18N) << "xi18n() needs at least one parameter";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = kxi18nd(d->m_translationDomain.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = kxi18n(message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18nc(const QString &context,
                                  const QString &message,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "xi18nc() needs at least two arguments";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = kxi18ndc(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = kxi18nc(context.toUtf8().constData(), message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18np(const QString &singular,
                                  const QString &plural,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18np() needs at least two arguments";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = kxi18ndp(d->m_translationDomain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = kxi18np(singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ncp(const QString &context,
                                   const QString &singular,
                                   const QString &plural,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18ncp() needs at least three arguments";
        return QString();
    }

    Q_D(const KLocalizedContext);
    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage =
            kxi18ndcp(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = kxi18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18nd(const QString &domain,
                                  const QString &message,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (domain.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "xi18nd() needs at least two parameters";
        return QString();
    }

    KLocalizedString trMessage = kxi18nd(domain.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ndc(const QString &domain,
                                   const QString &context,
                                   const QString &message,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "x18ndc() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = kxi18ndc(domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ndp(const QString &domain,
                                   const QString &singular,
                                   const QString &plural,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (domain.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18ndp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = kxi18ndp(domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ndcp(const QString &domain,
                                    const QString &context,
                                    const QString &singular,
                                    const QString &plural,
                                    const QVariant &param1,
                                    const QVariant &param2,
                                    const QVariant &param3,
                                    const QVariant &param4,
                                    const QVariant &param5,
                                    const QVariant &param6,
                                    const QVariant &param7,
                                    const QVariant &param8,
                                    const QVariant &param9,
                                    const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18ndcp() needs at least four arguments";
        return QString();
    }

    KLocalizedString trMessage =
        kxi18ndcp(domain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

#include "moc_klocalizedcontext.cpp"
