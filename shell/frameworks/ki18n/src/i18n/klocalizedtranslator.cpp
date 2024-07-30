/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "klocalizedtranslator.h"
#include "klocalizedstring.h"

// Qt
#include <QMetaObject>
#include <QMetaProperty>

class KLocalizedTranslatorPrivate
{
public:
    QString translationDomain;
    QSet<QString> monitoredContexts;
};

KLocalizedTranslator::KLocalizedTranslator(QObject *parent)
    : QTranslator(parent)
    , d(new KLocalizedTranslatorPrivate)
{
}

KLocalizedTranslator::~KLocalizedTranslator()
{
}

void KLocalizedTranslator::setTranslationDomain(const QString &translationDomain)
{
    d->translationDomain = translationDomain;
}

void KLocalizedTranslator::addContextToMonitor(const QString &context)
{
    d->monitoredContexts.insert(context);
}

void KLocalizedTranslator::removeContextToMonitor(const QString &context)
{
    d->monitoredContexts.remove(context);
}

QString KLocalizedTranslator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    if (d->translationDomain.isEmpty() || !d->monitoredContexts.contains(QString::fromUtf8(context))) {
        return QTranslator::translate(context, sourceText, disambiguation, n);
    }
    if (qstrlen(disambiguation) == 0) {
        return ki18nd(d->translationDomain.toUtf8().constData(), sourceText).toString();
    } else {
        return ki18ndc(d->translationDomain.toUtf8().constData(), disambiguation, sourceText).toString();
    }
}

#include "moc_klocalizedtranslator.cpp"
