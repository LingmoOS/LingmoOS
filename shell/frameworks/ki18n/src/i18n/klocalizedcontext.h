/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLOCALIZEDCONTEXT_H
#define KLOCALIZEDCONTEXT_H

#include <QObject>
#include <QVariant>
#include <ki18n_export.h>
#include <memory>

/**
 * @class KLocalizedContext klocalizedcontext.h <KLocalizedContext>
 *
 * This class is meant to be used to simplify integration of the KI18n framework
 * in QML.
 *
 * The way to do so, is by creating this object and setting it as a context
 * object:
 *
 * @code
 * QQuickView* view = new QQuickView;
 * view.engine()->rootContext()->setContextObject(new KLocalizedContext(view));
 * @endcode
 *
 * Then i18n*() and xi18n*() functions should be available for use from the code
 * loaded in the engine, for the view.
 *
 * @note Plural functions differ from the C/C++ version. On QML/JS we can get a
 * real value easily. To solve warnings on those cases we'll cast the first argument
 * to make sure it's taken into account for the plural.
 *
 * @since 5.17
 */

class KI18N_EXPORT KLocalizedContext : public QObject
{
    Q_OBJECT

    /**
     * This property only needs to be specified if the context is being run on a library.
     * in an application there is no need to set the translation domain as the application's
     * domain can be used.
     */
    Q_PROPERTY(QString translationDomain READ translationDomain WRITE setTranslationDomain NOTIFY translationDomainChanged)

public:
    explicit KLocalizedContext(QObject *parent = nullptr);
    ~KLocalizedContext() override;

    QString translationDomain() const;
    void setTranslationDomain(const QString &domain);

