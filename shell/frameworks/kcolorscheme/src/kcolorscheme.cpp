/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorscheme.h"
#include "kcolorschemehelpers_p.h"

#include "kcolorscheme_debug.h"

#include <KColorUtils>
#include <KConfig>
#include <KConfigGroup>

#include <QBrush>
#include <QColor>

// BEGIN StateEffects
StateEffects::StateEffects(QPalette::ColorGroup state, const KSharedConfigPtr &config)
    : _color(0, 0, 0, 0) //, _chain(0) not needed yet
{
    QString group;
    if (state == QPalette::Disabled) {
        group = QStringLiteral("ColorEffects:Disabled");
    } else if (state == QPalette::Inactive) {
        group = QStringLiteral("ColorEffects:Inactive");
    }

    for (auto &effect : _effects) {
        effect = 0;
    }

    // NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
    if (!group.isEmpty()) {
        KConfigGroup cfg(config, group);
        const bool enabledByDefault = (state == QPalette::Disabled);
        if (cfg.readEntry("Enable", enabledByDefault)) {
            _effects[Intensity] = cfg.readEntry("IntensityEffect", (int)(state == QPalette::Disabled ? IntensityDarken : IntensityNoEffect));
            _effects[Color] = cfg.readEntry("ColorEffect", (int)(state == QPalette::Disabled ? ColorNoEffect : ColorDesaturate));
            _effects[Contrast] = cfg.readEntry("ContrastEffect", (int)(state == QPalette::Disabled ? ContrastFade : ContrastTint));
            _amount[Intensity] = cfg.readEntry("IntensityAmount", state == QPalette::Disabled ? 0.10 : 0.0);
            _amount[Color] = cfg.readEntry("ColorAmount", state == QPalette::Disabled ? 0.0 : -0.9);
            _amount[Contrast] = cfg.readEntry("ContrastAmount", state == QPalette::Disabled ? 0.65 : 0.25);
            if (_effects[Color] > ColorNoEffect) {
                _color = cfg.readEntry("Color", state == QPalette::Disabled ? QColor(56, 56, 56) : QColor(112, 111, 110));
            }
        }
    }
}

QBrush StateEffects::brush(const QBrush &background) const
{
    QColor color = background.color(); // TODO - actually work on brushes
    switch (_effects[Intensity]) {
    case IntensityShade:
        color = KColorUtils::shade(color, _amount[Intensity]);
        break;
    case IntensityDarken:
        color = KColorUtils::darken(color, _amount[Intensity]);
        break;
    case IntensityLighten:
        color = KColorUtils::lighten(color, _amount[Intensity]);
        break;
    }
    switch (_effects[Color]) {
    case ColorDesaturate:
        color = KColorUtils::darken(color, 0.0, 1.0 - _amount[Color]);
        break;
    case ColorFade:
        color = KColorUtils::mix(color, _color, _amount[Color]);
        break;
    case ColorTint:
        color = KColorUtils::tint(color, _color, _amount[Color]);
        break;
    }
    return QBrush(color);
}

QBrush StateEffects::brush(const QBrush &foreground, const QBrush &background) const
{
    QColor color = foreground.color(); // TODO - actually work on brushes
    QColor bg = background.color();
    // Apply the foreground effects
    switch (_effects[Contrast]) {
    case ContrastFade:
        color = KColorUtils::mix(color, bg, _amount[Contrast]);
        break;
    case ContrastTint:
        color = KColorUtils::tint(color, bg, _amount[Contrast]);
        break;
    }
    // Now apply global effects
    return brush(color);
}
// END StateEffects

// BEGIN default colors
struct SerializedColors {
    QColor NormalBackground;
    QColor AlternateBackground;
    QColor NormalText;
    QColor InactiveText;
    QColor ActiveText;
    QColor LinkText;
    QColor VisitedText;
    QColor NegativeText;
    QColor NeutralText;
    QColor PositiveText;
};

struct DecorationColors {
    QColor Focus;
    QColor Hover;
};

