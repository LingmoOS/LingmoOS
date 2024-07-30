/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// We don't want i18n to be expanded to i18nd here
#undef TRANSLATION_DOMAIN

#include <cstdlib>

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QLibrary>
#include <QList>
#include <QMutexLocker>
#include <QPluginLoader>
#include <QRecursiveMutex>
#include <QStandardPaths>
#include <QStringList>

#include <common_helpers_p.h>
#include <kcatalog_p.h>
#include <klocalizedstring.h>
#include <ktranscript_p.h>
#include <kuitsetup_p.h>

#include "ki18n_logging.h"

// Truncate string, for output of long messages.
static QString shortenMessage(const QString &str)
{
    const int maxlen = 20;
    if (str.length() <= maxlen) {
        return str;
    } else {
        return QStringView(str).left(maxlen) + QLatin1String("...");
    }
}

static void splitLocale(const QString &aLocale, QString &language, QString &country, QString &modifier, QString &charset)
{
    QString locale = aLocale;

    language.clear();
    country.clear();
    modifier.clear();
    charset.clear();

    // In case there are several concatenated locale specifications,
    // truncate all but first.
    int f = locale.indexOf(QLatin1Char(':'));
    if (f >= 0) {
        locale.truncate(f);
    }

    // now decompose into [language[_territory][.codeset][@modifier]]
    f = locale.indexOf(QLatin1Char('@'));
    if (f >= 0) {
        modifier = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('.'));
    if (f >= 0) {
        charset = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('_'));
    if (f >= 0) {
        country = locale.mid(f + 1);
        locale.truncate(f);
    }

    language = locale;
}

static void appendLocaleString(QStringList &languages, const QString &value)
{
    // Process the value to create possible combinations.
    QString language;
    QString country;
    QString modifier;
    QString charset;
    splitLocale(value, language, country, modifier, charset);

    if (language.isEmpty()) {
        return;
    }

    if (!country.isEmpty() && !modifier.isEmpty()) {
        languages += language + QLatin1Char('_') + country + QLatin1Char('@') + modifier;
    }
    // NOTE: Priority is unclear in case both the country and
    // the modifier are present. Should really language@modifier be of
    // higher priority than language_country?
    // In at least one case (Serbian language), it is better this way.
    if (!modifier.isEmpty()) {
        languages += language + QLatin1Char('@') + modifier;
    }
    if (!country.isEmpty()) {
        languages += language + QLatin1Char('_') + country;
    }
    languages += language;
}

static void appendLanguagesFromVariable(QStringList &languages, const char *envar, bool isList = false)
{
    QByteArray qenvar(qgetenv(envar));
    if (!qenvar.isEmpty()) {
        QString value = QFile::decodeName(qenvar);
        if (isList) {
            const auto listLanguages = value.split(QLatin1Char(':'), Qt::SkipEmptyParts);
            for (const QString &v : listLanguages) {
                appendLocaleString(languages, v);
            }
        } else {
            appendLocaleString(languages, value);
        }
    }
}

#if !defined(Q_OS_UNIX) || defined(Q_OS_ANDROID)
static void appendLanguagesFromQLocale(QStringList &languages, const QLocale &locale)
{
    const QStringList uiLangs = locale.uiLanguages();
    for (QString value : uiLangs) { // no const ref because of replace() below
        appendLocaleString(languages, value.replace(QLatin1Char('-'), QLatin1Char('_')));
    }
}
#endif

// Extract the first country code from a list of language_COUNTRY strings.
// Country code is converted to all lower case letters.
static QString extractCountry(const QStringList &languages)
{
    QString country;
    for (const QString &language : languages) {
        int pos1 = language.indexOf(QLatin1Char('_'));
        if (pos1 >= 0) {
            ++pos1;
            int pos2 = pos1;
            while (pos2 < language.length() && language[pos2].isLetter()) {
                ++pos2;
            }
            country = language.mid(pos1, pos2 - pos1);
            break;
        }
    }
    country = country.toLower();
    return country;
}

typedef qulonglong pluraln;
typedef qlonglong intn;
typedef qulonglong uintn;
typedef double realn;

class KLocalizedStringPrivate
{
    friend class KLocalizedString;

    QByteArray domain;
    QStringList languages;
    Kuit::VisualFormat format;
    QByteArray context;
    QByteArray text;
    QByteArray plural;
    QStringList arguments;
    QList<QVariant> values;
    QHash<int, KLocalizedString> klsArguments;
    QHash<int, int> klsArgumentFieldWidths;
    QHash<int, QChar> klsArgumentFillChars;
    bool numberSet;
    pluraln number;
    int numberOrdinal;
    QHash<QString, QString> dynamicContext;
    bool markupAware;
    bool relaxedSubs;

    KLocalizedStringPrivate()
        : format()
        , numberSet(false)
        , markupAware(false)
        , relaxedSubs(false)
    {
    }

    static void translateRaw(const QByteArray &domain,
                             const QStringList &languages,
                             const QByteArray &msgctxt,
                             const QByteArray &msgid,
                             const QByteArray &msgid_plural,
                             qulonglong n,
                             QString &language,
                             QString &translation);