    Q_INVOKABLE QString i18n(const QString &message,
                             const QVariant &p1 = QVariant(),
                             const QVariant &p2 = QVariant(),
                             const QVariant &p3 = QVariant(),
                             const QVariant &p4 = QVariant(),
                             const QVariant &p5 = QVariant(),
                             const QVariant &p6 = QVariant(),
                             const QVariant &p7 = QVariant(),
                             const QVariant &p8 = QVariant(),
                             const QVariant &p9 = QVariant(),
                             const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18nc(const QString &context,
                              const QString &message,
                              const QVariant &p1 = QVariant(),
                              const QVariant &p2 = QVariant(),
                              const QVariant &p3 = QVariant(),
                              const QVariant &p4 = QVariant(),
                              const QVariant &p5 = QVariant(),
                              const QVariant &p6 = QVariant(),
                              const QVariant &p7 = QVariant(),
                              const QVariant &p8 = QVariant(),
                              const QVariant &p9 = QVariant(),
                              const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18np(const QString &singular,
                              const QString &plural,
                              const QVariant &p1 = QVariant(),
                              const QVariant &p2 = QVariant(),
                              const QVariant &p3 = QVariant(),
                              const QVariant &p4 = QVariant(),
                              const QVariant &p5 = QVariant(),
                              const QVariant &p6 = QVariant(),
                              const QVariant &p7 = QVariant(),
                              const QVariant &p8 = QVariant(),
                              const QVariant &p9 = QVariant(),
                              const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18ncp(const QString &context,
                               const QString &singular,
                               const QString &plural,
                               const QVariant &p1 = QVariant(),
                               const QVariant &p2 = QVariant(),
                               const QVariant &p3 = QVariant(),
                               const QVariant &p4 = QVariant(),
                               const QVariant &p5 = QVariant(),
                               const QVariant &p6 = QVariant(),
                               const QVariant &p7 = QVariant(),
                               const QVariant &p8 = QVariant(),
                               const QVariant &p9 = QVariant(),
                               const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18nd(const QString &domain,
                              const QString &message,
                              const QVariant &p1 = QVariant(),
                              const QVariant &p2 = QVariant(),
                              const QVariant &p3 = QVariant(),
                              const QVariant &p4 = QVariant(),
                              const QVariant &p5 = QVariant(),
                              const QVariant &p6 = QVariant(),
                              const QVariant &p7 = QVariant(),
                              const QVariant &p8 = QVariant(),
                              const QVariant &p9 = QVariant(),
                              const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18ndc(const QString &domain,
                               const QString &context,
                               const QString &message,
                               const QVariant &p1 = QVariant(),
                               const QVariant &p2 = QVariant(),
                               const QVariant &p3 = QVariant(),
                               const QVariant &p4 = QVariant(),
                               const QVariant &p5 = QVariant(),
                               const QVariant &p6 = QVariant(),
                               const QVariant &p7 = QVariant(),
                               const QVariant &p8 = QVariant(),
                               const QVariant &p9 = QVariant(),
                               const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18ndp(const QString &domain,
                               const QString &singular,
                               const QString &plural,
                               const QVariant &p1 = QVariant(),
                               const QVariant &p2 = QVariant(),
                               const QVariant &p3 = QVariant(),
                               const QVariant &p4 = QVariant(),
                               const QVariant &p5 = QVariant(),
                               const QVariant &p6 = QVariant(),
                               const QVariant &p7 = QVariant(),
                               const QVariant &p8 = QVariant(),
                               const QVariant &p9 = QVariant(),
                               const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString i18ndcp(const QString &domain,
                                const QString &context,
                                const QString &singular,
                                const QString &plural,
                                const QVariant &p1 = QVariant(),
                                const QVariant &p2 = QVariant(),
                                const QVariant &p3 = QVariant(),
                                const QVariant &p4 = QVariant(),
                                const QVariant &p5 = QVariant(),
                                const QVariant &p6 = QVariant(),
                                const QVariant &p7 = QVariant(),
                                const QVariant &p8 = QVariant(),
                                const QVariant &p9 = QVariant(),
                                const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18n(const QString &message,
                              const QVariant &p1 = QVariant(),
                              const QVariant &p2 = QVariant(),
                              const QVariant &p3 = QVariant(),
                              const QVariant &p4 = QVariant(),
                              const QVariant &p5 = QVariant(),
                              const QVariant &p6 = QVariant(),
                              const QVariant &p7 = QVariant(),
                              const QVariant &p8 = QVariant(),
                              const QVariant &p9 = QVariant(),
                              const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18nc(const QString &context,
                               const QString &message,
                               const QVariant &p1 = QVariant(),
                               const QVariant &p2 = QVariant(),
                               const QVariant &p3 = QVariant(),
                               const QVariant &p4 = QVariant(),
                               const QVariant &p5 = QVariant(),
                               const QVariant &p6 = QVariant(),
                               const QVariant &p7 = QVariant(),
                               const QVariant &p8 = QVariant(),
                               const QVariant &p9 = QVariant(),
                               const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18np(const QString &singular,
                               const QString &plural,
                               const QVariant &p1 = QVariant(),
                               const QVariant &p2 = QVariant(),
                               const QVariant &p3 = QVariant(),
                               const QVariant &p4 = QVariant(),
                               const QVariant &p5 = QVariant(),
                               const QVariant &p6 = QVariant(),
                               const QVariant &p7 = QVariant(),
                               const QVariant &p8 = QVariant(),
                               const QVariant &p9 = QVariant(),
                               const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18ncp(const QString &context,
                                const QString &singular,
                                const QString &plural,
                                const QVariant &p1 = QVariant(),
                                const QVariant &p2 = QVariant(),
                                const QVariant &p3 = QVariant(),
                                const QVariant &p4 = QVariant(),
                                const QVariant &p5 = QVariant(),
                                const QVariant &p6 = QVariant(),
                                const QVariant &p7 = QVariant(),
                                const QVariant &p8 = QVariant(),
                                const QVariant &p9 = QVariant(),
                                const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18nd(const QString &domain,
                               const QString &message,
                               const QVariant &p1 = QVariant(),
                               const QVariant &p2 = QVariant(),
                               const QVariant &p3 = QVariant(),
                               const QVariant &p4 = QVariant(),
                               const QVariant &p5 = QVariant(),
                               const QVariant &p6 = QVariant(),
                               const QVariant &p7 = QVariant(),
                               const QVariant &p8 = QVariant(),
                               const QVariant &p9 = QVariant(),
                               const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18ndc(const QString &domain,
                                const QString &context,
                                const QString &message,
                                const QVariant &p1 = QVariant(),
                                const QVariant &p2 = QVariant(),
                                const QVariant &p3 = QVariant(),
                                const QVariant &p4 = QVariant(),
                                const QVariant &p5 = QVariant(),
                                const QVariant &p6 = QVariant(),
                                const QVariant &p7 = QVariant(),
                                const QVariant &p8 = QVariant(),
                                const QVariant &p9 = QVariant(),
                                const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18ndp(const QString &domain,
                                const QString &singular,
                                const QString &plural,
                                const QVariant &p1 = QVariant(),
                                const QVariant &p2 = QVariant(),
                                const QVariant &p3 = QVariant(),
                                const QVariant &p4 = QVariant(),
                                const QVariant &p5 = QVariant(),
                                const QVariant &p6 = QVariant(),
                                const QVariant &p7 = QVariant(),
                                const QVariant &p8 = QVariant(),
                                const QVariant &p9 = QVariant(),
                                const QVariant &p10 = QVariant()) const;

    Q_INVOKABLE QString xi18ndcp(const QString &domain,
                                 const QString &context,
                                 const QString &singular,
                                 const QString &plural,
                                 const QVariant &p1 = QVariant(),
                                 const QVariant &p2 = QVariant(),
                                 const QVariant &p3 = QVariant(),
                                 const QVariant &p4 = QVariant(),
                                 const QVariant &p5 = QVariant(),
                                 const QVariant &p6 = QVariant(),
                                 const QVariant &p7 = QVariant(),
                                 const QVariant &p8 = QVariant(),
                                 const QVariant &p9 = QVariant(),
                                 const QVariant &p10 = QVariant()) const;

Q_SIGNALS:
    void translationDomainChanged(const QString &translationDomain);

private:
    std::unique_ptr<class KLocalizedContextPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KLocalizedContext)
};

#endif
