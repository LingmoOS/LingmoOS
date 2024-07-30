/*
 *   SPDX-FileCopyrightText: 2007 Ryan P. Bitanga <ephebiphobic@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "spellcheck.h"

#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QMimeData>
#include <QSet>

#include <KConfigGroup>
#include <KLocalizedString>

SpellCheckRunner::SpellCheckRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
}

SpellCheckRunner::~SpellCheckRunner() = default;

// Load a default dictionary and some locale names
void SpellCheckRunner::loadData()
{
    // Load the default speller, with the default language
    auto defaultSpellerIt = m_spellers.find(QString());
    if (defaultSpellerIt == m_spellers.end()) {
        defaultSpellerIt = m_spellers.insert(QString(), QSharedPointer<Sonnet::Speller>(new Sonnet::Speller(QString())));
    }
    auto &defaultSpeller = defaultSpellerIt.value();

    // store all language names, makes it possible to type "spell german TERM" if english locale is set
    // Need to construct a map between natual language names and names the spell-check recognises.
    m_availableLangCodes = defaultSpeller->availableLanguages();
    // We need to filter the available languages so that we associate the natural language
    // name (eg. 'german') with one sub-code.
    QSet<QString> families;
    // First get the families
    for (const QString &code : std::as_const(m_availableLangCodes)) {
        families += code.left(2);
    }
    // Now for each family figure out which is the main code.
    for (const QString &fcode : std::as_const(families)) {
        const QStringList family = m_availableLangCodes.filter(fcode);
        QString code;
        // If we only have one code, use it.
        // If a string is the default language, use it
        if (family.contains(defaultSpeller->language())) {
            code = defaultSpeller->language();
        } else if (fcode == QLatin1String("en")) {
            // If the family is english, default to en_US.
            const auto enUS = QStringLiteral("en_US");
            if (family.contains(enUS)) {
                code = enUS;
            }
        } else if (family.contains(fcode + QLatin1Char('_') + fcode.toUpper())) {
            // If we have a speller of the form xx_XX, try that.
            // This gets us most European languages with more than one spelling.
            code = fcode + QLatin1Char('_') + fcode.toUpper();
        } else {
            // Otherwise, pick the first value as it is highest priority.
            code = family.first();
        }
        // Finally, add code to the map.
        // FIXME: We need someway to map languageCodeToName
        const QString name; // = locale->languageCodeToName(fcode);
        if (!name.isEmpty()) {
            m_languages[name.toLower()] = code;
        }
    }
}

void SpellCheckRunner::reloadConfiguration()
{
    const KConfigGroup cfg = config();
    m_triggerWord = cfg.readEntry("trigger", i18n("spell"));
    // Processing will be triggered by "keyword "
    m_requireTriggerWord = cfg.readEntry("requireTriggerWord", true) && !m_triggerWord.isEmpty();
    m_triggerWord += QLatin1Char(' ');

    QStringList exampleQueries{i18nc("Spelling checking runner syntax, first word is trigger word, e.g.  \"spell\".", "%1:q:", m_triggerWord)};
    if (!m_requireTriggerWord) {
        exampleQueries.append(QStringLiteral(":q:"));
    }
    RunnerSyntax s(exampleQueries, i18n("Checks the spelling of :q:."));

    if (m_requireTriggerWord) {
        setTriggerWords({m_triggerWord});
        setMinLetterCount(minLetterCount() + 2); // We want at least two letters after the trigger word
    } else {
        setMinLetterCount(2);
        setMatchRegex(QRegularExpression());
    }

    setSyntaxes({RunnerSyntax(s)});
    // Clear the data arrays to save memory
    m_spellers.clear();
    m_availableLangCodes.clear();
}

/* Take the input query, split into a list, and see if it contains a language to spell in.
 * Return the empty string if we can't match a language. */
