#include <QVariant>
#include "font_property_push_button.h"

FontPropertyPushButton::FontPropertyPushButton(QWidget *parent)
    : ResizablePushButton(parent)
{
    setTabletModeButtonSize(QSize(36, 36), QSize(48, 48));
    setProperty("isWindowButton", 0x1);
    setProperty("useIconHighlightEffect", 0x2);
    setCheckable(true);
    setFlat(true);
}

FontPropertyPushButton::~FontPropertyPushButton()
{
}