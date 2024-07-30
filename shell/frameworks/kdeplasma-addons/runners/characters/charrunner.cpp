/* SPDX-FileCopyrightText: 2010 Anton Kreuzkamp <akreuzkamp@web.de>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "charrunner.h"
#include "config_keys.h"

// KF
#include <KConfigGroup>
#include <KLocalizedString>
#include <KRunner/QueryMatch>
// Qt
#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>

CharacterRunner::CharacterRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
}

CharacterRunner::~CharacterRunner()
{
}

void CharacterRunner::reloadConfiguration()
{
    const KConfigGroup grp = config();
    m_triggerWord = grp.readEntry(CONFIG_TRIGGERWORD, DEFAULT_TRIGGERWORD.toString());
    m_aliases = grp.readEntry(CONFIG_ALIASES, QStringList());
    m_codes = grp.readEntry(CONFIG_CODES, QStringList());
    if (m_codes.size() != m_aliases.size()) {
        m_aliases.clear();
        m_codes.clear();
        qWarning() << "Config entries for alias list and code list have different sizes, ignoring all.";
    }

    addSyntax(m_triggerWord + QStringLiteral(":q:"), i18n("Creates Characters from :q: if it is a hexadecimal code or defined alias."));
    setTriggerWords({m_triggerWord});
    setMinLetterCount(minLetterCount() + 1);
}

void CharacterRunner::match(RunnerContext &context)
{
    QString term = context.query().remove(QLatin1Char(' '));
    term = term.remove(0, m_triggerWord.length()); // remove the triggerword

    // replace aliases by their hex.-code
    if (m_aliases.contains(term)) {
        term = m_codes[m_aliases.indexOf(term)];
    }

    bool ok;
    int hex = term.toInt(&ok, 16); // convert query into int
    if (!ok) {
        return;
    }

    // QChar asserts this.
    if (hex > 0xffff) {
        return;
    }

    // make special character out of the hex.-code
    const QString specChar = QChar(hex);
    QueryMatch match(this);
    match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
    match.setIconName(QStringLiteral("accessories-character-map"));
    match.setText(specChar);
    match.setData(specChar);
    context.addMatch(match);
}

void CharacterRunner::run(const RunnerContext & /*context*/, const QueryMatch &match)
{
    QGuiApplication::clipboard()->setText(match.data().toString());
}

K_PLUGIN_CLASS_WITH_JSON(CharacterRunner, "plasma-runner-character.json")

#include "charrunner.moc"
