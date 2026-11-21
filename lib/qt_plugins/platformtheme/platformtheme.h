#ifndef PLATFORMTHEME_H
#define PLATFORMTHEME_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#include <private/qgenericunixtheme_p.h>
#else
#include <private/qgenericunixthemes_p.h>
#endif
#include "hintsettings.h"

class PlatformTheme : public QObject, public QGenericUnixTheme
{
    Q_OBJECT

public:
    PlatformTheme();
    ~PlatformTheme() override;

    QVariant themeHint(ThemeHint hint) const override;
    Qt::ColorScheme colorScheme() const override;
    const QPalette *palette(Palette type = SystemPalette) const override;
    const QFont *font(Font type) const override;

    bool usePlatformNativeDialog(DialogType type) const override;
    QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;

private:
    void onFontChanged();
    void onIconThemeChanged();
    void onPaletteChanged();

private:
    HintsSettings *m_hints;
};

#endif // PLATFORMTHEME_H
