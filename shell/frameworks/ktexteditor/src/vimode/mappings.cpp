/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katepartdebug.h"
#include "kateviinputmode.h"
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>
#include <vimode/inputmodemanager.h>
#include <vimode/keyparser.h>
#include <vimode/mappings.h>

#include <KConfigGroup>

using namespace KateVi;

void Mappings::readConfig(const KConfigGroup &config)
{
    readMappings(config, QStringLiteral("Normal"), NormalModeMapping);
    readMappings(config, QStringLiteral("Visual"), VisualModeMapping);
    readMappings(config, QStringLiteral("Insert"), InsertModeMapping);
    readMappings(config, QStringLiteral("Command"), CommandModeMapping);
}

void Mappings::writeConfig(KConfigGroup &config) const
{
    writeMappings(config, QStringLiteral("Normal"), NormalModeMapping);
    writeMappings(config, QStringLiteral("Visual"), VisualModeMapping);
    writeMappings(config, QStringLiteral("Insert"), InsertModeMapping);
    writeMappings(config, QStringLiteral("Command"), CommandModeMapping);
}

void Mappings::writeMappings(KConfigGroup &config, const QString &mappingModeName, MappingMode mappingMode) const
{
    config.writeEntry(mappingModeName + QLatin1String(" Mode Mapping Keys"), getAll(mappingMode, true));
    QStringList l;
    QList<bool> recursives;
    const auto all = getAll(mappingMode);
    l.reserve(all.size());
    recursives.reserve(all.size());
    for (const QString &s : all) {
        l << KeyParser::self()->decodeKeySequence(get(mappingMode, s));
        recursives << isRecursive(mappingMode, s);
    }
    config.writeEntry(mappingModeName + QLatin1String(" Mode Mappings"), l);
    config.writeEntry(mappingModeName + QLatin1String(" Mode Mappings Recursion"), recursives);

    QChar leader = (m_leader.isNull()) ? QChar::fromLatin1('\\') : m_leader;
    config.writeEntry(QStringLiteral("Map Leader"), QString(leader));
}

void Mappings::readMappings(const KConfigGroup &config, const QString &mappingModeName, MappingMode mappingMode)
{
    const QStringList keys = config.readEntry(mappingModeName + QLatin1String(" Mode Mapping Keys"), QStringList());
    const QStringList mappings = config.readEntry(mappingModeName + QLatin1String(" Mode Mappings"), QStringList());
    const QList<bool> isRecursive = config.readEntry(mappingModeName + QLatin1String(" Mode Mappings Recursion"), QList<bool>());

    const QString &mapLeader = config.readEntry(QStringLiteral("Map Leader"), QString());
    m_leader = (mapLeader.isEmpty()) ? QChar::fromLatin1('\\') : mapLeader[0];

    // sanity check
    if (keys.length() == mappings.length()) {
        for (int i = 0; i < keys.length(); i++) {
            // "Recursion" is a newly-introduced part of the config that some users won't have,
            // so rather than abort (and lose our mappings) if there are not enough entries, simply
            // treat any missing ones as Recursive (for backwards compatibility).
            MappingRecursion recursion = Recursive;
            if (isRecursive.size() > i && !isRecursive.at(i)) {
                recursion = NonRecursive;
            }
            add(mappingMode, keys.at(i), mappings.at(i), recursion);
        }
    } else {
        qCDebug(LOG_KTE) << "Error when reading mappings from " << mappingModeName << " config: number of keys != number of values";
    }
}

void Mappings::add(MappingMode mode, const QString &from, const QString &to, MappingRecursion recursion)
{
    const QString &encodedMapping = KeyParser::self()->encodeKeySequence(from);

    if (from.isEmpty()) {
        return;
    }

    const QString encodedTo = KeyParser::self()->encodeKeySequence(to);
    Mapping mapping = {encodedTo, (recursion == Recursive), false};

    // Add this mapping as is.
    m_mappings[mode][encodedMapping] = mapping;

    // In normal mode replace the <leader> with its value.
    if (mode == NormalModeMapping) {
        QString other = from;
        other.replace(QLatin1String("<leader>"), m_leader);
        other = KeyParser::self()->encodeKeySequence(other);
        if (other != encodedMapping) {
            mapping.temporary = true;
            m_mappings[mode][other] = mapping;
        }
    }
}

void Mappings::remove(MappingMode mode, const QString &from)
{
    const QString &encodedMapping = KeyParser::self()->encodeKeySequence(from);
    m_mappings[mode].remove(encodedMapping);
}

void Mappings::clear(MappingMode mode)
{
    m_mappings[mode].clear();
}

QString Mappings::get(MappingMode mode, const QString &from, bool decode, bool includeTemporary) const
{
    if (!m_mappings[mode].contains(from)) {
        return QString();
    }

    const Mapping &mapping = m_mappings[mode][from];
    if (mapping.temporary && !includeTemporary) {
        return QString();
    }

    const QString &ret = mapping.encoded;
    if (decode) {
        return KeyParser::self()->decodeKeySequence(ret);
    }

    return ret;
}

QStringList Mappings::getAll(MappingMode mode, bool decode, bool includeTemporary) const
{
    QStringList mappings;
    const QHash<QString, Mapping> mappingsForMode = m_mappings[mode];

    for (auto i = mappingsForMode.begin(); i != mappingsForMode.end(); i++) {
        if (!includeTemporary && i.value().temporary) {
            continue;
        }

        if (decode) {
            mappings << KeyParser::self()->decodeKeySequence(i.key());
        } else {
            mappings << i.key();
        }
    }
    return mappings;
}

bool Mappings::isRecursive(MappingMode mode, const QString &from) const
{
    if (!m_mappings[mode].contains(from)) {
        return false;
    }

    return m_mappings[mode][from].recursive;
}

void Mappings::setLeader(const QChar &leader)
{
    m_leader = leader;
}

Mappings::MappingMode Mappings::mappingModeForCurrentViMode(KateViInputMode *viInputMode)
{
    if (viInputMode->viModeEmulatedCommandBar()->isActive()) {
        return CommandModeMapping;
    }
    const ViMode mode = viInputMode->viInputModeManager()->getCurrentViMode();
    switch (mode) {
    case ViMode::NormalMode:
        return NormalModeMapping;
    case ViMode::VisualMode:
    case ViMode::VisualLineMode:
    case ViMode::VisualBlockMode:
        return VisualModeMapping;
    case ViMode::InsertMode:
    case ViMode::ReplaceMode:
        return InsertModeMapping;
    default:
        Q_ASSERT(false && "unrecognised ViMode!");
        return NormalModeMapping; // Return arbitrary mode to satisfy compiler.
    }
}
