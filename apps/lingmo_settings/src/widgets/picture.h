// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>

#include "ui/color_mixer.h"
#include "ui/view.h"

namespace settings {

// The path of one of settings' installed files, such as "images/logo.svg".
std::string dataPath(const std::string& name);
// Where dataPath() looks; tests point it at the source tree.
void setDataDirectory(std::string directory);

// One of settings' own pictures, drawn to fill the view, optionally tinted
// and clipped to rounded corners.
class Picture : public Lingmo::ui::View
{
public:
    explicit Picture(std::string path = {});

    void setPath(std::string path);
    void setTint(std::optional<Lingmo::ui::ColorRecipe> tint);
    void setCornerRadius(double radius);

protected:
    void paint(Lingmo::ui::Painter& painter) override;

private:
    std::string m_path;
    std::optional<Lingmo::ui::ColorRecipe> m_tint;
    double m_radius = 0;
};

} // namespace settings