// clang-format off
// These numbers come from the default color scheme which is currently
// Breeze Light ([breeze repo]/colors/BreezeLight.colors)
static const SerializedColors defaultViewColors = {
    { 255, 255, 255 }, // Background
    { 247, 247, 247 }, // Alternate
    {  35,  38, 41  }, // Normal
    { 112, 125, 138 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 155,  89, 182 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SerializedColors defaultWindowColors = {
    { 239, 240, 241 }, // Background
    { 227, 229, 231 }, // Alternate
    {  35,  38, 41  }, // Normal
    { 112, 125, 138 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 155,  89, 182 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SerializedColors defaultButtonColors = {
    { 252, 252, 252 }, // Background
    { 163, 212, 250 }, // Alternate
    {  35,  38, 41  }, // Normal
    { 112, 125, 138 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 155,  89, 182 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SerializedColors defaultSelectionColors = {
    {  61, 174, 233 }, // Background
    { 163, 212, 250 }, // Alternate
    { 255, 255, 255 }, // Normal
    { 112, 125, 138 }, // Inactive
    { 255, 255, 255 }, // Active
    { 253, 188,  75 }, // Link
    { 155,  89, 182 }, // Visited
    { 176,  55,  69 }, // Negative
    { 198,  92,   0 }, // Neutral
    {  23, 104,  57 }  // Positive
};

static const SerializedColors defaultTooltipColors = {
    { 247, 247, 247 }, // Background
    { 239, 240, 241 }, // Alternate
    {  35,  38,  41 }, // Normal
    { 112, 125, 138 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 155,  89, 182 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SerializedColors defaultComplementaryColors = {
    {  42,  46,  50 }, // Background
    {  27,  30,  32 }, // Alternate
    { 252, 252, 252 }, // Normal
    { 161, 169, 177 }, // Inactive
    {  61, 174, 233 }, // Active
    {  29, 153, 243 }, // Link
    { 155,  89, 182 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SerializedColors defaultHeaderColors = {
    { 222, 224, 226 }, // Background
    { 239, 240, 241 }, // Alternate
    {  35,  38,  41 }, // Normal
    { 112, 125, 138 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 155,  89, 182 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const DecorationColors defaultDecorationColors = {
    {  61, 174, 233 }, // Focus
    { 147, 206, 233 }, // Hover
};
// END default colors
// clang-format off

//BEGIN KColorSchemePrivate
class KColorSchemePrivate : public QSharedData
{
public:
    explicit KColorSchemePrivate(const KSharedConfigPtr &, QPalette::ColorGroup state, KColorScheme::ColorSet set);
    ~KColorSchemePrivate()
    {
    }

    QBrush background(KColorScheme::BackgroundRole) const;
    QBrush foreground(KColorScheme::ForegroundRole) const;
    QBrush decoration(KColorScheme::DecorationRole) const;
    qreal contrast() const;

    struct Brushes {
        std::array<QBrush, KColorScheme::NForegroundRoles> fg;
        std::array<QBrush, KColorScheme::NBackgroundRoles> bg;
        std::array<QBrush, KColorScheme::NDecorationRoles> deco;

        bool operator==(const Brushes &b) const
        {
            return this == &b || (fg == b.fg && bg == b.bg && deco == b.deco);
        }
    } _brushes;

    qreal _contrast;
};

static SerializedColors loadSerializedColors(const KConfigGroup &group, const SerializedColors &defaults)
{
    constexpr std::array configMap = {
        std::pair{"ForegroundNormal", &SerializedColors::NormalText},
        std::pair{"ForegroundInactive", &SerializedColors::InactiveText},
        std::pair{"ForegroundActive", &SerializedColors::ActiveText},
        std::pair{"ForegroundLink", &SerializedColors::LinkText},
        std::pair{"ForegroundVisited", &SerializedColors::VisitedText},
        std::pair{"ForegroundNegative", &SerializedColors::NegativeText},
        std::pair{"ForegroundNeutral", &SerializedColors::NeutralText},
        std::pair{"ForegroundPositive", &SerializedColors::PositiveText},
        std::pair{"BackgroundNormal", &SerializedColors::NormalBackground},
        std::pair{"BackgroundAlternate", &SerializedColors::AlternateBackground},
    };
    SerializedColors loadedColors;
    for (const auto &entry : configMap) {
      loadedColors.*(entry.second) = group.readEntry(entry.first, defaults.*(entry.second));
    }
    return loadedColors;
}

static DecorationColors loadDecorationColors(const KConfigGroup &group, const DecorationColors &defaults)
{
    DecorationColors colors;
    colors.Focus = group.readEntry("DecorationFocus", defaults.Focus);
    colors.Hover = group.readEntry("DecorationHover", defaults.Hover);
    return colors;
}

KColorSchemePrivate::KColorSchemePrivate(const KSharedConfigPtr &config, QPalette::ColorGroup state, KColorScheme::ColorSet set)
{
    QString groupName;
    SerializedColors defaultColors;
    DecorationColors defaultDecoColors = defaultDecorationColors;
    QColor tint;
    switch (set) {
    case KColorScheme::Window:
        groupName = QStringLiteral("Colors:Window");
        defaultColors = defaultWindowColors;
        break;
    case KColorScheme::Button:
        groupName = QStringLiteral("Colors:Button");
        defaultColors = defaultButtonColors;
        break;
    case KColorScheme::Selection: {
        const KConfigGroup inactiveEffectGroup(config, QStringLiteral("ColorEffects:Inactive"));
        // NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
        const bool inactiveSelectionEffect = inactiveEffectGroup.readEntry("ChangeSelectionColor", inactiveEffectGroup.readEntry("Enable", true));
        // if enabled, inactive/disabled uses Window colors instead, ala gtk
        // ...except tinted with the Selection:NormalBackground color so it looks more like selection
        if (state == QPalette::Active || (state == QPalette::Inactive && !inactiveSelectionEffect)) {
            groupName = QStringLiteral("Colors:Selection");
           defaultColors = defaultSelectionColors;
        } else if (state == QPalette::Inactive) {
            groupName = QStringLiteral("Colors:Window");
            defaultColors = defaultWindowColors;
            tint = config->group(QStringLiteral("Colors:Selection")).readEntry("BackgroundNormal", defaultSelectionColors.NormalBackground);
        } else { // disabled (...and still want this branch when inactive+disabled exists)
            groupName = QStringLiteral("Colors:Window");
            defaultColors = defaultWindowColors;
        }
    } break;
    case KColorScheme::Tooltip:
        groupName = QStringLiteral("Colors:Tooltip");
        defaultColors = defaultTooltipColors;
        break;
    case KColorScheme::Complementary:
        groupName = QStringLiteral("Colors:Complementary");
        defaultColors = defaultComplementaryColors;
        break;
    case KColorScheme::Header:
        groupName = QStringLiteral("Colors:Header");
        defaultColors = loadSerializedColors(config->group(QStringLiteral("Colors:Window")), defaultHeaderColors);
        defaultDecoColors = loadDecorationColors(config->group(QStringLiteral("Colors:Window")), defaultDecorationColors);
        break;
    case KColorScheme::NColorSets:
        qCWarning(KCOLORSCHEME) << "ColorSet::NColorSets is not a valid color set value to pass to KColorScheme::KColorScheme";
        [[fallthrough]];
    case KColorScheme::View:
        groupName = QStringLiteral("Colors:View");
        defaultColors = defaultViewColors;
        break;
    }

    KConfigGroup cfg(config, groupName);
    bool hasInactivePalette = false;
    if (state == QPalette::Inactive) {
        KConfigGroup inactiveGroup = KConfigGroup(&cfg, QStringLiteral("Inactive"));
        if (inactiveGroup.exists()) {
            cfg = inactiveGroup;
            hasInactivePalette = true;
        }
    }

    _contrast = KColorScheme::contrastF(config);

     const SerializedColors loadedColors = loadSerializedColors(cfg, defaultColors);
     const DecorationColors loadedDecoColors = loadDecorationColors(cfg, defaultDecoColors);

    _brushes.fg[KColorScheme::NormalText] = loadedColors.NormalText;
    _brushes.fg[KColorScheme::InactiveText] = loadedColors.InactiveText;
    _brushes.fg[KColorScheme::ActiveText] = loadedColors.ActiveText;
    _brushes.fg[KColorScheme::LinkText] = loadedColors.LinkText;
    _brushes.fg[KColorScheme::VisitedText] = loadedColors.VisitedText;
    _brushes.fg[KColorScheme::NegativeText] = loadedColors.NegativeText;
    _brushes.fg[KColorScheme::NeutralText] = loadedColors.NeutralText;
    _brushes.fg[KColorScheme::PositiveText] = loadedColors.PositiveText;

    _brushes.bg[KColorScheme::NormalBackground] = loadedColors.NormalBackground;
    _brushes.bg[KColorScheme::AlternateBackground] = loadedColors.AlternateBackground;

    _brushes.deco[KColorScheme::FocusColor] = loadedDecoColors.Focus;
    _brushes.deco[KColorScheme::HoverColor] = loadedDecoColors.Hover;

    if (tint.isValid()) {
        // adjustment
        _brushes.bg[KColorScheme::NormalBackground] =
            KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(), tint, 0.4);
        _brushes.bg[KColorScheme::AlternateBackground] =
            KColorUtils::tint(_brushes.bg[KColorScheme::AlternateBackground].color(), tint, 0.4);
    }

    // apply state adjustments
    if (state != QPalette::Active || (state == QPalette::Inactive && !hasInactivePalette)) {
        StateEffects effects(state, config);
        for (auto &fg : _brushes.fg) {
            fg = effects.brush(fg, _brushes.bg[KColorScheme::NormalBackground]);
        }
        for (auto &deco : _brushes.deco) {
            deco = effects.brush(deco, _brushes.bg[KColorScheme::NormalBackground]);
        }
        _brushes.bg[KColorScheme::NormalBackground] = effects.brush(_brushes.bg[KColorScheme::NormalBackground]);
        _brushes.bg[KColorScheme::AlternateBackground] = effects.brush(_brushes.bg[KColorScheme::AlternateBackground]);
    }

    // calculated backgrounds
    _brushes.bg[KColorScheme::ActiveBackground] =
        KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(),
                          _brushes.fg[KColorScheme::ActiveText].color());
    _brushes.bg[KColorScheme::LinkBackground] =
        KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(),
                          _brushes.fg[KColorScheme::LinkText].color());
    _brushes.bg[KColorScheme::VisitedBackground] =
        KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(),
                          _brushes.fg[KColorScheme::VisitedText].color());
    _brushes.bg[KColorScheme::NegativeBackground] =
        KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(),
                          _brushes.fg[KColorScheme::NegativeText].color());
    _brushes.bg[KColorScheme::NeutralBackground] =
        KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(),
                          _brushes.fg[KColorScheme::NeutralText].color());
    _brushes.bg[KColorScheme::PositiveBackground] =
        KColorUtils::tint(_brushes.bg[KColorScheme::NormalBackground].color(),
                          _brushes.fg[KColorScheme::PositiveText].color());
}

QBrush KColorSchemePrivate::background(KColorScheme::BackgroundRole role) const
{
    if (role >= KColorScheme::NormalBackground && role < KColorScheme::NBackgroundRoles) {
        return _brushes.bg[role];
    } else {
        return _brushes.bg[KColorScheme::NormalBackground];
    }
}

QBrush KColorSchemePrivate::foreground(KColorScheme::ForegroundRole role) const
{
    if (role >= KColorScheme::NormalText && role < KColorScheme::NForegroundRoles) {
        return _brushes.fg[role];
    } else {
        return _brushes.fg[KColorScheme::NormalText];
    }
}

QBrush KColorSchemePrivate::decoration(KColorScheme::DecorationRole role) const
{
    if (role >= KColorScheme::FocusColor && role < KColorScheme::NDecorationRoles) {
        return _brushes.deco[role];
    } else {
        return _brushes.deco[KColorScheme::FocusColor];
    }
}

qreal KColorSchemePrivate::contrast() const
{
    return _contrast;
}
//END KColorSchemePrivate

//BEGIN KColorScheme
KColorScheme::KColorScheme(const KColorScheme &) = default;
KColorScheme &KColorScheme::operator=(const KColorScheme &) = default;
KColorScheme::KColorScheme(KColorScheme &&) = default;
KColorScheme &KColorScheme::operator=(KColorScheme &&) = default;
KColorScheme::~KColorScheme() = default;

KColorScheme::KColorScheme(QPalette::ColorGroup state, ColorSet set, KSharedConfigPtr config)
    : d(new KColorSchemePrivate(config ? config : defaultConfig(), state, set))
{
}

bool KColorScheme::operator==(const KColorScheme &other) const
{
    return d == other.d
        || (d->_contrast == other.d->_contrast
            && d->_brushes == other.d->_brushes)
    ;
}

// static
qreal KColorScheme::contrastF(const KSharedConfigPtr &config)
{
    KConfigGroup g(config ? config : defaultConfig(), QStringLiteral("KDE"));
    return 0.1 * g.readEntry("contrast", 7);
}

QBrush KColorScheme::background(BackgroundRole role) const
{
    return d->background(role);
}

QBrush KColorScheme::foreground(ForegroundRole role) const
{
    return d->foreground(role);
}

QBrush KColorScheme::decoration(DecorationRole role) const
{
    return d->decoration(role);
}

QColor KColorScheme::shade(ShadeRole role) const
{
    return shade(background().color(), role, d->contrast());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role)
{
    return shade(color, role, KColorScheme::contrastF());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role, qreal contrast, qreal chromaAdjust)
{
    // nan -> 1.0
    contrast = (1.0 > contrast ? (-1.0 < contrast ? contrast : -1.0) : 1.0);
    qreal y = KColorUtils::luma(color);
    qreal yi = 1.0 - y;

    // handle very dark colors (base, mid, dark, shadow == midlight, light)
    if (y < 0.006) {
        switch (role) {
        case KColorScheme::LightShade:
            return KColorUtils::shade(color, 0.05 + 0.95 * contrast, chromaAdjust);
        case KColorScheme::MidShade:
            return KColorUtils::shade(color, 0.01 + 0.20 * contrast, chromaAdjust);
        case KColorScheme::DarkShade:
            return KColorUtils::shade(color, 0.02 + 0.40 * contrast, chromaAdjust);
        default:
            return KColorUtils::shade(color, 0.03 + 0.60 * contrast, chromaAdjust);
        }
    }

    // handle very light colors (base, midlight, light == mid, dark, shadow)
    if (y > 0.93) {
        switch (role) {
        case KColorScheme::MidlightShade:
            return KColorUtils::shade(color, -0.02 - 0.20 * contrast, chromaAdjust);
        case KColorScheme::DarkShade:
            return KColorUtils::shade(color, -0.06 - 0.60 * contrast, chromaAdjust);
        case KColorScheme::ShadowShade:
            return KColorUtils::shade(color, -0.10 - 0.90 * contrast, chromaAdjust);
        default:
            return KColorUtils::shade(color, -0.04 - 0.40 * contrast, chromaAdjust);
        }
    }

    // handle everything else
    qreal lightAmount = (0.05 + y * 0.55) * (0.25 + contrast * 0.75);
    qreal darkAmount = (- y) * (0.55 + contrast * 0.35);
    switch (role) {
    case KColorScheme::LightShade:
        return KColorUtils::shade(color, lightAmount, chromaAdjust);
    case KColorScheme::MidlightShade:
        return KColorUtils::shade(color, (0.15 + 0.35 * yi) * lightAmount, chromaAdjust);
    case KColorScheme::MidShade:
        return KColorUtils::shade(color, (0.35 + 0.15 * y) * darkAmount, chromaAdjust);
    case KColorScheme::DarkShade:
        return KColorUtils::shade(color, darkAmount, chromaAdjust);
    default:
        return KColorUtils::darken(KColorUtils::shade(color, darkAmount, chromaAdjust), 0.5 + 0.3 * y);
    }
}

void KColorScheme::adjustBackground(QPalette &palette, BackgroundRole newRole, QPalette::ColorRole color,
                                    ColorSet set, KSharedConfigPtr config)
{
    palette.setBrush(QPalette::Active,   color, KColorScheme(QPalette::Active,   set, config).background(newRole));
    palette.setBrush(QPalette::Inactive, color, KColorScheme(QPalette::Inactive, set, config).background(newRole));
    palette.setBrush(QPalette::Disabled, color, KColorScheme(QPalette::Disabled, set, config).background(newRole));
}

void KColorScheme::adjustForeground(QPalette &palette, ForegroundRole newRole, QPalette::ColorRole color,
                                    ColorSet set, KSharedConfigPtr config)
{
    palette.setBrush(QPalette::Active,   color, KColorScheme(QPalette::Active,   set, config).foreground(newRole));
    palette.setBrush(QPalette::Inactive, color, KColorScheme(QPalette::Inactive, set, config).foreground(newRole));
    palette.setBrush(QPalette::Disabled, color, KColorScheme(QPalette::Disabled, set, config).foreground(newRole));
}

bool KColorScheme::isColorSetSupported(const KSharedConfigPtr &config, KColorScheme::ColorSet set)
{
    switch (set) {
        case View:
            return config->hasGroup(QStringLiteral("Colors:View"));
        case Window:
            return config->hasGroup(QStringLiteral("Colors:Window"));
        case Button:
            return config->hasGroup(QStringLiteral("Colors:Button"));
        case Selection:
            return config->hasGroup(QStringLiteral("Colors:Selection"));
        case Tooltip:
            return config->hasGroup(QStringLiteral("Colors:Tooltip"));
        case Complementary:
            return config->hasGroup(QStringLiteral("Colors:Complementary"));
        case Header:
            return config->hasGroup(QStringLiteral("Colors:Header"));
        case NColorSets:
            break;
    }

    return false;
}

QPalette KColorScheme::createApplicationPalette(const KSharedConfigPtr &config)
{
    QPalette palette;

    static const QPalette::ColorGroup states[QPalette::NColorGroups] = {
        QPalette::Active, QPalette::Inactive, QPalette::Disabled
    };

    // TT thinks tooltips shouldn't use active, so we use our active colors for all states
    KColorScheme schemeTooltip(QPalette::Active, KColorScheme::Tooltip, config);

    for (auto state : states) {
        KColorScheme schemeView(state, KColorScheme::View, config);
        KColorScheme schemeWindow(state, KColorScheme::Window, config);
        KColorScheme schemeButton(state, KColorScheme::Button, config);
        KColorScheme schemeSelection(state, KColorScheme::Selection, config);

        palette.setBrush(state, QPalette::WindowText, schemeWindow.foreground());
        palette.setBrush(state, QPalette::Window, schemeWindow.background());
        palette.setBrush(state, QPalette::Base, schemeView.background());
        palette.setBrush(state, QPalette::Text, schemeView.foreground());
        palette.setBrush(state, QPalette::Button, schemeButton.background());
        palette.setBrush(state, QPalette::ButtonText, schemeButton.foreground());
        palette.setBrush(state, QPalette::Highlight, schemeSelection.background());
        palette.setBrush(state, QPalette::HighlightedText, schemeSelection.foreground());
        palette.setBrush(state, QPalette::ToolTipBase, schemeTooltip.background());
        palette.setBrush(state, QPalette::ToolTipText, schemeTooltip.foreground());
        palette.setBrush(state, QPalette::PlaceholderText, schemeView.foreground(KColorScheme::InactiveText));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        palette.setBrush(state, QPalette::Accent, schemeSelection.background());
#endif

        palette.setColor(state, QPalette::Light, schemeWindow.shade(KColorScheme::LightShade));
        palette.setColor(state, QPalette::Midlight, schemeWindow.shade(KColorScheme::MidlightShade));
        palette.setColor(state, QPalette::Mid, schemeWindow.shade(KColorScheme::MidShade));
        palette.setColor(state, QPalette::Dark, schemeWindow.shade(KColorScheme::DarkShade));
        palette.setColor(state, QPalette::Shadow, schemeWindow.shade(KColorScheme::ShadowShade));

        palette.setBrush(state, QPalette::AlternateBase, schemeView.background(KColorScheme::AlternateBackground));
        palette.setBrush(state, QPalette::Link, schemeView.foreground(KColorScheme::LinkText));
        palette.setBrush(state, QPalette::LinkVisited, schemeView.foreground(KColorScheme::VisitedText));
    }

    return palette;
}

//END KColorScheme
