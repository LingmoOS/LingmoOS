// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tracks.h"
#include "languages.h"

#include "base/i18n.h"
#include "base/strings.h"

using namespace Lingmo;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace tracks
{

std::string label(const MpvNode& track)
{
    const std::string title(base::strings::trimmed(track["title"].toString()));
    const std::string language = languages::displayName(track["lang"].toString());
    if (!title.empty() && !language.empty()
            && base::strings::compare(title, language, base::strings::CaseSensitivity::Insensitive) != 0)
        // TRANSLATORS: a track's own title, then its language.
        return format(tr("{title} ({language})"), {{"title", title}, {"language", language}});
    if (!title.empty())
        return title;
    if (!language.empty())
        return language;
    return format(tr("Track {number}"), {{"number", std::to_string(track["id"].toInt())}});
}

Split split(const MpvNode& trackList)
{
    Split result;
    for (const MpvNode& track : trackList.list()) {
        const std::string type = track["type"].toString();
        if (type == "video") {
            if (!track["albumart"].toBool())
                result.hasVideo = true;
            continue;
        }

        Track choice;
        choice.id = int(track["id"].toInt());
        choice.label = label(track);
        choice.selected = track["selected"].toBool();
        choice.external = track["external"].toBool();
        if (type == "audio")
            result.audio.push_back(std::move(choice));
        else if (type == "sub")
            result.subtitle.push_back(std::move(choice));
    }
    return result;
}

} // namespace tracks