    QString toString(const QByteArray &domain, const QStringList &languages, Kuit::VisualFormat format, bool isArgument = false) const;
    QString substituteSimple(const QString &translation, const QStringList &arguments, QChar plchar = QLatin1Char('%'), bool isPartial = false) const;
    QString formatMarkup(const QByteArray &domain, const QString &language, const QString &context, const QString &text, Kuit::VisualFormat format) const;
    QString substituteTranscript(const QString &scriptedTranslation,
                                 const QString &language,
                                 const QString &country,
                                 const QString &ordinaryTranslation,
                                 const QStringList &arguments,
                                 const QList<QVariant> &values,
                                 bool &fallback) const;
    int resolveInterpolation(const QString &scriptedTranslation,
                             int pos,
                             const QString &language,
                             const QString &country,
                             const QString &ordinaryTranslation,
                             const QStringList &arguments,
                             const QList<QVariant> &values,
                             QString &result,
                             bool &fallback) const;
    QVariant segmentToValue(const QString &segment) const;
    QString postTranscript(const QString &pcall,
                           const QString &language,
                           const QString &country,
                           const QString &finalTranslation,
                           const QStringList &arguments,
                           const QList<QVariant> &values) const;

    static const KCatalog &getCatalog(const QByteArray &domain, const QString &language);
    static void locateScriptingModule(const QByteArray &domain, const QString &language);

    static void loadTranscript();

    void checkNumber(pluraln a)
    {
        if (!plural.isEmpty() && !numberSet) {
            number = a;
            numberSet = true;
            numberOrdinal = arguments.size();
        }
    }
};

typedef QHash<QString, KCatalog *> KCatalogPtrHash;

class KLocalizedStringPrivateStatics
{
public:
    QHash<QByteArray, KCatalogPtrHash> catalogs;
    QStringList languages;

    QByteArray ourDomain;
    QByteArray applicationDomain;
    const QString codeLanguage;
    QStringList localeLanguages;

    const QString theFence;
    const QString startInterp;
    const QString endInterp;
    const QChar scriptPlchar;
    const QChar scriptVachar;

    const QString scriptDir;
    QHash<QString, QList<QByteArray>> scriptModules;
    QList<QStringList> scriptModulesToLoad;

    bool loadTranscriptCalled;
    KTranscript *ktrs;

    QHash<QString, KuitFormatter *> formatters;

    QList<QByteArray> qtDomains;
    QList<int> qtDomainInsertCount;

    QRecursiveMutex klspMutex;

    KLocalizedStringPrivateStatics();
    ~KLocalizedStringPrivateStatics();

    void initializeLocaleLanguages();
};

KLocalizedStringPrivateStatics::KLocalizedStringPrivateStatics()
    : catalogs()
    , languages()

    , ourDomain(QByteArrayLiteral("ki18n6"))
    , applicationDomain()
    , codeLanguage(QStringLiteral("en_US"))
    , localeLanguages()

    , theFence(QStringLiteral("|/|"))
    , startInterp(QStringLiteral("$["))
    , endInterp(QStringLiteral("]"))
    , scriptPlchar(QLatin1Char('%'))
    , scriptVachar(QLatin1Char('^'))

    , scriptDir(QStringLiteral("LC_SCRIPTS"))
    , scriptModules()
    , scriptModulesToLoad()

    , loadTranscriptCalled(false)
    , ktrs(nullptr)

    , formatters()

    , qtDomains()
    , qtDomainInsertCount()
{
    initializeLocaleLanguages();
    languages = localeLanguages;
}

KLocalizedStringPrivateStatics::~KLocalizedStringPrivateStatics()
{
    for (const KCatalogPtrHash &languageCatalogs : std::as_const(catalogs)) {
        qDeleteAll(languageCatalogs);
    }
    // ktrs is handled by QLibrary.
    // delete ktrs;
    qDeleteAll(formatters);
}

Q_GLOBAL_STATIC(KLocalizedStringPrivateStatics, staticsKLSP)

void KLocalizedStringPrivateStatics::initializeLocaleLanguages()
{
    QMutexLocker lock(&klspMutex);

    // Collect languages by same order of priority as for gettext(3).
    // LANGUAGE contains list of language codes, not locale string.
    appendLanguagesFromVariable(localeLanguages, "LANGUAGE", true);
    appendLanguagesFromVariable(localeLanguages, "LC_ALL");
    appendLanguagesFromVariable(localeLanguages, "LC_MESSAGES");
    appendLanguagesFromVariable(localeLanguages, "LANG");
#if !defined(Q_OS_UNIX) || defined(Q_OS_ANDROID)
    // For non UNIX platforms the environment variables might not
    // suffice so we add system locale UI languages, too.
    appendLanguagesFromQLocale(localeLanguages, QLocale::system());
#endif
}

KLocalizedString::KLocalizedString()
    : d(new KLocalizedStringPrivate)
{
}

KLocalizedString::KLocalizedString(const char *domain, const char *context, const char *text, const char *plural, bool markupAware)
    : d(new KLocalizedStringPrivate)
{
    d->domain = domain;
    d->languages.clear();
    d->format = Kuit::UndefinedFormat;
    d->context = context;
    d->text = text;
    d->plural = plural;
    d->numberSet = false;
    d->number = 0;
    d->numberOrdinal = 0;
    d->markupAware = markupAware;
    d->relaxedSubs = false;
}

KLocalizedString::KLocalizedString(const KLocalizedString &rhs)
    : d(new KLocalizedStringPrivate(*rhs.d))
{
}

KLocalizedString &KLocalizedString::operator=(const KLocalizedString &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }
    return *this;
}

KLocalizedString::~KLocalizedString() = default;

bool KLocalizedString::isEmpty() const
{
    return d->text.isEmpty();
}

