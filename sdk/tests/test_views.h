// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/view.h"

namespace Lingmo::UI::test {

// A view with a fixed size hint.
class Box : public View
{
public:
    explicit Box(SizeF size)
        : m_size(size)
    {
    }

    SizeF sizeHint() const override { return m_size; }

private:
    SizeF m_size;
};

} // namespace Lingmo::UI::test
