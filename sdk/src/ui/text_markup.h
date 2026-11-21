// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "ui/label.h"

namespace Lingmo::ui {

// Parses the markup subset notifications and update notes use into Label
// spans: <b>/<strong>, <i>/<em>, <u>, <a href>, <br>, <p>, <div>,
// <h1>..<h6> (bold and a line of their own) and <ul>/<ol>/<li> (a bullet and
// a line each). Other tags are dropped, and the common character references
// are decoded. Links take `linkColor` and stay underlined.
std::vector<Label::Span> styledTextSpans(const std::string &markup, Color linkColor);

} // namespace Lingmo::ui
