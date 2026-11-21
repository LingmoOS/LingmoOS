// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/searchfield.h"

#include "base/i18n.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kHeight = 28;

SearchField::SearchField()
    : Lingmo::ui::SearchField(tr("Search"))
{
    setIconMetrics(metrics::s, metrics::xs, metrics::xs);
    setLayoutHints({.fillWidth = true});
}

SizeF SearchField::sizeHint() const
{
    return {Lingmo::ui::SearchField::sizeHint().width, kHeight};
}

} // namespace settings