void KLocalizedStringPrivate::translateRaw(const QByteArray &domain,
                                           const QStringList &languages,
                                           const QByteArray &msgctxt,
                                           const QByteArray &msgid,
                                           const QByteArray &msgid_plural,
                                           qulonglong n,
                                           QString &language,
                                           QString &msgstr)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    // Empty msgid would result in returning the catalog header,
    // which is never intended, so warn and return empty translation.
    if (msgid.isNull() || msgid.isEmpty()) {
        qCWarning(KI18N) << "KLocalizedString: "
                            "Trying to look up translation of \"\", fix the code.";
        language.clear();
        msgstr.clear();
        return;
    }
    // Gettext semantics allows empty context, but it is pointless, so warn.
    if (!msgctxt.isNull() && msgctxt.isEmpty()) {
        qCWarning(KI18N) << "KLocalizedString: "
                            "Using \"\" as context, fix the code.";
    }
    // Gettext semantics allows empty plural, but it is pointless, so warn.
    if (!msgid_plural.isNull() && msgid_plural.isEmpty()) {
        qCWarning(KI18N) << "KLocalizedString: "
                            "Using \"\" as plural text, fix the code.";
    }

    // Set translation to text in code language, in case no translation found.
    msgstr = msgid_plural.isNull() || n == 1 ? QString::fromUtf8(msgid) : QString::fromUtf8(msgid_plural);
    language = s->codeLanguage;

    if (domain.isEmpty()) {
        qCWarning(KI18N) << "KLocalizedString: Domain is not set for this string, translation will not work. Please see https://api.kde.org/frameworks/ki18n/html/prg_guide.html msgid:" << msgid << "msgid_plural:" << msgid_plural
                         << "msgctxt:" << msgctxt;
        return;
    }

    // Languages are ordered from highest to lowest priority.
    for (const QString &testLanguage : languages) {
        // If code language reached, no catalog lookup is needed.
        if (testLanguage == s->codeLanguage) {
            return;
        }
        const KCatalog &catalog = getCatalog(domain, testLanguage);
        QString testMsgstr;
        if (!msgctxt.isNull() && !msgid_plural.isNull()) {
            testMsgstr = catalog.translate(msgctxt, msgid, msgid_plural, n);
        } else if (!msgid_plural.isNull()) {
            testMsgstr = catalog.translate(msgid, msgid_plural, n);
        } else if (!msgctxt.isNull()) {
            testMsgstr = catalog.translate(msgctxt, msgid);
        } else {
            testMsgstr = catalog.translate(msgid);
        }
        if (!testMsgstr.isEmpty()) {
            // Translation found.
            language = testLanguage;
            msgstr = testMsgstr;
            return;
        }
    }
}

QString KLocalizedString::toString() const
{
    return d->toString(d->domain, d->languages, d->format);
}

QString KLocalizedString::toString(const char *domain) const
{
    return d->toString(domain, d->languages, d->format);
}

QString KLocalizedString::toString(const QStringList &languages) const
{
    return d->toString(d->domain, languages, d->format);
}

QString KLocalizedString::toString(Kuit::VisualFormat format) const
{
    return d->toString(d->domain, d->languages, format);
}

