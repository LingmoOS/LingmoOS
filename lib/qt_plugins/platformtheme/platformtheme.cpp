#include "platformtheme.h"
#include "filedialoghelper.h"
#include "touchpadscrolling.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QVariant>
#include <QWindow>

// Qt Private
#include <private/qiconloader_p.h>
#include <qpa/qwindowsysteminterface.h>

static QColor accentColor(int index)
{
    static const QColor accents[] = {
        QColor(0x33, 0x85, 0xFF),
        QColor(0xFF, 0x5C, 0x6D),
        QColor(0x35, 0xBF, 0x56),
        QColor(0x82, 0x66, 0xFF),
        QColor(0xCA, 0x64, 0xAC),
        QColor(0xFE, 0xA0, 0x42),
        QColor(0x4F, 0x59, 0x6B),
    };
    return index >= 0 && index < int(std::size(accents)) ? accents[index] : accents[0];
}

static QPalette lingmoPalette(bool dark, int accentIndex)
{
    const QColor window = dark ? QColor(0x1C, 0x1C, 0x1D) : QColor(0xF5, 0xF6, 0xFA);
    const QColor base = dark ? QColor(0x2C, 0x2C, 0x2D) : QColor(0xFF, 0xFF, 0xFF);
    const QColor alternate = dark ? QColor(0x3C, 0x3C, 0x3D) : QColor(0xF2, 0xF4, 0xF5);
    const QColor text = dark ? QColor(0xFF, 0xFF, 0xFF) : QColor(0x32, 0x32, 0x38);
    const QColor disabledText = dark ? QColor(0x88, 0x88, 0x88) : QColor(0x64, 0x64, 0x6E);
    const QColor highlight = accentColor(accentIndex);

    QPalette palette(window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, alternate);
    palette.setColor(QPalette::ToolTipBase, window);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::PlaceholderText, disabledText);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, base);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, highlight);
    palette.setColor(QPalette::Highlight, highlight);
    palette.setColor(QPalette::Accent, highlight);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Light, base.lighter(150));
    palette.setColor(QPalette::Midlight, base.lighter(125));
    palette.setColor(QPalette::Mid, base.darker(125));
    palette.setColor(QPalette::Dark, base.darker(150));
    palette.setColor(QPalette::Shadow, Qt::black);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, highlight.lighter(dark ? 130 : 115));
    palette.setColor(QPalette::Disabled, QPalette::Accent, palette.color(QPalette::Disabled, QPalette::Highlight));
    return palette;
}

PlatformTheme::PlatformTheme()
    : m_hints(new HintsSettings)
{
    connect(m_hints, &HintsSettings::systemFontChanged, this, &PlatformTheme::onFontChanged);
    connect(m_hints, &HintsSettings::systemFontPointSizeChanged, this,
            &PlatformTheme::onFontChanged);
    connect(m_hints, &HintsSettings::iconThemeChanged, this, &PlatformTheme::onIconThemeChanged);
    connect(m_hints, &HintsSettings::cursorThemeChanged, this, [] {
        QWindowSystemInterface::handleThemeChange();
    });
    connect(m_hints, &HintsSettings::darkModeChanged, this, [] {
        QWindowSystemInterface::handleThemeChange();
    });
    connect(m_hints, &HintsSettings::accentColorChanged, this, &PlatformTheme::onPaletteChanged);

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);
    // The application object is still being constructed.
    QMetaObject::invokeMethod(this, &TouchpadScrolling::install, Qt::QueuedConnection);
}

PlatformTheme::~PlatformTheme()
{
    delete m_hints;
}

QVariant PlatformTheme::themeHint(QPlatformTheme::ThemeHint hintType) const
{
    QVariant hint = m_hints->hint(hintType);
    if (hint.isValid()) {
        return hint;
    } else {
        return QGenericUnixTheme::themeHint(hintType);
    }
}

Qt::ColorScheme PlatformTheme::colorScheme() const
{
    return m_hints->darkMode() ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light;
}

const QPalette *PlatformTheme::palette(Palette type) const
{
    if (type == SystemPalette) {
        static QPalette palette;
        palette = lingmoPalette(m_hints->darkMode(), m_hints->accentColor());
        return &palette;
    }

    return QGenericUnixTheme::palette(type);
}

const QFont *PlatformTheme::font(Font type) const
{
    if (type == FixedFont) {
        const QString &fontName = m_hints->systemFixedFont();
        qreal fontSize = m_hints->systemFontPointSize();
        static QFont font = QFont(QString());
        font.setFamily(fontName);
        font.setPointSizeF(fontSize);
        return &font;
    }

    const QString &fontName = m_hints->systemFont();
    qreal fontSize = m_hints->systemFontPointSize();
    static QFont font = QFont(QString());
    font.setFamily(fontName);
    font.setPointSizeF(fontSize);
    return &font;
}

bool PlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    return type == FileDialog;
}

QPlatformDialogHelper *PlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    if (type == FileDialog)
        return new FileDialogHelper;
    return QGenericUnixTheme::createPlatformDialogHelper(type);
}

void PlatformTheme::onFontChanged()
{
    QFont font;
    font.setFamily(m_hints->systemFont());
    font.setPointSizeF(m_hints->systemFontPointSize());

    // Change font
    if (qobject_cast<QApplication *>(QCoreApplication::instance()))
        QApplication::setFont(font);
    else if (qobject_cast<QGuiApplication *>(QCoreApplication::instance()))
        QGuiApplication::setFont(font);
}

void PlatformTheme::onIconThemeChanged()
{
    QIconLoader::instance()->updateSystemTheme();

    QEvent update(QEvent::UpdateRequest);
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Tool || window->type() == Qt::Popup
                || window->type() == Qt::ToolTip)
            continue;

        qApp->sendEvent(window, &update);
    }
}

void PlatformTheme::onPaletteChanged()
{
    QWindowSystemInterface::handleThemeChange();
}
