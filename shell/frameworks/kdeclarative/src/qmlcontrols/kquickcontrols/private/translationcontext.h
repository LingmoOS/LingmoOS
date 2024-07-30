/*
    SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef TRANSLATIONCONTEXT_H
#define TRANSLATIONCONTEXT_H

#include <QObject>

class TranslationContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString domain READ domain WRITE setDomain NOTIFY domainChanged)

public:
    explicit TranslationContext(QObject *parent = nullptr);
    ~TranslationContext() override;

    QString domain() const;

public Q_SLOTS:
    void setDomain(const QString &domain);

Q_SIGNALS:
    void domainChanged(const QString &domain);

public:
    Q_INVOKABLE QString i18n(const QString &message,
                             const QString &param1 = QString(),
                             const QString &param2 = QString(),
                             const QString &param3 = QString(),
                             const QString &param4 = QString(),
                             const QString &param5 = QString(),
                             const QString &param6 = QString(),
                             const QString &param7 = QString(),
                             const QString &param8 = QString(),
                             const QString &param9 = QString(),
                             const QString &param10 = QString()) const;

    Q_INVOKABLE QString i18nc(const QString &context,
                              const QString &message,
                              const QString &param1 = QString(),
                              const QString &param2 = QString(),
                              const QString &param3 = QString(),
                              const QString &param4 = QString(),
                              const QString &param5 = QString(),
                              const QString &param6 = QString(),
                              const QString &param7 = QString(),
                              const QString &param8 = QString(),
                              const QString &param9 = QString(),
                              const QString &param10 = QString()) const;

    Q_INVOKABLE QString i18np(const QString &singular,
                              const QString &plural,
                              const QString &param1 = QString(),
                              const QString &param2 = QString(),
                              const QString &param3 = QString(),
                              const QString &param4 = QString(),
                              const QString &param5 = QString(),
                              const QString &param6 = QString(),
                              const QString &param7 = QString(),
                              const QString &param8 = QString(),
                              const QString &param9 = QString(),
                              const QString &param10 = QString()) const;

    Q_INVOKABLE QString i18ncp(const QString &context,
                               const QString &singular,
                               const QString &plural,
                               const QString &param1 = QString(),
                               const QString &param2 = QString(),
                               const QString &param3 = QString(),
                               const QString &param4 = QString(),
                               const QString &param5 = QString(),
                               const QString &param6 = QString(),
                               const QString &param7 = QString(),
                               const QString &param8 = QString(),
                               const QString &param9 = QString(),
                               const QString &param10 = QString()) const;

private:
    Q_DISABLE_COPY(TranslationContext)

    QString m_domain;
};

#endif // TRANSLATIONCONTEXT_H