QString KLocalizedStringPrivate::toString(const QByteArray &domain, const QStringList &languages, Kuit::VisualFormat format, bool isArgument) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    // Assure the message has been supplied.
    if (text.isEmpty()) {
        qCWarning(KI18N) << "Trying to convert empty KLocalizedString to QString.";
#ifndef NDEBUG
        return QStringLiteral("(I18N_EMPTY_MESSAGE)");
#else
        return QString();
#endif
    }

    // Check whether plural argument has been supplied, if message has plural.
    if (!plural.isEmpty() && !numberSet) {
        qCWarning(KI18N) << "Plural argument to message" << shortenMessage(QString::fromUtf8(text)) << "not supplied before conversion.";
    }

    // Resolve inputs.
    QByteArray resolvedDomain = domain;
    if (resolvedDomain.isEmpty()) {
        resolvedDomain = s->applicationDomain;
    }
    QStringList resolvedLanguages = languages;
    if (resolvedLanguages.isEmpty()) {
        resolvedLanguages = s->languages;
    }
    Kuit::VisualFormat resolvedFormat = format;

    // Get raw translation.
    QString language;
    QString rawTranslation;
    translateRaw(resolvedDomain, resolvedLanguages, context, text, plural, number, language, rawTranslation);
    QString country = extractCountry(resolvedLanguages);

    // Set ordinary translation and possibly scripted translation.
    QString translation;
    QString scriptedTranslation;
    int fencePos = rawTranslation.indexOf(s->theFence);
    if (fencePos > 0) {
        // Script fence has been found, strip the scripted from the
        // ordinary translation.
        translation = rawTranslation.left(fencePos);

        // Scripted translation.
        scriptedTranslation = rawTranslation.mid(fencePos + s->theFence.length());

        // Try to initialize Transcript if not initialized and script not empty.
        // FIXME: And also if Transcript not disabled: where to configure this?
        if (!s->loadTranscriptCalled && !scriptedTranslation.isEmpty()) {
            loadTranscript();

            // Definitions from this library's scripting module
            // must be available to all other modules.
            // So force creation of this library's catalog here,
            // to make sure the scripting module is loaded.
            getCatalog(s->ourDomain, language);
        }
    } else if (fencePos < 0) {
        // No script fence, use translation as is.
        translation = rawTranslation;
    } else { // fencePos == 0
        // The msgstr starts with the script fence, no ordinary translation.
        // This is not allowed, consider message not translated.
        qCWarning(KI18N) << "Scripted message" << shortenMessage(translation) << "without ordinary translation, discarded.";
        translation = plural.isEmpty() || number == 1 ? QString::fromUtf8(text) : QString::fromUtf8(plural);
    }

    // Resolve substituted KLocalizedString arguments.
    QStringList resolvedArguments;
    QList<QVariant> resolvedValues;
    for (int i = 0; i < arguments.size(); i++) {
        auto lsIt = klsArguments.constFind(i);
        if (lsIt != klsArguments.constEnd()) {
            const KLocalizedString &kls = *lsIt;
            int fieldWidth = klsArgumentFieldWidths.value(i);
            QChar fillChar = klsArgumentFillChars.value(i);
            // Override argument's languages and format, but not domain.
            bool isArgumentSub = true;
            QString resdArg = kls.d->toString(kls.d->domain, resolvedLanguages, resolvedFormat, isArgumentSub);
            resolvedValues.append(resdArg);
            if (markupAware && !kls.d->markupAware) {
                resdArg = Kuit::escape(resdArg);
            }
            resdArg = QStringLiteral("%1").arg(resdArg, fieldWidth, fillChar);
            resolvedArguments.append(resdArg);
        } else {
            QString resdArg = arguments[i];
            if (markupAware) {
                resdArg = Kuit::escape(resdArg);
            }
            resolvedArguments.append(resdArg);
            resolvedValues.append(values[i]);
        }
    }

    // Substitute placeholders in ordinary translation.
    QString finalTranslation = substituteSimple(translation, resolvedArguments);
    if (markupAware && !isArgument) {
        // Resolve markup in ordinary translation.
        finalTranslation = formatMarkup(resolvedDomain, language, QString::fromUtf8(context), finalTranslation, resolvedFormat);
    }

    // If there is also a scripted translation.
    if (!scriptedTranslation.isEmpty()) {
        // Evaluate scripted translation.
        bool fallback = false;
        scriptedTranslation = substituteTranscript(scriptedTranslation, language, country, finalTranslation, resolvedArguments, resolvedValues, fallback);

        // If any translation produced and no fallback requested.
        if (!scriptedTranslation.isEmpty() && !fallback) {
            if (markupAware && !isArgument) {
                // Resolve markup in scripted translation.
                scriptedTranslation = formatMarkup(resolvedDomain, language, QString::fromUtf8(context), scriptedTranslation, resolvedFormat);
            }
            finalTranslation = scriptedTranslation;
        }
    }

    // Execute any scripted post calls; they cannot modify the final result,
    // but are used to set states.
    if (s->ktrs != nullptr) {
        const QStringList pcalls = s->ktrs->postCalls(language);
        for (const QString &pcall : pcalls) {
            postTranscript(pcall, language, country, finalTranslation, resolvedArguments, resolvedValues);
        }
    }

    return finalTranslation;
}