QString SpellCheckRunner::findLang(const QStringList &terms)
{
    if (terms.isEmpty()) {
        return QString();
    }

    // If first term is a language code (like en_GB, en_gb or en), set it as the spell-check language
    const auto langCodeIt = std::find_if(m_availableLangCodes.cbegin(), m_availableLangCodes.cend(), [&terms](const QString &languageCode) {
        return languageCode.startsWith(terms[0], Qt::CaseInsensitive);
    });
    if (langCodeIt != m_availableLangCodes.cend()) {
        return *langCodeIt;
    }

    // If we have two terms and the first is a language name (eg 'french'),
    // set it as the available language
    if (terms.count() >= 2) {
        QString code;
        {
            // Is this a descriptive language name?
            QMap<QString, QString>::const_iterator it = m_languages.constFind(terms[0].toLower());
            if (it != m_languages.constEnd()) {
                code = *it;
            }
            // Maybe it is a subset of a language code?
            else {
                QStringList codes = QStringList(m_languages.values()).filter(terms[0]);
                if (!codes.isEmpty()) {
                    code = codes.first();
                }
            }
        }

        if (!code.isEmpty()) {
            // We found a valid language! Check still available
            // Does the spell-checker like it?
            if (m_availableLangCodes.contains(code)) {
                return code;
            }
        }
        // FIXME: Support things like 'british english' or 'canadian french'
    }
    return QString();
}

void SpellCheckRunner::match(RunnerContext &context)
{
    const QString term = context.query();
    QString query = term;

    if (m_requireTriggerWord) {
        int len = m_triggerWord.length();
        if (query.left(len) != m_triggerWord) {
            return;
        }
        query = query.mid(len).trimmed();
    }
    if (m_spellers.isEmpty()) {
        loadData();
    }

    // Pointer to speller object with our chosen language
    QSharedPointer<Sonnet::Speller> speller = m_spellers[QString()];

    QString lang;
    if (speller->isValid()) {
        QStringList terms = query.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        lang = findLang(terms);
        // If we found a language, create a new speller object using it.
        if (!lang.isEmpty()) {
            // First term is the language
            terms.removeFirst();
            // New speller object if we don't already have one
            if (!m_spellers.contains(lang)) {
                // Check nothing happened while we were acquiring the lock
                if (!m_spellers.contains(lang)) {
                    m_spellers[lang] = QSharedPointer<Sonnet::Speller>(new Sonnet::Speller(lang));
                }
            }
            speller = m_spellers[lang];
            // Rejoin the strings
            query = terms.join(QLatin1Char(' '));
        }
    }

    if (query.size() < 2) {
        return;
    }

    if (speller->isValid()) {
        const auto fillMatch = [this, &context, &query, &speller](const QString &langCode = QString()) {
            if (!langCode.isEmpty()) {
                speller->setLanguage(langCode);
            }

            QStringList suggestions;
            const bool correct = speller->checkAndSuggest(query, suggestions);

            if (correct) {
                QueryMatch match(this);
                match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
                match.setIconName(QStringLiteral("checkbox"));
                match.setText(query);
                match.setSubtext(i18nc("Term is spelled correctly", "Correct"));
                match.setData(query);
                context.addMatch(match);
            } else if (!suggestions.isEmpty()) {
                for (const auto &suggestion : std::as_const(suggestions)) {
                    QueryMatch match(this);
                    match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
                    match.setIconName(QStringLiteral("edit-rename"));
                    match.setText(suggestion);
                    match.setSubtext(i18n("Suggested term"));
                    match.setData(suggestion);
                    context.addMatch(match);
                }
            } else {
                return false;
            }

            return true;
        };

        if (!fillMatch() && lang.isEmpty() && m_availableLangCodes.count() >= 2) {
            // Perhaps the term is not in the default dictionary, try other dictionaries.
            const QString defaultLangCode = speller->language();
            for (const QString &langCode : std::as_const(m_availableLangCodes)) {
                if (langCode == defaultLangCode) {
                    continue;
                }

                if (fillMatch(langCode)) {
                    // The dictionary returns valid results
                    break;
                }
            }
            // No need to reset the default language as the speller will be reset in destroydata()
        }
    } else {
        QueryMatch match(this);
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        match.setIconName(QStringLiteral("data-error"));
        match.setText(xi18nc("@info", "No dictionary found. Please install <resource>hunspell</resource> package using your package manager"));
        context.addMatch(match);
    }
}

void SpellCheckRunner::run(const RunnerContext & /*context*/, const QueryMatch &match)
{
    QGuiApplication::clipboard()->setText(match.data().toString());
}

QMimeData *SpellCheckRunner::mimeDataForMatch(const QueryMatch &match)
{
    QMimeData *result = new QMimeData();
    const QString text = match.data().toString();
    result->setText(text);
    return result;
}

K_PLUGIN_CLASS_WITH_JSON(SpellCheckRunner, "plasma-runner-spellchecker.json")

#include "spellcheck.moc"
