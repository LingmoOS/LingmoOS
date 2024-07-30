/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KLOCALIZEDTRANSLATOR_H
#define KLOCALIZEDTRANSLATOR_H

#include <ki18n_export.h>

#include <QTranslator>

#include <memory>

class KLocalizedTranslatorPrivate;

/**
 * @class KLocalizedTranslator klocalizedtranslator.h <KLocalizedTranslator>
 *
 * @brief A QTranslator using KLocalizedString for translations.
 *
 * This class allows to translate strings in Qt's translation system with KLocalizedString.
 * An example is the translation of a dynamically loaded user interface through QUILoader.
 *
 * To use this Translator install it in the QCoreApplication and provide the translation domain
 * to be used. The Translator can operate for multiple contexts, those needs to be specified.
 *
 * Example for translating a UI loaded through QUILoader:
 * @code
 * // create translator and install in QCoreApplication
 * KLocalizedTranslator *translator = new KLocalizedTranslator(this);
 * QCoreApplication::instance()->installTranslator(translator);
 * translator->setTranslationDomain(QStringLiteral("MyAppsDomain"));
 *
 * // create the QUILoader
 * QUiLoader *loader = new QUiLoader(this);
 * loader->setLanguageChangeEnabled(true);
 *
 * // load the UI
 * QFile uiFile(QStringLiteral("/path/to/userInterface.ui"));
 * uiFile.open(QFile::ReadOnly);
 * QWidget *loadedWidget = loader->load(&uiFile, this);
 * uiFile.close();
 *
 * // the object name of the loaded UI is the context in this case
 * translator->addContextToMonitor(loadedWidget->objectName());
 *
 * // send a LanguageChange event, this will re-translate using our translator
 * QEvent le(QEvent::LanguageChange);
 * QCoreApplication::sendEvent(loadedWidget, &le);
 * @endcode
 *
 * @since 5.0
 **/
class KI18N_EXPORT KLocalizedTranslator : public QTranslator
{
    Q_OBJECT
public:
    explicit KLocalizedTranslator(QObject *parent = nullptr);
    virtual ~KLocalizedTranslator();
    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const override;

    /**
     * Sets the @p translationDomain to be used.
     *
     * The translation domain is required. Without the translation domain any invocation of
     * translate() will be delegated to the base class.
     *
     * @param translationDomain The translation domain to be used.
     **/
    void setTranslationDomain(const QString &translationDomain);

    /**
     * Adds a @p context for which this Translator should be active.
     *
     * The Translator only translates texts with a context matching one of the monitored contexts.
     * If the context is not monitored, the translate() method delegates to the base class.
     *
     * @param context The context for which the Translator should be active
     *
     * @see removeContextToMonitor
     **/
    void addContextToMonitor(const QString &context);

    /**
     * Stop translating for the given @p context.
     *
     * @param context The context for which the Translator should no longer be active
     *
     * @see addContextToMonitor
     **/
    void removeContextToMonitor(const QString &context);

private:
    std::unique_ptr<KLocalizedTranslatorPrivate> const d;
};

#endif // KLOCALIZEDTRANSLATOR_H