QString KLocalizedStringPrivate::substituteSimple(const QString &translation, const QStringList &arguments, QChar plchar, bool isPartial) const
{
#ifdef NDEBUG
    Q_UNUSED(isPartial);
#endif

    QStringList tsegs; // text segments per placeholder occurrence
    QList<int> plords; // ordinal numbers per placeholder occurrence
#ifndef NDEBUG
    QList<int> ords; // indicates which placeholders are present
#endif
    int slen = translation.length();
    int spos = 0;
    int tpos = translation.indexOf(plchar);
    while (tpos >= 0) {
        int ctpos = tpos;

        ++tpos;
        if (tpos == slen) {
            break;
        }

        if (translation[tpos].digitValue() > 0) {
            // NOTE: %0 is not considered a placeholder.
            // Get the placeholder ordinal.
            int plord = 0;
            while (tpos < slen && translation[tpos].digitValue() >= 0) {
                plord = 10 * plord + translation[tpos].digitValue();
                ++tpos;
            }
            --plord; // ordinals are zero based

#ifndef NDEBUG
            // Perhaps enlarge storage for indicators.
            // Note that QList<int> will initialize new elements to 0,
            // as they are supposed to be.
            if (plord >= ords.size()) {
                ords.resize(plord + 1);
            }

            // Indicate that placeholder with computed ordinal is present.
            ords[plord] = 1;
#endif

            // Store text segment prior to placeholder and placeholder number.
            tsegs.append(translation.mid(spos, ctpos - spos));
            plords.append(plord);

            // Position of next text segment.
            spos = tpos;
        }

        tpos = translation.indexOf(plchar, tpos);
    }
    // Store last text segment.
    tsegs.append(translation.mid(spos));

#ifndef NDEBUG
    // Perhaps enlarge storage for plural-number ordinal.
    if (!plural.isEmpty() && numberOrdinal >= ords.size()) {
        ords.resize(numberOrdinal + 1);
    }

    // Message might have plural but without plural placeholder, which is an
    // allowed state. To ease further logic, indicate that plural placeholder
    // is present anyway if message has plural.
    if (!plural.isEmpty()) {
        ords[numberOrdinal] = 1;
    }
#endif

    // Assemble the final string from text segments and arguments.
    QString finalTranslation;
    for (int i = 0; i < plords.size(); i++) {
        finalTranslation.append(tsegs.at(i));
        if (plords.at(i) >= arguments.size()) { // too little arguments
            // put back the placeholder
            finalTranslation.append(QLatin1Char('%') + QString::number(plords.at(i) + 1));
#ifndef NDEBUG
            if (!isPartial) {
                // spoof the message
                finalTranslation.append(QStringLiteral("(I18N_ARGUMENT_MISSING)"));
            }
#endif
        } else { // just fine
            finalTranslation.append(arguments.at(plords.at(i)));
        }
    }
    finalTranslation.append(tsegs.last());

#ifndef NDEBUG
    if (!isPartial && !relaxedSubs) {
        // Check that there are no gaps in numbering sequence of placeholders.
        bool gaps = false;
        for (int i = 0; i < ords.size(); i++) {
            if (!ords.at(i)) {
                gaps = true;
                qCWarning(KI18N).nospace() << "Placeholder %" << QString::number(i + 1) << " skipped in message " << shortenMessage(translation);
            }
        }
        // If no gaps, check for mismatch between the number of
        // unique placeholders and actually supplied arguments.
        if (!gaps && ords.size() != arguments.size()) {
            qCWarning(KI18N) << arguments.size() << "instead of" << ords.size() << "arguments to message" << shortenMessage(translation)
                             << "supplied before conversion";
        }

        // Some spoofs.
        if (gaps) {
            finalTranslation.append(QStringLiteral("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
        }
        if (ords.size() < arguments.size()) {
            finalTranslation.append(QStringLiteral("(I18N_EXCESS_ARGUMENTS_SUPPLIED)"));
        }
    }
    if (!isPartial) {
        if (!plural.isEmpty() && !numberSet) {
            finalTranslation.append(QStringLiteral("(I18N_PLURAL_ARGUMENT_MISSING)"));
        }
    }
#endif

    return finalTranslation;
}

QString KLocalizedStringPrivate::formatMarkup(const QByteArray &domain,
                                              const QString &language,
                                              const QString &context,
                                              const QString &text,
                                              Kuit::VisualFormat format) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QHash<QString, KuitFormatter *>::iterator formatter = s->formatters.find(language);
    if (formatter == s->formatters.end()) {
        formatter = s->formatters.insert(language, new KuitFormatter(language));
    }
    return (*formatter)->format(domain, context, text, format);
}

QString KLocalizedStringPrivate::substituteTranscript(const QString &scriptedTranslation,
                                                      const QString &language,
                                                      const QString &country,
                                                      const QString &ordinaryTranslation,
                                                      const QStringList &arguments,
                                                      const QList<QVariant> &values,
                                                      bool &fallback) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    if (s->ktrs == nullptr) {
        // Scripting engine not available.
        return QString();
    }

    // Iterate by interpolations.
    QString finalTranslation;
    fallback = false;
    int ppos = 0;
    int tpos = scriptedTranslation.indexOf(s->startInterp);
    while (tpos >= 0) {
        // Resolve substitutions in preceding text.
        QString ptext = substituteSimple(scriptedTranslation.mid(ppos, tpos - ppos), arguments, s->scriptPlchar, true);
        finalTranslation.append(ptext);

        // Resolve interpolation.
        QString result;
        bool fallbackLocal;
        tpos = resolveInterpolation(scriptedTranslation, tpos, language, country, ordinaryTranslation, arguments, values, result, fallbackLocal);

        // If there was a problem in parsing the interpolation, cannot proceed
        // (debug info already reported while parsing).
        if (tpos < 0) {
            return QString();
        }
        // If fallback has been explicitly requested, indicate global fallback
        // but proceed with evaluations (other interpolations may set states).
        if (fallbackLocal) {
            fallback = true;
        }

        // Add evaluated interpolation to the text.
        finalTranslation.append(result);

        // On to next interpolation.
        ppos = tpos;
        tpos = scriptedTranslation.indexOf(s->startInterp, tpos);
    }
    // Last text segment.
    finalTranslation.append(substituteSimple(scriptedTranslation.mid(ppos), arguments, s->scriptPlchar, true));

    // Return empty string if fallback was requested.
    return fallback ? QString() : finalTranslation;
}

