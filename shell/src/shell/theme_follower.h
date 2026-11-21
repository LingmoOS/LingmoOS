// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <memory>
#include "base/settings.h"
#include "core/preferences.h"
#include "decoration/decoration_theme.h"
#include "lingmo/appearance/appearance.h"
#include "ui/theme.h"
namespace Lingmo::Shell {
class ShellThemeFollower : public decoration::ThemeObserver
{
public:
    ShellThemeFollower(decoration::Theme *theme, const std::string &settingsPath)
        : m_theme(theme)
        , m_settingsPath(settingsPath.empty() ? preferencesFile("services") : settingsPath)
    {
        const base::Settings settings(settingsPath.empty() ? preferencesFile("services")
                                                           : settingsPath);
        m_iconTheme = settings.stringValue("Appearance/IconTheme", "lingmo-light");
        m_fontFamily = settings.stringValue("Appearance/Font", "");
        m_fontPointSize = settings.doubleValue("Appearance/FontSize", 9);
        if (m_theme)
            m_theme->addObserver(this);
        m_appearance = std::make_unique<appearance::Appearance>();
        m_changed = m_appearance->changed.connect([this] { apply(); });
        apply();
    }

    ~ShellThemeFollower() override
    {
        if (m_theme)
            m_theme->removeObserver(this);
    }

    void onThemeChanged() override { apply(); }

private:
    void apply()
    {
        static constexpr ui::Color kAccents[] = {ui::accent::blue,   ui::accent::red,
                                                 ui::accent::green,  ui::accent::purple,
                                                 ui::accent::pink,   ui::accent::orange,
                                                 ui::accent::grey};
        const bool dark = m_theme && m_theme->darkMode();
        ui::Theme theme = dark ? ui::Theme::dark() : ui::Theme::light();
        const int accent = m_theme ? m_theme->accentColor() : 0;
        if (accent >= 0 && accent < int(std::size(kAccents)))
            theme.highlight = kAccents[accent];
        // services starts after the compositor. Follow its initial/restarted
        // owner, not just decoration colour changes, and re-read file fallback.
        const base::Settings settings(m_settingsPath);
        m_iconTheme=settings.stringValue("Appearance/IconTheme", "lingmo-light");
        m_fontFamily=settings.stringValue("Appearance/Font", "");
        m_fontPointSize=settings.doubleValue("Appearance/FontSize",9);
        if(m_appearance && m_appearance->isAvailable()) {
            if(!m_appearance->iconTheme().empty())m_iconTheme=m_appearance->iconTheme();
            m_fontFamily=m_appearance->fontFamily();
            m_fontPointSize=m_appearance->fontPointSize();
        }
        theme.iconTheme = m_iconTheme.empty()?"lingmo-light":m_iconTheme;
        theme.fontFamily = m_fontFamily;
        theme.fontPointSize = m_fontPointSize > 0 ? m_fontPointSize : 9;
        theme.continuousCorners = ui::Theme::current().continuousCorners;
        ui::Theme::setCurrent(theme);
    }

    decoration::Theme *m_theme;
    std::string m_settingsPath;
    std::unique_ptr<appearance::Appearance> m_appearance;
    base::Subscription m_changed;
    std::string m_iconTheme;
    std::string m_fontFamily;
    double m_fontPointSize = 9;
};

} // namespace Lingmo::Shell
