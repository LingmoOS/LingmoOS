#include <QObject>
#include <QVariant>
#include <QDebug>

#include "font_color_push_button.h"

FontColorPushButton::FontColorPushButton(QWidget *parent,
                                         const QColor &color,
                                         BtnType type)
    : ResizablePushButton(parent)
    , m_buttonColor(color)
{
    if (type != None) {
        setIconType(type);
    }
    setProperty("useIconHighlightEffect", 0x0);
    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::styleStatusUpdate,
                     this, &FontColorPushButton::handleStyleStatus);


}

FontColorPushButton::~FontColorPushButton()
{
}

QIcon FontColorPushButton::getButtonIcon(const QColor &buttonColor, BtnType iconType)
{
    QString type{QString()};
    switch (iconType)
    {
    case BtnType::Circle:
        type = "circle";
        break;
    case BtnType::Rect:
        type = "rect";
        break;
    default:
        break;
    }
    QString colorName = buttonColor.name();
    colorName.remove("#");
    QString iconName = QString(":/%1-%2.svg").arg(type).arg(colorName.toUpper());
    QIcon icon(iconName);
    return  icon;
}

void FontColorPushButton::setButtonColor(const QColor &color)
{
    if (color == QColor() || color == m_buttonColor) {
        return;
    }
    setIcon(getButtonIcon(color, m_IconType));
    m_buttonColor = color;
}


void FontColorPushButton::setIconType(BtnType type)
{
    if (type == m_IconType || type == None) {
        return;
    }
    setIcon(getButtonIcon(m_buttonColor, type));
    m_IconType = type;
}

void FontColorPushButton::handlePADMode()
{
    setIconType(BtnType::Circle);
    ResizablePushButton::handlePADMode();
}

void FontColorPushButton::handlePCMode()
{
    setIconType(BtnType::Rect);
    ResizablePushButton::handlePCMode();
}

void FontColorPushButton::handleDarkMode()
{
    if (m_buttonColor == QColor() || m_buttonColor.name().toUpper() == "#000000") {
        setButtonColor(QColor("#FFFFFF"));
    }
}

void FontColorPushButton::handleLightMode()
{
    if (m_buttonColor == QColor() || m_buttonColor.name().toUpper() == "#FFFFFF") {
        setButtonColor(QColor("#000000"));
    }
}

void FontColorPushButton::showEvent(QShowEvent *event)
{
    ResizablePushButton::showEvent(event);
    handleStyleStatus(LingmoUISettingsMonitor::instance().styleStatus());
}

QColor FontColorPushButton::buttonColor() const
{
    return m_buttonColor;
}

void FontColorPushButton::handleStyleStatus(LingmoUISettingsMonitor::StyleStatus status)
{
    switch (status) {
    case LingmoUISettingsMonitor::StyleStatus::LIGHT:
        handleLightMode();
        break;
    case LingmoUISettingsMonitor::StyleStatus::DARK:
        handleDarkMode();
        break;
    default:
        break;
    }
}