int KLocalizedStringPrivate::resolveInterpolation(const QString &scriptedTranslation,
                                                  int pos,
                                                  const QString &language,
                                                  const QString &country,
                                                  const QString &ordinaryTranslation,
                                                  const QStringList &arguments,
                                                  const QList<QVariant> &values,
                                                  QString &result,
                                                  bool &fallback) const
{
    // pos is the position of opening character sequence.
    // Returns the position of first character after closing sequence,
    // or -1 in case of parsing error.
    // result is set to result of Transcript evaluation.
    // fallback is set to true if Transcript evaluation requested so.

    KLocalizedStringPrivateStatics *s = staticsKLSP();

    result.clear();
    fallback = false;

    // Split interpolation into arguments.
    QList<QVariant> iargs;
    const int slen = scriptedTranslation.length();
    const int islen = s->startInterp.length();
    const int ielen = s->endInterp.length();
    int tpos = pos + s->startInterp.length();
    while (1) {
        // Skip whitespace.
        while (tpos < slen && scriptedTranslation[tpos].isSpace()) {
            ++tpos;
        }
        if (tpos == slen) {
            qCWarning(KI18N) << "Unclosed interpolation" << scriptedTranslation.mid(pos, tpos - pos) << "in message" << shortenMessage(scriptedTranslation);
            return -1;
        }
        if (QStringView(scriptedTranslation).mid(tpos, ielen) == s->endInterp) {
            break; // no more arguments
        }

        // Parse argument: may be concatenated from free and quoted text,
        // and sub-interpolations.
        // Free and quoted segments may contain placeholders, substitute them;
        // recurse into sub-interpolations.
        // Free segments may be value references, parse and record for
        // consideration at the end.
        // Mind backslash escapes throughout.
        QStringList segs;
        QVariant vref;
        while (!scriptedTranslation[tpos].isSpace() && scriptedTranslation.mid(tpos, ielen) != s->endInterp) {
            if (scriptedTranslation[tpos] == QLatin1Char('\'')) { // quoted segment
                QString seg;
                ++tpos; // skip opening quote
                // Find closing quote.
                while (tpos < slen && scriptedTranslation[tpos] != QLatin1Char('\'')) {
                    if (scriptedTranslation[tpos] == QLatin1Char('\\')) {
                        ++tpos; // escape next character
                    }
                    seg.append(scriptedTranslation[tpos]);
                    ++tpos;
                }
                if (tpos == slen) {
                    qCWarning(KI18N) << "Unclosed quote in interpolation" << scriptedTranslation.mid(pos, tpos - pos) << "in message"
                                     << shortenMessage(scriptedTranslation);
                    return -1;
                }

                // Append to list of segments, resolving placeholders.
                segs.append(substituteSimple(seg, arguments, s->scriptPlchar, true));

                ++tpos; // skip closing quote
            } else if (scriptedTranslation.mid(tpos, islen) == s->startInterp) { // sub-interpolation
                QString resultLocal;
                bool fallbackLocal;
                tpos = resolveInterpolation(scriptedTranslation, tpos, language, country, ordinaryTranslation, arguments, values, resultLocal, fallbackLocal);
                if (tpos < 0) { // unrecoverable problem in sub-interpolation
                    // Error reported in the subcall.
                    return tpos;
                }
                if (fallbackLocal) { // sub-interpolation requested fallback
                    fallback = true;
                }
                segs.append(resultLocal);
            } else { // free segment
                QString seg;
                // Find whitespace, quote, opening or closing sequence.
                while (tpos < slen && !scriptedTranslation[tpos].isSpace() //
                       && scriptedTranslation[tpos] != QLatin1Char('\'') //
                       && scriptedTranslation.mid(tpos, islen) != s->startInterp //
                       && scriptedTranslation.mid(tpos, ielen) != s->endInterp) {
                    if (scriptedTranslation[tpos] == QLatin1Char('\\')) {
                        ++tpos; // escape next character
                    }
                    seg.append(scriptedTranslation[tpos]);
                    ++tpos;
                }
                if (tpos == slen) {
                    qCWarning(KI18N) << "Non-terminated interpolation" << scriptedTranslation.mid(pos, tpos - pos) << "in message"
                                     << shortenMessage(scriptedTranslation);
                    return -1;
                }

                // The free segment may look like a value reference;
                // in that case, record which value it would reference,
                // and add verbatim to the segment list.
                // Otherwise, do a normal substitution on the segment.
                vref = segmentToValue(seg);
                if (vref.isValid()) {
                    segs.append(seg);
                } else {
                    segs.append(substituteSimple(seg, arguments, s->scriptPlchar, true));
                }
            }
        }

        // Append this argument to rest of the arguments.
        // If the there was a single text segment and it was a proper value
        // reference, add it instead of the joined segments.
        // Otherwise, add the joined segments.
        if (segs.size() == 1 && vref.isValid()) {
            iargs.append(vref);
        } else {
            iargs.append(segs.join(QString()));
        }
    }
    tpos += ielen; // skip to first character after closing sequence

    // NOTE: Why not substitute placeholders (via substituteSimple) in one
    // global pass, then handle interpolations in second pass? Because then
    // there is the danger of substituted text or sub-interpolations producing
    // quotes and escapes themselves, which would mess up the parsing.

    // Evaluate interpolation.
    QString msgctxt = QString::fromUtf8(context);
    QString msgid = QString::fromUtf8(text);
    QString scriptError;
    bool fallbackLocal;
    result = s->ktrs->eval(iargs,
                           language,
                           country,
                           msgctxt,
                           dynamicContext,
                           msgid,
                           arguments,
                           values,
                           ordinaryTranslation,
                           s->scriptModulesToLoad,
                           scriptError,
                           fallbackLocal);
    // s->scriptModulesToLoad will be cleared during the call.

    if (fallbackLocal) { // evaluation requested fallback
        fallback = true;
    }
    if (!scriptError.isEmpty()) { // problem with evaluation
        fallback = true; // also signal fallback
        if (!scriptError.isEmpty()) {
            qCWarning(KI18N) << "Interpolation" << scriptedTranslation.mid(pos, tpos - pos) << "in" << shortenMessage(scriptedTranslation)
                             << "failed:" << scriptError;
        }
    }

    return tpos;
}

QVariant KLocalizedStringPrivate::segmentToValue(const QString &segment) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    // Return invalid variant if segment is either not a proper
    // value reference, or the reference is out of bounds.

    // Value reference must start with a special character.
    if (!segment.startsWith(s->scriptVachar)) {
        return QVariant();
    }

    // Reference number must start with 1-9.
    // (If numstr is empty, toInt() will return 0.)
    QString numstr = segment.mid(1);
    int numstrAsInt = QStringView(numstr).left(1).toInt();
    if (numstrAsInt < 1) {
        return QVariant();
    }

    // Number must be valid and in bounds.
    bool ok;
    int index = numstr.toInt(&ok) - 1;
    if (!ok || index >= values.size()) {
        return QVariant();
    }

    // Passed all hoops.
    return values.at(index);
}

QString KLocalizedStringPrivate::postTranscript(const QString &pcall,
                                                const QString &language,
                                                const QString &country,
                                                const QString &finalTranslation,
                                                const QStringList &arguments,
                                                const QList<QVariant> &values) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    if (s->ktrs == nullptr) {
        // Scripting engine not available.
        // (Though this cannot happen, we wouldn't be here then.)
        return QString();
    }

    // Resolve the post call.
    QList<QVariant> iargs;
    iargs.append(pcall);
    QString msgctxt = QString::fromUtf8(context);
    QString msgid = QString::fromUtf8(text);
    QString scriptError;
    bool fallback;
    s->ktrs->eval(iargs, language, country, msgctxt, dynamicContext, msgid, arguments, values, finalTranslation, s->scriptModulesToLoad, scriptError, fallback);
    // s->scriptModulesToLoad will be cleared during the call.

    // If the evaluation went wrong.
    if (!scriptError.isEmpty()) {
        qCWarning(KI18N) << "Post call" << pcall << "for message" << shortenMessage(msgid) << "failed:" << scriptError;
        return QString();
    }

    return finalTranslation;
}

KLocalizedString KLocalizedString::withLanguages(const QStringList &languages) const
{
    KLocalizedString kls(*this);
    kls.d->languages = languages;
    return kls;
}

KLocalizedString KLocalizedString::withDomain(const char *domain) const
{
    KLocalizedString kls(*this);
    kls.d->domain = domain;
    return kls;
}

KLocalizedString KLocalizedString::withFormat(Kuit::VisualFormat format) const
{
    KLocalizedString kls(*this);
    kls.d->format = format;
    return kls;
}

KLocalizedString KLocalizedString::subs(int a, int fieldWidth, int base, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->checkNumber(std::abs(a));
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, base, fillChar));
    kls.d->values.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(uint a, int fieldWidth, int base, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->checkNumber(a);
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, base, fillChar));
    kls.d->values.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(long a, int fieldWidth, int base, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->checkNumber(std::abs(a));
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, base, fillChar));
    kls.d->values.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(ulong a, int fieldWidth, int base, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->checkNumber(a);
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, base, fillChar));
    kls.d->values.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(qlonglong a, int fieldWidth, int base, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->checkNumber(qAbs(a));
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, base, fillChar));
    kls.d->values.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(qulonglong a, int fieldWidth, int base, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->checkNumber(a);
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, base, fillChar));
    kls.d->values.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(double a, int fieldWidth, char format, int precision, QChar fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->arguments.append(QStringLiteral("%L1").arg(a, fieldWidth, format, precision, fillChar));
    kls.d->values.append(static_cast<realn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(QChar a, int fieldWidth, QChar fillChar) const
{
    KLocalizedString kls(*this);
    QString baseArg = QString(a);
    QString fmtdArg = QStringLiteral("%1").arg(a, fieldWidth, fillChar);
    kls.d->arguments.append(fmtdArg);
    kls.d->values.append(baseArg);
    return kls;
}

KLocalizedString KLocalizedString::subs(const QString &a, int fieldWidth, QChar fillChar) const
{
    KLocalizedString kls(*this);
    QString baseArg = a;
    QString fmtdArg = QStringLiteral("%1").arg(a, fieldWidth, fillChar);
    kls.d->arguments.append(fmtdArg);
    kls.d->values.append(baseArg);
    return kls;
}

KLocalizedString KLocalizedString::subs(const KLocalizedString &a, int fieldWidth, QChar fillChar) const
{
    KLocalizedString kls(*this);
    // KLocalizedString arguments must be resolved inside toString
    // when the domain, language, visual format, etc. become known.
    int i = kls.d->arguments.size();
    kls.d->klsArguments[i] = a;
    kls.d->klsArgumentFieldWidths[i] = fieldWidth;
    kls.d->klsArgumentFillChars[i] = fillChar;
    kls.d->arguments.append(QString());
    kls.d->values.append(0);
    return kls;
}

KLocalizedString KLocalizedString::inContext(const QString &key, const QString &value) const
{
    KLocalizedString kls(*this);
    kls.d->dynamicContext[key] = value;
    return kls;
}

KLocalizedString KLocalizedString::relaxSubs() const
{
    KLocalizedString kls(*this);
    kls.d->relaxedSubs = true;
    return kls;
}

KLocalizedString KLocalizedString::ignoreMarkup() const
{
    KLocalizedString kls(*this);
    kls.d->markupAware = false;
    return kls;
}

QByteArray KLocalizedString::untranslatedText() const
{
    return d->text;
}

void KLocalizedString::setApplicationDomain(const QByteArray &domain)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->applicationDomain = domain;
}

QByteArray KLocalizedString::applicationDomain()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    return s->applicationDomain;
}

QStringList KLocalizedString::languages()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    return s->languages;
}

void KLocalizedString::setLanguages(const QStringList &languages)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->languages = languages;
}

void KLocalizedString::clearLanguages()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->languages = s->localeLanguages;
}

bool KLocalizedString::isApplicationTranslatedInto(const QString &language)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    return language == s->codeLanguage || !KCatalog::catalogLocaleDir(s->applicationDomain, language).isEmpty();
}

QSet<QString> KLocalizedString::availableApplicationTranslations()
{
    return availableDomainTranslations(staticsKLSP()->applicationDomain);
}

QSet<QString> KLocalizedString::availableDomainTranslations(const QByteArray &domain)
{
    QSet<QString> availableLanguages;

    if (!domain.isEmpty()) {
        availableLanguages = KCatalog::availableCatalogLanguages(domain);
        availableLanguages.insert(staticsKLSP()->codeLanguage);
    }

    return availableLanguages;
}

const KCatalog &KLocalizedStringPrivate::getCatalog(const QByteArray &domain, const QString &language)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    QHash<QByteArray, KCatalogPtrHash>::iterator languageCatalogs = s->catalogs.find(domain);
    if (languageCatalogs == s->catalogs.end()) {
        languageCatalogs = s->catalogs.insert(domain, KCatalogPtrHash());
    }
    KCatalogPtrHash::iterator catalog = languageCatalogs->find(language);
    if (catalog == languageCatalogs->end()) {
        catalog = languageCatalogs->insert(language, new KCatalog(domain, language));
        locateScriptingModule(domain, language);
    }
    return **catalog;
}

void KLocalizedStringPrivate::locateScriptingModule(const QByteArray &domain, const QString &language)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    QString modapath =
        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/%1/%2/%3/%3.js").arg(language, s->scriptDir, QLatin1String{domain}));

    // If the module exists and hasn't been already included.
    if (!modapath.isEmpty() && !s->scriptModules[language].contains(domain)) {
        // Indicate that the module has been considered.
        s->scriptModules[language].append(domain);

        // Store the absolute path and language of the module,
        // to load on next script evaluation.
        QStringList module;
        module.append(modapath);
        module.append(language);
        s->scriptModulesToLoad.append(module);
    }
}

extern "C" {
typedef KTranscript *(*InitFunc)();
}

void KLocalizedStringPrivate::loadTranscript()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->loadTranscriptCalled = true;
    s->ktrs = nullptr; // null indicates that Transcript is not available

    // QPluginLoader is just used to find the plugin
    QPluginLoader loader(QStringLiteral("kf6/ktranscript"));
    if (loader.fileName().isEmpty()) {
        qCWarning(KI18N) << "Cannot find Transcript plugin.";
        return;
    }

    QLibrary lib(loader.fileName());
    if (!lib.load()) {
        qCWarning(KI18N) << "Cannot load Transcript plugin:" << lib.errorString();
        return;
    }

    InitFunc initf = (InitFunc)lib.resolve("load_transcript");
    if (!initf) {
        lib.unload();
        qCWarning(KI18N) << "Cannot find function load_transcript in Transcript plugin.";
        return;
    }

    s->ktrs = initf();
}

QString KLocalizedString::localizedFilePath(const QString &filePath)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    // Check if l10n subdirectory is present, stop if not.
    QFileInfo fileInfo(filePath);
    QString locDirPath = fileInfo.path() + QLatin1Char('/') + QLatin1String("l10n");
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    // Go through possible localized paths by priority of languages,
    // return first that exists.
    QString fileName = fileInfo.fileName();
    for (const QString &lang : std::as_const(s->languages)) {
        QString locFilePath = locDirPath + QLatin1Char('/') + lang + QLatin1Char('/') + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocalizedString::removeAcceleratorMarker(const QString &label)
{
    return ::removeAcceleratorMarker(label);
}

void KLocalizedString::addDomainLocaleDir(const QByteArray &domain, const QString &path)
{
    KCatalog::addDomainLocaleDir(domain, path);
}

KLocalizedString ki18n(const char *text)
{
    return KLocalizedString(nullptr, nullptr, text, nullptr, false);
}

KLocalizedString ki18nc(const char *context, const char *text)
{
    return KLocalizedString(nullptr, context, text, nullptr, false);
}

KLocalizedString ki18np(const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, nullptr, singular, plural, false);
}

KLocalizedString ki18ncp(const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, context, singular, plural, false);
}

KLocalizedString ki18nd(const char *domain, const char *text)
{
    return KLocalizedString(domain, nullptr, text, nullptr, false);
}

KLocalizedString ki18ndc(const char *domain, const char *context, const char *text)
{
    return KLocalizedString(domain, context, text, nullptr, false);
}

KLocalizedString ki18ndp(const char *domain, const char *singular, const char *plural)
{
    return KLocalizedString(domain, nullptr, singular, plural, false);
}

KLocalizedString ki18ndcp(const char *domain, const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(domain, context, singular, plural, false);
}

KLocalizedString kxi18n(const char *text)
{
    return KLocalizedString(nullptr, nullptr, text, nullptr, true);
}

KLocalizedString kxi18nc(const char *context, const char *text)
{
    return KLocalizedString(nullptr, context, text, nullptr, true);
}

KLocalizedString kxi18np(const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, nullptr, singular, plural, true);
}

KLocalizedString kxi18ncp(const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, context, singular, plural, true);
}

KLocalizedString kxi18nd(const char *domain, const char *text)
{
    return KLocalizedString(domain, nullptr, text, nullptr, true);
}

KLocalizedString kxi18ndc(const char *domain, const char *context, const char *text)
{
    return KLocalizedString(domain, context, text, nullptr, true);
}

KLocalizedString kxi18ndp(const char *domain, const char *singular, const char *plural)
{
    return KLocalizedString(domain, nullptr, singular, plural, true);
}

KLocalizedString kxi18ndcp(const char *domain, const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(domain, context, singular, plural, true);
}
